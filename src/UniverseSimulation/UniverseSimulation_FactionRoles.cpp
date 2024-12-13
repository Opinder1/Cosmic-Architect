#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetRoleInfo(const UUID& role_id)
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

	godot::Dictionary UniverseSimulation::GetPermissionInfo(const UUID& permission_id)
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

	UUID UniverseSimulation::GetEntityRole(const UUID& faction_id, const UUID& entity_id)
	{
		godot::Dictionary entity = GetEntityInfo(entity_id);

		if (entity.is_empty())
		{
			return UUID{};
		}

		godot::Dictionary entity_factions = entity.find_key("factions");
		
		if (entity_factions.is_empty())
		{
			return UUID{};
		}
		
		return entity_factions.find_key(faction_id);
	}

	void UniverseSimulation::AddFactionRole(const UUID& faction_id, const UUID& role_id, const godot::Dictionary& role_info)
	{
		if (DeferCommand<&UniverseSimulation::AddFactionRole>(faction_id, role_id, role_info))
		{
			return;
		}
	}

	void UniverseSimulation::RemoveFactionRole(const UUID& faction_id, const UUID& role_id)
	{
		if (DeferCommand<&UniverseSimulation::RemoveFactionRole>(faction_id, role_id))
		{
			return;
		}
	}

	void UniverseSimulation::ModifyFactionRole(const UUID& faction_id, const UUID& role_id, const godot::Dictionary& role_info)
	{
		if (DeferCommand<&UniverseSimulation::ModifyFactionRole>(faction_id, role_id, role_info))
		{
			return;
		}
	}

	void UniverseSimulation::AddPermissionToRole(const UUID& faction_id, const UUID& role_id, const UUID& permission_id)
	{
		if (DeferCommand<&UniverseSimulation::AddPermissionToRole>(faction_id, role_id, permission_id))
		{
			return;
		}
	}

	void UniverseSimulation::RemovePermissionFromRole(const UUID& faction_id, const UUID& role_id, const UUID& permission_id)
	{
		if (DeferCommand<&UniverseSimulation::RemovePermissionFromRole>(faction_id, role_id, permission_id))
		{
			return;
		}
	}

	void UniverseSimulation::SetEntityRole(const UUID& faction_id, const UUID& entity_id, const UUID& role_id)
	{
		if (DeferCommand<&UniverseSimulation::SetEntityRole>(faction_id, entity_id, role_id))
		{
			return;
		}
	}

	bool UniverseSimulation::EntityHasPermission(const UUID& faction_id, const UUID& entity_id, const UUID& permission_id)
	{
		// TODO: Have some better return value than just a bool so we can also have an error return value

		UUID entity_role = GetEntityRole(faction_id, entity_id);

		if (entity_role == UUID())
		{
			return false;
		}

		godot::Dictionary faction_role = GetRoleInfo(entity_role);

		if (faction_role.is_empty())
		{
			return false;
		}

		UUIDVector permissions = faction_role.find_key("permissions");

		if (permissions.is_empty())
		{
			return false;
		}

		return permissions.find(permission_id);
	}
}