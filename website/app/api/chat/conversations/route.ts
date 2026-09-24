import { createClient } from '@/lib/supabase/server'
import { requireAuth } from '@/lib/auth'
import { NextResponse } from 'next/server'

export async function POST(request: Request) {
  try {
    const user = await requireAuth()
    const supabase = await createClient()
    const { auction_id } = await request.json()

    if (!auction_id) {
      return NextResponse.json({ error: 'auction_id is required' }, { status: 400 })
    }

    const { data: existing } = await supabase
      .from('chat_conversations')
      .select('*')
      .eq('auction_id', auction_id)
      .eq('user_id', user.id)
      .single()

    if (existing) {
      return NextResponse.json(existing)
    }

    const { data: conversation, error } = await supabase
      .from('chat_conversations')
      .insert({
        auction_id,
        user_id: user.id,
        status: 'open',
      })
      .select()
      .single()

    if (error) throw error

    return NextResponse.json(conversation)
  } catch (error: unknown) {
    const message = error instanceof Error ? error.message : 'Failed to create conversation'
    return NextResponse.json({ error: message }, { status: 500 })
  }
}

export async function GET(request: Request) {
  try {
    const user = await requireAuth()
    const supabase = await createClient()
    const { searchParams } = new URL(request.url)
    const auctionId = searchParams.get('auction_id')

    if (!auctionId) {
      return NextResponse.json({ error: 'auction_id is required' }, { status: 400 })
    }

    const { data: userData } = await supabase
      .from('users')
      .select('role')
      .eq('id', user.id)
      .single()

    const isAdminRole = userData?.role === 'admin' || userData?.role === 'super_admin' || userData?.role === 'moderator'
    if (isAdminRole) {
      const { data: conversations, error } = await supabase
        .from('chat_conversations')
        .select(`
          *,
          user:users!chat_conversations_user_id_fkey(id, email, anonymous_name, display_name, phone),
          assigned_admin:users!chat_conversations_assigned_admin_id_fkey(id, email, display_name)
        `)
        .eq('auction_id', auctionId)
        .order('last_message_at', { ascending: false })

      if (error) throw error
      return NextResponse.json(conversations)
    }

    const { data: conversation, error } = await supabase
      .from('chat_conversations')
      .select('*')
      .eq('auction_id', auctionId)
      .eq('user_id', user.id)
      .single()

    if (error && error.code !== 'PGRST116') throw error
    return NextResponse.json(conversation || null)
  } catch (error: unknown) {
    const message = error instanceof Error ? error.message : 'Failed to get conversations'
    return NextResponse.json({ error: message }, { status: 500 })
  }
}
