'use client'

import { useState, useCallback } from 'react'
import { MessageCircle } from 'lucide-react'
import AuctionChatDrawer from './AuctionChatDrawer'

interface Props {
  auctionId: string
}

export default function AuctionChatButton({ auctionId }: Props) {
  const [isDrawerOpen, setIsDrawerOpen] = useState(false)
  const [unreadCount, setUnreadCount] = useState(0)

  const handleUnreadChange = useCallback((count: number) => {
    setUnreadCount(count)
  }, [])

  return (
    <>
      <button
        onClick={() => setIsDrawerOpen(true)}
        className="flex items-center gap-2 px-4 py-2.5 bg-[var(--sapphire)]/20 border border-[var(--sapphire)]/30 rounded-lg text-[var(--sapphire)] hover:bg-[var(--sapphire)]/30 transition-colors relative"
      >
        <MessageCircle className="w-4 h-4" />
        💬 Chat Inbox
        {unreadCount > 0 && (
          <span className="absolute -top-2 -right-2 w-5 h-5 bg-red-500 text-white text-[10px] font-bold rounded-full flex items-center justify-center">
            {unreadCount > 9 ? '9+' : unreadCount}
          </span>
        )}
      </button>

      <AuctionChatDrawer
        auctionId={auctionId}
        isOpen={isDrawerOpen}
        onClose={() => setIsDrawerOpen(false)}
        onUnreadChange={handleUnreadChange}
      />
    </>
  )
}
