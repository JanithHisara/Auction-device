'use client'

import { useState, useEffect, useRef, useCallback } from 'react'
import { createPortal } from 'react-dom'
import { createClient } from '@/lib/supabase/client'
import { ChatConversation, ChatMessage, User } from '@/types/database'
import { MessageCircle, X, Send, Loader2, CheckCircle, ArrowLeft, Circle } from 'lucide-react'

type Tab = 'unassigned' | 'mine' | 'all' | 'resolved'

type ConversationWithRelations = ChatConversation & {
  user?: User
  assigned_admin?: User
}

interface PresenceState {
  admin_id: string
  admin_name: string
  conversation_id: string | null
}

interface Props {
  auctionId: string
  isOpen: boolean
  onClose: () => void
  onUnreadChange: (count: number) => void
}

export default function AuctionChatDrawer({ auctionId, isOpen, onClose, onUnreadChange }: Props) {
  const [conversations, setConversations] = useState<ConversationWithRelations[]>([])
  const [activeConv, setActiveConv] = useState<ConversationWithRelations | null>(null)
  const [messages, setMessages] = useState<ChatMessage[]>([])
  const [input, setInput] = useState('')
  const [isSending, setIsSending] = useState(false)
  const [isLoadingMessages, setIsLoadingMessages] = useState(false)
  const [isLoadingConvs, setIsLoadingConvs] = useState(false)
  const [activeTab, setActiveTab] = useState<Tab>('unassigned')
  const [currentAdminId, setCurrentAdminId] = useState<string>('')
  const [currentAdminName, setCurrentAdminName] = useState<string>('')
  const [viewingAdmins, setViewingAdmins] = useState<PresenceState[]>([])
  const messagesEndRef = useRef<HTMLDivElement>(null)
  const inputRef = useRef<HTMLInputElement>(null)
  const supabase = createClient()

  const scrollToBottom = useCallback(() => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' })
  }, [])

  // Get current admin identity
  useEffect(() => {
    const getAdmin = async () => {
      const { data: { user } } = await supabase.auth.getUser()
      if (user) {
        setCurrentAdminId(user.id)
        const { data: profile } = await supabase
          .from('users')
          .select('display_name, email')
          .eq('id', user.id)
          .single()
        setCurrentAdminName(profile?.display_name || profile?.email || 'Admin')
      }
    }
    getAdmin()
  }, [supabase])

  // Fetch conversations
  const fetchConversations = useCallback(async () => {
    if (!auctionId) return
    setIsLoadingConvs(true)
    try {
      const res = await fetch(`/api/chat/conversations?auction_id=${auctionId}`)
      const data = await res.json()
      if (Array.isArray(data)) {
        setConversations(data)
        const totalUnread = data.reduce((sum: number, c: ConversationWithRelations) => sum + (c.unread_by_admin || 0), 0)
        onUnreadChange(totalUnread)
      }
    } finally {
      setIsLoadingConvs(false)
    }
  }, [auctionId, onUnreadChange])

  useEffect(() => {
    fetchConversations()
  }, [fetchConversations])

  // Realtime: conversation updates
  useEffect(() => {
    if (!auctionId) return

    const channel = supabase
      .channel(`admin-chat-convs-${auctionId}`)
      .on(
        'postgres_changes',
        {
          event: '*',
          schema: 'public',
          table: 'chat_conversations',
          filter: `auction_id=eq.${auctionId}`,
        },
        () => {
          fetchConversations()
        }
      )
      .subscribe()

    return () => {
      supabase.removeChannel(channel)
    }
  }, [auctionId, supabase, fetchConversations])

  // Fetch messages for active conversation
  useEffect(() => {
    if (!activeConv?.id) return

    const fetchMessages = async () => {
      setIsLoadingMessages(true)
      const res = await fetch(`/api/chat/conversations/${activeConv.id}/messages`)
      const data = await res.json()
      if (Array.isArray(data)) {
        setMessages(data)
      }
      setIsLoadingMessages(false)
      setTimeout(scrollToBottom, 100)
    }
    fetchMessages()

    // Mark as read
    fetch(`/api/chat/conversations/${activeConv.id}`, {
      method: 'PATCH',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ action: 'mark_read' }),
    })
  }, [activeConv?.id, scrollToBottom])

  // Realtime: new messages in active conversation
  useEffect(() => {
    if (!activeConv?.id) return

    const channel = supabase
      .channel(`admin-chat-msgs-${activeConv.id}`)
      .on(
        'postgres_changes',
        {
          event: 'INSERT',
          schema: 'public',
          table: 'chat_messages',
          filter: `conversation_id=eq.${activeConv.id}`,
        },
        async (payload) => {
          const newMsg = payload.new as ChatMessage
          if (newMsg.sender_id === currentAdminId) return

          const { data: sender } = await supabase
            .from('users')
            .select('id, email, display_name, anonymous_name')
            .eq('id', newMsg.sender_id)
            .single()

          setMessages(prev => [...prev, { ...newMsg, sender: sender as ChatMessage['sender'] }])
          setTimeout(scrollToBottom, 100)

          fetch(`/api/chat/conversations/${activeConv.id}`, {
            method: 'PATCH',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ action: 'mark_read' }),
          })
        }
      )
      .subscribe()

    return () => {
      supabase.removeChannel(channel)
    }
  }, [activeConv?.id, currentAdminId, supabase, scrollToBottom])

  // Presence: track which admin is viewing which conversation
  useEffect(() => {
    if (!auctionId || !currentAdminId) return

    const presenceChannel = supabase.channel(`admin-presence-${auctionId}`, {
      config: { presence: { key: currentAdminId } },
    })

    presenceChannel
      .on('presence', { event: 'sync' }, () => {
        const state = presenceChannel.presenceState<PresenceState>()
        const admins: PresenceState[] = []
        for (const key of Object.keys(state)) {
          if (key !== currentAdminId) {
            const entries = state[key]
            if (entries?.length) {
              admins.push(entries[0])
            }
          }
        }
        setViewingAdmins(admins)
      })
      .subscribe(async (status) => {
        if (status === 'SUBSCRIBED') {
          await presenceChannel.track({
            admin_id: currentAdminId,
            admin_name: currentAdminName,
            conversation_id: activeConv?.id || null,
          })
        }
      })

    return () => {
      supabase.removeChannel(presenceChannel)
    }
  }, [auctionId, currentAdminId, currentAdminName, activeConv?.id, supabase])

  const handleSend = async (e: React.FormEvent) => {
    e.preventDefault()
    if (!input.trim() || isSending || !activeConv?.id) return

    const messageContent = input.trim()
    setInput('')
    setIsSending(true)

    const optimistic: ChatMessage = {
      id: `temp-${Date.now()}`,
      conversation_id: activeConv.id,
      sender_id: currentAdminId,
      sender_role: 'admin',
      content: messageContent,
      created_at: new Date().toISOString(),
      sender: { id: currentAdminId, email: '', display_name: currentAdminName } as ChatMessage['sender'],
    }
    setMessages(prev => [...prev, optimistic])
    setTimeout(scrollToBottom, 50)

    try {
      const res = await fetch(`/api/chat/conversations/${activeConv.id}/messages`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ content: messageContent }),
      })

      if (!res.ok) throw new Error('Failed to send')

      const sent = await res.json()
      setMessages(prev => prev.map(m => m.id === optimistic.id ? sent : m))
      fetchConversations()
    } catch {
      setMessages(prev => prev.filter(m => !m.id.startsWith('temp-')))
      setInput(messageContent)
    } finally {
      setIsSending(false)
      inputRef.current?.focus()
    }
  }

  const handleResolve = async () => {
    if (!activeConv?.id) return
    await fetch(`/api/chat/conversations/${activeConv.id}`, {
      method: 'PATCH',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ action: 'resolve' }),
    })
    setActiveConv(null)
    fetchConversations()
  }

  const getUserDisplayName = (conv: ConversationWithRelations) => {
    return conv.user?.display_name || conv.user?.anonymous_name || conv.user?.email || 'Unknown User'
  }

  const filteredConversations = conversations.filter(conv => {
    switch (activeTab) {
      case 'unassigned':
        return conv.status === 'open' && !conv.assigned_admin_id
      case 'mine':
        return conv.assigned_admin_id === currentAdminId && conv.status !== 'resolved'
      case 'all':
        return conv.status !== 'resolved'
      case 'resolved':
        return conv.status === 'resolved'
    }
  })

  const sortedConversations = [...filteredConversations].sort((a, b) => {
    if (a.unread_by_admin > 0 && b.unread_by_admin === 0) return -1
    if (a.unread_by_admin === 0 && b.unread_by_admin > 0) return 1
    return new Date(a.last_message_at).getTime() - new Date(b.last_message_at).getTime()
  })

  const tabCounts = {
    unassigned: conversations.filter(c => c.status === 'open' && !c.assigned_admin_id).length,
    mine: conversations.filter(c => c.assigned_admin_id === currentAdminId && c.status !== 'resolved').length,
    all: conversations.filter(c => c.status !== 'resolved').length,
    resolved: conversations.filter(c => c.status === 'resolved').length,
  }

  const statusDot: Record<string, string> = {
    open: 'bg-red-500',
    active: 'bg-blue-500',
    waiting: 'bg-amber-500',
    resolved: 'bg-gray-500',
  }

  const formatTimeAgo = (dateStr: string) => {
    const diff = Date.now() - new Date(dateStr).getTime()
    const mins = Math.floor(diff / 60000)
    if (mins < 1) return 'now'
    if (mins < 60) return `${mins}m`
    const hours = Math.floor(mins / 60)
    if (hours < 24) return `${hours}h`
    return `${Math.floor(hours / 24)}d`
  }

  const formatTime = (dateStr: string) => {
    return new Date(dateStr).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
  }

  const adminsViewingConv = (convId: string) => {
    return viewingAdmins.filter(a => a.conversation_id === convId)
  }

  const [mounted, setMounted] = useState(false)
  useEffect(() => { setMounted(true) }, [])

  if (!isOpen || !mounted) return null

  return createPortal(
    <>
      {/* Backdrop */}
      <div className="fixed inset-0 z-[9999] bg-black/40" onClick={onClose} />

      {/* Drawer */}
      <div className="fixed right-0 top-0 bottom-0 z-[10000] w-full sm:w-[420px] bg-[var(--background)] border-l border-[var(--border)] shadow-2xl flex flex-col">
        {/* Header */}
        <div className="flex items-center justify-between px-4 py-3 bg-[var(--surface)] border-b border-[var(--border)]">
          <div className="flex items-center gap-3">
            {activeConv && (
              <button
                onClick={() => setActiveConv(null)}
                className="w-8 h-8 rounded-lg hover:bg-[var(--surface-elevated)] flex items-center justify-center text-[var(--text-muted)] hover:text-white transition-colors"
              >
                <ArrowLeft className="w-4 h-4" />
              </button>
            )}
            <div>
              <h3 className="text-sm font-bold text-white">
                {activeConv ? getUserDisplayName(activeConv) : 'Chat Inbox'}
              </h3>
              {activeConv && (
                <p className="text-xs text-[var(--text-muted)]">
                  {activeConv.user?.email}
                  {activeConv.user?.phone && ` · ${activeConv.user.phone}`}
                </p>
              )}
            </div>
          </div>
          <div className="flex items-center gap-2">
            {activeConv && activeConv.status !== 'resolved' && (
              <button
                onClick={handleResolve}
                className="flex items-center gap-1.5 px-3 py-1.5 bg-emerald-500/20 text-emerald-400 rounded-lg text-xs font-medium hover:bg-emerald-500/30 transition-colors"
              >
                <CheckCircle className="w-3.5 h-3.5" />
                Resolve
              </button>
            )}
            <button
              onClick={onClose}
              className="w-8 h-8 rounded-lg hover:bg-[var(--surface-elevated)] flex items-center justify-center text-[var(--text-muted)] hover:text-white transition-colors"
            >
              <X className="w-4 h-4" />
            </button>
          </div>
        </div>

        {activeConv ? (
          /* Thread View */
          <>
            {/* Presence banner */}
            {adminsViewingConv(activeConv.id).length > 0 && (
              <div className="px-4 py-2 bg-blue-500/10 border-b border-blue-500/20 text-xs text-blue-400 flex items-center gap-2">
                <Circle className="w-2 h-2 fill-blue-400" />
                Also viewing: {adminsViewingConv(activeConv.id).map(a => a.admin_name).join(', ')}
              </div>
            )}

            {/* Assigned info */}
            {activeConv.assigned_admin_id && activeConv.assigned_admin_id !== currentAdminId && activeConv.assigned_admin && (
              <div className="px-4 py-2 bg-amber-500/10 border-b border-amber-500/20 text-xs text-amber-400">
                Assigned to {activeConv.assigned_admin.display_name || activeConv.assigned_admin.email}
              </div>
            )}

            {/* Messages */}
            <div className="flex-1 overflow-y-auto px-4 py-3 space-y-3">
              {isLoadingMessages ? (
                <div className="flex items-center justify-center h-full">
                  <Loader2 className="w-5 h-5 animate-spin text-[var(--text-muted)]" />
                </div>
              ) : (
                messages.map((msg) => {
                  const isAdmin = msg.sender_role !== 'user'
                  const senderName = isAdmin
                    ? (msg.sender?.display_name || msg.sender?.email || 'Admin')
                    : getUserDisplayName(activeConv)
                  return (
                    <div
                      key={msg.id}
                      className={`flex ${isAdmin ? 'justify-end' : 'justify-start'}`}
                    >
                      <div className="max-w-[80%]">
                        <p className={`text-[10px] font-medium mb-1 ${
                          isAdmin ? 'text-right mr-1 text-[var(--sapphire)]' : 'ml-1 text-[var(--text-muted)]'
                        }`}>
                          {senderName}
                        </p>
                        <div
                          className={`px-3 py-2 rounded-2xl text-sm leading-relaxed ${
                            isAdmin
                              ? 'bg-[var(--sapphire)]/20 text-white rounded-br-md border border-[var(--sapphire)]/30'
                              : 'bg-[var(--surface)] text-white rounded-bl-md border border-[var(--border)]'
                          }`}
                        >
                          {msg.content}
                        </div>
                        <p className={`text-[10px] text-[var(--text-muted)] mt-1 ${isAdmin ? 'text-right mr-1' : 'ml-1'}`}>
                          {formatTime(msg.created_at)}
                        </p>
                      </div>
                    </div>
                  )
                })
              )}
              <div ref={messagesEndRef} />
            </div>

            {/* Input */}
            {activeConv.status !== 'resolved' ? (
              <form onSubmit={handleSend} className="px-3 py-3 border-t border-[var(--border)] bg-[var(--surface)]">
                <div className="flex items-center gap-2">
                  <input
                    ref={inputRef}
                    type="text"
                    value={input}
                    onChange={(e) => setInput(e.target.value)}
                    placeholder="Reply as admin..."
                    className="flex-1 bg-[var(--background)] border border-[var(--border)] rounded-xl px-3 py-2.5 text-sm text-white placeholder-[var(--text-muted)] focus:outline-none focus:border-[var(--sapphire)]/50"
                    autoComplete="off"
                  />
                  <button
                    type="submit"
                    disabled={!input.trim() || isSending}
                    className="w-10 h-10 bg-[var(--sapphire)] hover:bg-[var(--sapphire)]/80 disabled:opacity-40 text-white rounded-xl flex items-center justify-center transition-colors flex-shrink-0"
                  >
                    {isSending ? (
                      <Loader2 className="w-4 h-4 animate-spin" />
                    ) : (
                      <Send className="w-4 h-4" />
                    )}
                  </button>
                </div>
              </form>
            ) : (
              <div className="px-4 py-3 border-t border-[var(--border)] bg-[var(--surface)] text-center text-xs text-[var(--text-muted)]">
                This conversation is resolved
              </div>
            )}
          </>
        ) : (
          /* Conversation List */
          <>
            {/* Tabs */}
            <div className="flex border-b border-[var(--border)] bg-[var(--surface)]">
              {(['unassigned', 'mine', 'all', 'resolved'] as Tab[]).map((tab) => (
                <button
                  key={tab}
                  onClick={() => setActiveTab(tab)}
                  className={`flex-1 px-2 py-2.5 text-xs font-medium transition-colors relative ${
                    activeTab === tab
                      ? 'text-white'
                      : 'text-[var(--text-muted)] hover:text-[var(--text-secondary)]'
                  }`}
                >
                  <span className="capitalize">{tab}</span>
                  {tabCounts[tab] > 0 && (
                    <span className={`ml-1 px-1.5 py-0.5 rounded-full text-[10px] font-bold ${
                      tab === 'unassigned' && tabCounts[tab] > 0
                        ? 'bg-red-500 text-white'
                        : 'bg-[var(--surface-elevated)] text-[var(--text-muted)]'
                    }`}>
                      {tabCounts[tab]}
                    </span>
                  )}
                  {activeTab === tab && (
                    <div className="absolute bottom-0 left-2 right-2 h-0.5 bg-white rounded-full" />
                  )}
                </button>
              ))}
            </div>

            {/* List */}
            <div className="flex-1 overflow-y-auto">
              {isLoadingConvs ? (
                <div className="flex items-center justify-center h-32">
                  <Loader2 className="w-5 h-5 animate-spin text-[var(--text-muted)]" />
                </div>
              ) : sortedConversations.length === 0 ? (
                <div className="flex flex-col items-center justify-center h-32 text-center px-4">
                  <MessageCircle className="w-8 h-8 text-[var(--text-muted)] mb-2" />
                  <p className="text-sm text-[var(--text-muted)]">
                    {activeTab === 'unassigned' ? 'No unassigned conversations' :
                     activeTab === 'mine' ? 'No conversations assigned to you' :
                     activeTab === 'resolved' ? 'No resolved conversations' :
                     'No active conversations'}
                  </p>
                </div>
              ) : (
                sortedConversations.map(conv => {
                  const viewingThis = adminsViewingConv(conv.id)
                  return (
                    <button
                      key={conv.id}
                      onClick={() => setActiveConv(conv)}
                      className="w-full px-4 py-3 border-b border-[var(--border)] hover:bg-[var(--surface)] transition-colors text-left"
                    >
                      <div className="flex items-start gap-3">
                        {/* Status dot */}
                        <div className="mt-1.5 flex-shrink-0">
                          <div className={`w-2.5 h-2.5 rounded-full ${statusDot[conv.status] || 'bg-gray-500'}`} />
                        </div>

                        {/* Content */}
                        <div className="flex-1 min-w-0">
                          <div className="flex items-center justify-between gap-2 mb-0.5">
                            <span className="text-sm font-medium text-white truncate">
                              {getUserDisplayName(conv)}
                            </span>
                            <span className="text-[10px] text-[var(--text-muted)] flex-shrink-0">
                              {formatTimeAgo(conv.last_message_at)}
                            </span>
                          </div>

                          {/* Assigned tag + viewing indicators */}
                          <div className="flex items-center gap-2 mb-1">
                            {conv.assigned_admin_id && conv.assigned_admin_id !== currentAdminId && conv.assigned_admin && (
                              <span className="text-[10px] px-1.5 py-0.5 bg-[var(--surface-elevated)] text-[var(--text-muted)] rounded">
                                {conv.assigned_admin.display_name || conv.assigned_admin.email}
                              </span>
                            )}
                            {viewingThis.length > 0 && (
                              <span className="text-[10px] px-1.5 py-0.5 bg-blue-500/20 text-blue-400 rounded flex items-center gap-1">
                                <Circle className="w-1.5 h-1.5 fill-blue-400" />
                                {viewingThis.map(a => a.admin_name).join(', ')}
                              </span>
                            )}
                          </div>

                          <p className="text-xs text-[var(--text-muted)] truncate">
                            {conv.user?.email}
                          </p>
                        </div>

                        {/* Unread badge */}
                        {conv.unread_by_admin > 0 && (
                          <span className="flex-shrink-0 w-5 h-5 bg-red-500 text-white text-[10px] font-bold rounded-full flex items-center justify-center">
                            {conv.unread_by_admin > 9 ? '9+' : conv.unread_by_admin}
                          </span>
                        )}
                      </div>
                    </button>
                  )
                })
              )}
            </div>
          </>
        )}
      </div>
    </>,
    document.body
  )
}
