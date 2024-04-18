#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetRoleInfo(UUID role_id)
	{
		std::shared_lock lock(m_cache_mutex);
		
		auto it = m_read_cache.role_info_map.find(role_id);

		if (it != m_read_cache.role_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	godot::Dictionary UniverseSimulation::GetPermissionInfo(UUID permission_id)
	{
		std::shared_lock lock(m_cache_mutex);

		auto it = m_read_cache.permission_info_map.find(permission_id);

		if (it != m_read_cache.permission_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	UniverseSimulation::UUID UniverseSimulation::GetEntityRole(UUID faction_id, UUID entity_id)
	{
		std::shared_lock lock(m_cache_mutex);

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

	void UniverseSimulation::AddFactionRole(UUID faction_id, UUID role_id, const godot::Dictionary& role_info)
	{
		SIM_DEFER_COMMAND(k_commands->add_faction_role, faction_id, role_id, role_info);
	}

	void UniverseSimulation::RemoveFactionRole(UUID faction_id, UUID role_id)
	{
		SIM_DEFER_COMMAND(k_commands->remove_faction_role, faction_id, role_id);
	}

	void UniverseSimulation::ModifyFactionRole(UUID faction_id, UUID role_id, const godot::Dictionary& role_info)
	{
		SIM_DEFER_COMMAND(k_commands->modify_faction_role, faction_id, role_id, role_info);
	}

	void UniverseSimulation::AddPermissionToRole(UUID faction_id, UUID role_id, UUID permission_id)
	{
		SIM_DEFER_COMMAND(k_commands->add_permission_to_role, faction_id, role_id, permission_id);
	}

	void UniverseSimulation::RemovePermissionFromRole(UUID faction_id, UUID role_id, UUID permission_id)
	{
		SIM_DEFER_COMMAND(k_commands->remove_permission_from_role, faction_id, role_id, permission_id);
	}

	void UniverseSimulation::SetEntityRole(UUID faction_id, UUID entity_id, UUID role_id)
	{
		SIM_DEFER_COMMAND(k_commands->set_entity_role, faction_id, entity_id, role_id);
	}

	bool UniverseSimulation::EntityHasPermission(UUID faction_id, UUID entity_id, UUID permission_id)
	{
		return false;
	}
}