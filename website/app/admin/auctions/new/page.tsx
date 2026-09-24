'use client'

import { useState, useRef, useEffect } from 'react'
import { useRouter } from 'next/navigation'
import { createClient } from '@/lib/supabase/client'
import Link from 'next/link'
import ImageUploader from '@/components/gems/ImageUploader'
import { ArrowLeft, Loader2, Calendar, Image, Users, DollarSign, Gavel, TrendingUp, Target } from 'lucide-react'
import DateTimePicker from '@/components/ui/DateTimePicker'

// Convert a `datetime-local` value (interpreted in the admin's local timezone)
// into a UTC ISO string so timestamptz columns store the correct instant.
function toUTCISO(localDatetime: string) {
  if (!localDatetime) return localDatetime
  return new Date(localDatetime).toISOString()
}

export default function NewAuctionPage() {
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState<string | null>(null)
  const router = useRouter()
  const supabase = createClient()
  const errorRef = useRef<HTMLDivElement>(null)

  const [formData, setFormData] = useState({
    name: '',
    description: '',
    banner_image_url: '',
    password: '',
    auction_type: 'tender_base_fixed_bid' as 'progressive_elimination_auction' | 'tender_base_fixed_bid' | 'incremental_approval_auction',
    registration_start: '',
    registration_end: '',
    auction_start: '',
    auction_end: '',
    max_participants: '',
    entry_fee: '0',
  })

  const handleChange = (e: React.ChangeEvent<HTMLInputElement | HTMLTextAreaElement | HTMLSelectElement>) => {
    let val = e.target.value
    if (e.target.type === 'number') {
      val = val.replace(/^0+(?=\d)/, '')
    }
    setFormData(prev => ({ ...prev, [e.target.name]: val }))
  }

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    if (loading) return
    setError(null)
    setLoading(true)

    try {
      const regStart = new Date(formData.registration_start)
      const regEnd = new Date(formData.registration_end)
      const aucStart = new Date(formData.auction_start)
      const aucEnd = new Date(formData.auction_end)

      const now = new Date()
      if (regStart < now) {
        throw new Error('Registration start time must be in the future')
      }
      if (regEnd < now) {
        throw new Error('Registration end time must be in the future')
      }
      if (aucStart < now) {
        throw new Error('Auction start time must be in the future')
      }
      if (aucEnd < now) {
        throw new Error('Auction end time must be in the future')
      }

      if (regEnd <= regStart) {
        throw new Error('Registration end time must be after registration start time')
      }
      if (aucStart <= regEnd) {
        throw new Error('Auction start time must be after registration end time')
      }
      if (aucEnd <= aucStart) {
        throw new Error('Auction end time must be after auction start time')
      }

      if (!formData.password || formData.password.length !== 4 || !/^\d{4}$/.test(formData.password)) {
        throw new Error('Auction password must be exactly 4 numeric digits')
      }

      const { data: { user } } = await supabase.auth.getUser()
      if (!user) throw new Error('Not authenticated')


      // Check for duplicate auction name (case-insensitive)
      const { data: existing } = await supabase
        .from('auctions')
        .select('id')
        .ilike('name', formData.name.trim())
        .limit(1)
        .maybeSingle()
      if (existing) {
        throw new Error('An auction with this name already exists. Please choose a different name.')
      }
      const randomAuctionCode = 'AUC-' + Math.random().toString(36).substr(2, 6).toUpperCase();

      const { data, error: insertError } = await supabase
        .from('auctions')
        .insert({
          admin_id: user.id,
          name: formData.name,
          auction_code: randomAuctionCode,
          description: formData.description || null,
          banner_image_url: formData.banner_image_url || null,
          password: formData.password || null,
          auction_type: formData.auction_type,
          published_at: null, // Defaults to null, manual publishing flow
          registration_start: toUTCISO(formData.registration_start),
          registration_end: toUTCISO(formData.registration_end),
          auction_start: toUTCISO(formData.auction_start),
          auction_end: new Date('2099-12-31T23:59:59Z').toISOString(),
          max_participants: formData.max_participants ? parseInt(formData.max_participants) : null,
          entry_fee: parseFloat(formData.entry_fee) || 0,
          status: 'draft',
        })
        .select()
        .single()

      if (insertError) throw insertError

      router.push(`/admin/auctions/${data.id}`)
    } catch (err: unknown) {
      const message = err instanceof Error ? err.message : 'Failed to create auction'
      setError(message)
      setTimeout(() => { errorRef.current?.scrollIntoView({ behavior: 'smooth', block: 'center' }) }, 50)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="max-w-3xl mx-auto">
      <Link 
        href="/admin/auctions"
        className="inline-flex items-center gap-2 text-[var(--text-secondary)] hover:text-white mb-6 transition-colors"
      >
        <ArrowLeft className="w-4 h-4" />
        Back to Auctions
      </Link>

      <div className="card-glass rounded-2xl p-8">
        <h1 className="text-3xl font-bold text-white mb-2">Create New Auction</h1>
        <p className="text-[var(--text-secondary)] mb-8">Set up a new auction event</p>

        {error && (
          <div ref={errorRef} className="error-message mb-6 flex items-center gap-2">
            <span>⚠️</span>
            {error}
          </div>
        )}

        <form onSubmit={handleSubmit} className="space-y-8">
          <fieldset disabled={loading} className="border-0 p-0 m-0 min-w-0 space-y-8">
          {/* Basic Info */}
          <section className="space-y-4">
            <h2 className="text-lg font-bold text-white flex items-center gap-2">
              <span className="w-8 h-8 rounded-lg bg-[var(--gold)]/20 flex items-center justify-center text-sm">1</span>
              Basic Information
            </h2>
            
            <div>
              <label className="block text-sm text-[var(--text-secondary)] mb-2">
                Auction Name *
              </label>
              <input
                type="text"
                name="name"
                value={formData.name}
                onChange={handleChange}
                required
                placeholder="e.g., Spring Gemstone Collection 2024"
                className="w-full"
              />
            </div>

            <div>
              <label className="block text-sm text-[var(--text-secondary)] mb-2">
                Description
              </label>
              <textarea
                name="description"
                value={formData.description}
                onChange={handleChange}
                rows={3}
                placeholder="Describe what makes this auction special..."
                className="w-full resize-none"
              />
            </div>

            <div>
              <label className="block text-sm text-[var(--text-secondary)] mb-2 flex items-center gap-2">
                <Image className="w-4 h-4" />
                Banner Image/Media
              </label>
              <ImageUploader 
                images={[formData.banner_image_url || '']}
                maxImages={1}
                onChange={(images) => setFormData({ ...formData, banner_image_url: images[0] || '' })}
              />
            </div>
          </section>

          {/* Auction Type */}
          <section className="space-y-4">
            <h2 className="text-lg font-bold text-white flex items-center gap-2">
              <span className="w-8 h-8 rounded-lg bg-[var(--gold)]/20 flex items-center justify-center text-sm">2</span>
              Auction Type
            </h2>
            
            <div className="grid sm:grid-cols-3 gap-4">
              <label 
                className={`relative cursor-pointer p-5 rounded-xl border-2 transition-all ${
                  formData.auction_type === 'tender_base_fixed_bid' 
                    ? 'border-[var(--gold)] bg-[var(--gold)]/10' 
                    : 'border-[var(--border)] hover:border-[var(--gold)]/50'
                }`}
              >
                <input
                  type="radio"
                  name="auction_type"
                  value="tender_base_fixed_bid"
                  checked={formData.auction_type === 'tender_base_fixed_bid'}
                  onChange={handleChange}
                  className="sr-only"
                />
                <div className="flex items-start gap-3">
                  <div className={`w-10 h-10 rounded-lg flex items-center justify-center ${
                    formData.auction_type === 'tender_base_fixed_bid' ? 'bg-[var(--gold)]' : 'bg-[var(--surface)]'
                  }`}>
                    <TrendingUp className={`w-5 h-5 ${formData.auction_type === 'tender_base_fixed_bid' ? 'text-black' : 'text-[var(--text-muted)]'}`} />
                  </div>
                  <div>
                    <h3 className="font-bold text-white">Closed Bid Auction</h3>
                    <p className="text-sm text-[var(--text-muted)] mt-1">
                      Bidders submit any amount above minimum increment. Highest bid wins.
                    </p>
                  </div>
                </div>
                {formData.auction_type === 'tender_base_fixed_bid' && (
                  <div className="absolute top-3 right-3 w-5 h-5 bg-[var(--gold)] rounded-full flex items-center justify-center">
                    <span className="text-black text-xs">✓</span>
                  </div>
                )}
              </label>

              <label 
                className={`relative cursor-pointer p-5 rounded-xl border-2 transition-all ${
                  formData.auction_type === 'progressive_elimination_auction' 
                    ? 'border-[var(--gold)] bg-[var(--gold)]/10' 
                    : 'border-[var(--border)] hover:border-[var(--gold)]/50'
                }`}
              >
                <input
                  type="radio"
                  name="auction_type"
                  value="progressive_elimination_auction"
                  checked={formData.auction_type === 'progressive_elimination_auction'}
                  onChange={handleChange}
                  className="sr-only"
                />
                <div className="flex items-start gap-3">
                  <div className={`w-10 h-10 rounded-lg flex items-center justify-center ${
                    formData.auction_type === 'progressive_elimination_auction' ? 'bg-[var(--gold)]' : 'bg-[var(--surface)]'
                  }`}>
                    <Gavel className={`w-5 h-5 ${formData.auction_type === 'progressive_elimination_auction' ? 'text-black' : 'text-[var(--text-muted)]'}`} />
                  </div>
                  <div>
                    <h3 className="font-bold text-white">English Auction</h3>
                    <p className="text-sm text-[var(--text-muted)] mt-1">
                      Admin raises the price each round and only one bidder wins per round. If no one accepts, the last round winner wins.
                    </p>
                  </div>
                </div>
                {formData.auction_type === 'progressive_elimination_auction' && (
                  <div className="absolute top-3 right-3 w-5 h-5 bg-[var(--gold)] rounded-full flex items-center justify-center">
                    <span className="text-black text-xs">✓</span>
                  </div>
                )}
              </label>

              <label 
                className={`relative cursor-pointer p-5 rounded-xl border-2 transition-all ${
                  formData.auction_type === 'incremental_approval_auction' 
                    ? 'border-[var(--gold)] bg-[var(--gold)]/10' 
                    : 'border-[var(--border)] hover:border-[var(--gold)]/50'
                }`}
              >
                <input
                  type="radio"
                  name="auction_type"
                  value="incremental_approval_auction"
                  checked={formData.auction_type === 'incremental_approval_auction'}
                  onChange={handleChange}
                  className="sr-only"
                />
                <div className="flex items-start gap-3">
                  <div className={`w-10 h-10 rounded-lg flex items-center justify-center ${
                    formData.auction_type === 'incremental_approval_auction' ? 'bg-[var(--gold)]' : 'bg-[var(--surface)]'
                  }`}>
                    <Target className={`w-5 h-5 ${formData.auction_type === 'incremental_approval_auction' ? 'text-black' : 'text-[var(--text-muted)]'}`} />
                  </div>
                  <div>
                    <h3 className="font-bold text-white">Progressive Elimination Auction</h3>
                    <p className="text-sm text-[var(--text-muted)] mt-1">
                      Admin raises the price each round. Bidders who don&apos;t accept are eliminated. If no one accepts, the next round can start with a decreased price. Last remaining bidder wins.
                    </p>
                  </div>
                </div>
                {formData.auction_type === 'incremental_approval_auction' && (
                  <div className="absolute top-3 right-3 w-5 h-5 bg-[var(--gold)] rounded-full flex items-center justify-center">
                    <span className="text-black text-xs">✓</span>
                  </div>
                )}
              </label>
            </div>
          </section>

          {/* Schedule */}
          <section className="space-y-4">
            <h2 className="text-lg font-bold text-white flex items-center gap-2">
              <span className="w-8 h-8 rounded-lg bg-[var(--gold)]/20 flex items-center justify-center text-sm">3</span>
              Schedule
            </h2>
            


            <div className="grid sm:grid-cols-2 gap-4">
              <div>
                <label className="block text-sm text-[var(--text-secondary)] mb-2 flex items-center gap-2">
                  <Calendar className="w-4 h-4" />
                  Registration Opens *
                </label>
                <DateTimePicker
                  value={formData.registration_start}
                  onChange={(val) => setFormData(prev => ({ ...prev, registration_start: val }))}
                  required
                  placeholder="Select registration open time"
                />
              </div>
              <div>
                <label className="block text-sm text-[var(--text-secondary)] mb-2 flex items-center gap-2">
                  <Calendar className="w-4 h-4" />
                  Registration Closes *
                </label>
                <DateTimePicker
                  value={formData.registration_end}
                  onChange={(val) => setFormData(prev => ({ ...prev, registration_end: val }))}
                  required
                  placeholder="Select registration close time"
                />
              </div>
            </div>

            <div className="grid sm:grid-cols-1 gap-4">
              <div>
                <label className="block text-sm text-[var(--text-secondary)] mb-2 flex items-center gap-2">
                  <Calendar className="w-4 h-4" />
                  Auction Starts *
                </label>
                <DateTimePicker
                  value={formData.auction_start}
                  onChange={(val) => setFormData(prev => ({ ...prev, auction_start: val }))}
                  required
                  placeholder="Select auction start time"
                />
              </div>
            </div>
          </section>

          {/* Settings */}
          <section className="space-y-4">
            <h2 className="text-lg font-bold text-white flex items-center gap-2">
              <span className="w-8 h-8 rounded-lg bg-[var(--gold)]/20 flex items-center justify-center text-sm">4</span>
              Settings
            </h2>
            
            <div className="grid sm:grid-cols-2 gap-4">
              <div>
                <label className="block text-sm text-[var(--text-secondary)] mb-2 flex items-center gap-2">
                  <Users className="w-4 h-4" />
                  Max Participants
                </label>
                <input
                  type="number"
                  name="max_participants"
                  value={formData.max_participants}
                  onChange={handleChange}
                  min="1"
                  placeholder="Unlimited"
                  onWheel={(e) => e.currentTarget.blur()}
                  className="w-full"
                />
                <p className="text-xs text-[var(--text-muted)] mt-1">Leave empty for unlimited</p>
              </div>
              <div>
                <label className="block text-sm text-[var(--text-secondary)] mb-2 flex items-center gap-2">
                  <span className="font-bold">#</span>
                  Auction Password *
                </label>
                <input
                  type="text"
                  name="password"
                  value={formData.password}
                  onChange={handleChange}
                  maxLength={4}
                  pattern="\d{4}"
                  placeholder="e.g. 1234"
                  required
                  className="w-full"
                />
                <p className="text-xs text-[var(--text-muted)] mt-1">4-digit numeric password</p>
              </div>
              <div>
                <label className="block text-sm text-[var(--text-secondary)] mb-2 flex items-center gap-2">
                  <DollarSign className="w-4 h-4" />
                  Entry Fee ($)
                </label>
                <input
                  type="number"
                  name="entry_fee"
                  value={formData.entry_fee}
                  onChange={handleChange}
                  min="0"
                  step="0.01"
                  onWheel={(e) => e.currentTarget.blur()}
                  className="w-full"
                />
              </div>
            </div>
          </section>

          <div className="flex items-center gap-4 pt-6 border-t border-[var(--border)]">
            <button
              type="submit"
              disabled={loading}
              className="btn-gold flex items-center gap-2"
            >
              {loading ? (
                <>
                  <Loader2 className="w-5 h-5 animate-spin" />
                  <span>Creating...</span>
                </>
              ) : (
                <span>Create Auction</span>
              )}
            </button>
            <Link href="/admin/auctions" className="btn-outline">
              Cancel
            </Link>
          </div>
          </fieldset>
        </form>
      </div>
    </div>
  )
}