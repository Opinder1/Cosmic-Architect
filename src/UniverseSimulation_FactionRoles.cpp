#include "UniverseSimulation.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetRoleInfo(UUID role_id)
	{
		return {};
	}

	godot::Dictionary UniverseSimulation::GetPermissionInfo(UUID permission_id)
	{
		return {};
	}

	UniverseSimulation::UUID UniverseSimulation::GetEntityRole(UUID faction_id, UUID entity_id)
	{
		return {};
	}

	void UniverseSimulation::AddFactionRole(UUID faction_id, UUID role_id, const godot::Dictionary& role_info)
	{

	}

	void UniverseSimulation::RemoveFactionRole(UUID faction_id, UUID role_id)
	{

	}

	void UniverseSimulation::ModifyFactionRole(UUID faction_id, UUID role_id, const godot::Dictionary& role_info)
	{

	}

	void UniverseSimulation::AddPermissionToRole(UUID faction_id, UUID role_id, UUID permission_id)
	{

	}

	void UniverseSimulation::RemovePermissionFromRole(UUID faction_id, UUID role_id, UUID permission_id)
	{

	}

	void UniverseSimulation::SetEntityRole(UUID faction_id, UUID entity_id, UUID role_id)
	{

	}

	bool UniverseSimulation::EntityHasPermission(UUID faction_id, UUID entity_id, UUID permission_id)
	{
		return false;
	}
}