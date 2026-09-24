import { NextRequest, NextResponse } from 'next/server'
import { randomUUID } from 'crypto'
import { requirePermission } from '@/lib/auth'
import { createAdminClient } from '@/lib/supabase/admin'
import { PERMISSIONS } from '@/lib/permissions'

export async function POST(
  request: NextRequest,
  { params }: { params: Promise<{ id: string }> }
) {
  try {
    await requirePermission(PERMISSIONS.MANAGE_REGISTRATIONS)

    const { id: auctionId } = await params
    const body = await request.json()
    const { user_id } = body

    if (!user_id) {
      return NextResponse.json({ error: 'User ID is required' }, { status: 400 })
    }

    const adminClient = createAdminClient()

    // Verify auction exists
    const { data: auction } = await adminClient
      .from('auctions')
      .select('id, name, max_participants')
      .eq('id', auctionId)
      .single()

    if (!auction) {
      return NextResponse.json({ error: 'Auction not found' }, { status: 404 })
    }

    // Verify user exists
    const { data: user } = await adminClient
      .from('users')
      .select('id, email, display_name')
      .eq('id', user_id)
      .single()

    if (!user) {
      return NextResponse.json({ error: 'User not found' }, { status: 404 })
    }

    // Check if already registered
    const { data: existingReg } = await adminClient
      .from('auction_registrations')
      .select('id, approval_status')
      .eq('auction_id', auctionId)
      .eq('user_id', user_id)
      .single()

    if (existingReg) {
      return NextResponse.json(
        { error: `User is already registered (${existingReg.approval_status})` },
        { status: 409 },
      )
    }

    // Check max participants
    if (auction.max_participants) {
      const { count } = await adminClient
        .from('auction_registrations')
        .select('*', { count: 'exact', head: true })
        .eq('auction_id', auctionId)
        .eq('approval_status', 'approved')

      if ((count || 0) >= auction.max_participants) {
        return NextResponse.json(
          { error: 'Maximum participants reached for this auction' },
          { status: 409 },
        )
      }
    }

    // Register with auto-approval
    const { data: registration, error: regError } = await adminClient
      .from('auction_registrations')
      .insert({
        auction_id: auctionId,
        user_id,
        access_token: randomUUID(),
        approval_status: 'approved',
        approved_at: new Date().toISOString(),
        is_active: true,
      })
      .select()
      .single()

    if (regError) {
      return NextResponse.json({ error: regError.message }, { status: 500 })
    }

    return NextResponse.json({
      registration,
      user: { id: user.id, email: user.email, display_name: user.display_name },
    }, { status: 201 })
  } catch {
    return NextResponse.json({ error: 'Unauthorized' }, { status: 403 })
  }
}
