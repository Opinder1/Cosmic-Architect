#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetRoleInfo(const ID& role_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		
		auto it = m_info_cache.role_info_map.find(role_id);

		if (it != m_info_cache.role_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	godot::Dictionary UniverseServer::GetPermissionInfo(const ID& permission_id)
	{
		std::shared_lock lock(m_info_cache.mutex);

		auto it = m_info_cache.permission_info_map.find(permission_id);

		if (it != m_info_cache.permission_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	ID UniverseServer::GetEntityRole(const ID& faction_id, const ID& entity_id)
	{
		godot::Dictionary entity = GetEntityInfo(entity_id);

		if (entity.is_empty())
		{
			return ID{};
		}

		godot::Dictionary entity_factions = entity.find_key("factions");
		
		if (entity_factions.is_empty())
		{
			return ID{};
		}
		
		return entity_factions.find_key(faction_id);
	}

	void UniverseServer::AddFactionRole(const ID& faction_id, const ID& role_id, const godot::Dictionary& role_info)
	{
		if (DeferCommand<&UniverseServer::AddFactionRole>(faction_id, role_id, role_info))
		{
			return;
		}
	}

	void UniverseServer::RemoveFactionRole(const ID& faction_id, const ID& role_id)
	{
		if (DeferCommand<&UniverseServer::RemoveFactionRole>(faction_id, role_id))
		{
			return;
		}
	}

	void UniverseServer::ModifyFactionRole(const ID& faction_id, const ID& role_id, const godot::Dictionary& role_info)
	{
		if (DeferCommand<&UniverseServer::ModifyFactionRole>(faction_id, role_id, role_info))
		{
			return;
		}
	}

	void UniverseServer::AddPermissionToRole(const ID& faction_id, const ID& role_id, const ID& permission_id)
	{
		if (DeferCommand<&UniverseServer::AddPermissionToRole>(faction_id, role_id, permission_id))
		{
			return;
		}
	}

	void UniverseServer::RemovePermissionFromRole(const ID& faction_id, const ID& role_id, const ID& permission_id)
	{
		if (DeferCommand<&UniverseServer::RemovePermissionFromRole>(faction_id, role_id, permission_id))
		{
			return;
		}
	}

	void UniverseServer::SetEntityRole(const ID& faction_id, const ID& entity_id, const ID& role_id)
	{
		if (DeferCommand<&UniverseServer::SetEntityRole>(faction_id, entity_id, role_id))
		{
			return;
		}
	}

	bool UniverseServer::EntityHasPermission(const ID& faction_id, const ID& entity_id, const ID& permission_id)
	{
		// TODO: Have some better return value than just a bool so we can also have an error return value

		ID entity_role = GetEntityRole(faction_id, entity_id);

		if (entity_role == ID())
		{
			return false;
		}

		godot::Dictionary faction_role = GetRoleInfo(entity_role);

		if (faction_role.is_empty())
		{
			return false;
		}

		IDVector permissions = faction_role.find_key("permissions");

		if (permissions.is_empty())
		{
			return false;
		}

		return permissions.find(permission_id);
	}
}