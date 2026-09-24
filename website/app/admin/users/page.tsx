import { requirePermission } from '@/lib/auth'
import { PERMISSIONS } from '@/lib/permissions'
import UsersClient from '@/components/admin/UsersClient'

export default async function UsersPage() {
  await requirePermission(PERMISSIONS.MANAGE_USERS)

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-2xl font-bold text-white">User Management</h1>
        <p className="text-[var(--text-secondary)]">
          View and manage users. Assign roles to control access levels.
        </p>
      </div>

      <UsersClient />
    </div>
  )
}
