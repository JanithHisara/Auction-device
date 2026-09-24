import { createClient } from '@/lib/supabase/server'
import { requirePermission } from '@/lib/auth'
import { PERMISSIONS } from '@/lib/permissions'
import { NextRequest, NextResponse } from 'next/server'
import { sendAuctionAccessEmail } from '@/lib/email/resend'

export async function PATCH(
  request: NextRequest,
  { params }: { params: Promise<{ id: string; registrationId: string }> }
) {
  try {
    const { id: auctionId, registrationId } = await params
    const { approval_status } = await request.json()

    const user = await requirePermission(PERMISSIONS.MANAGE_REGISTRATIONS)
    const supabase = await createClient()

    if (!['approved', 'rejected'].includes(approval_status)) {
      return NextResponse.json({ message: 'Invalid status' }, { status: 400 })
    }

    const { data: registration, error: regError } = await supabase
      .from('auction_registrations')
      .select('*, user:users!auction_registrations_user_id_fkey(email, anonymous_name), auction:auctions(name, auction_start)')
      .eq('id', registrationId)
      .eq('auction_id', auctionId)
      .single()

    if (regError || !registration) {
      return NextResponse.json({ message: 'Registration not found' }, { status: 404 })
    }

    if (approval_status === 'approved') {
      const { data: auction } = await supabase
        .from('auctions')
        .select('max_participants')
        .eq('id', auctionId)
        .single()

      if (auction?.max_participants) {
        const { count } = await supabase
          .from('auction_registrations')
          .select('*', { count: 'exact', head: true })
          .eq('auction_id', auctionId)
          .eq('approval_status', 'approved')

        if (count && count >= auction.max_participants) {
          return NextResponse.json({ message: 'Max participants reached' }, { status: 400 })
        }
      }
    }

    const { error: updateError } = await supabase
      .from('auction_registrations')
      .update({
        approval_status,
        approved_at: new Date().toISOString(),
        approved_by: user.id,
      })
      .eq('id', registrationId)

    if (updateError) {
      console.error('Update error:', updateError)
      return NextResponse.json({ message: 'Failed to update' }, { status: 500 })
    }

    if (approval_status === 'approved' && registration.user?.email) {
      try {
        const auctionDate = new Date(registration.auction.auction_start).toLocaleDateString('en-US', {
          weekday: 'long',
          month: 'long',
          day: 'numeric',
          year: 'numeric',
          hour: '2-digit',
          minute: '2-digit',
        })

        await sendAuctionAccessEmail({
          to: registration.user.email,
          auctionName: registration.auction.name,
          auctionDate,
          accessToken: registration.access_token,
        })

        await supabase
          .from('auction_registrations')
          .update({ email_sent_at: new Date().toISOString() })
          .eq('id', registrationId)

      } catch (emailError) {
        console.error('Email send error:', emailError)
      }
    }

    return NextResponse.json({
      success: true,
      message: `Registration ${approval_status}`,
    })

  } catch (error) {
    console.error('Error:', error)
    return NextResponse.json({ message: 'Internal server error' }, { status: 500 })
  }
}
