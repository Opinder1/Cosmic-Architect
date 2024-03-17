#include "UniverseSimulation.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetFactionInfo(UUID faction_id)
	{
		return {};
	}

	UniverseSimulation::UUIDVector UniverseSimulation::GetJoinedFactions()
	{
		return {};
	}

	void UniverseSimulation::JoinFaction(UUID faction_id, const godot::Dictionary& request_info)
	{

	}

	void UniverseSimulation::LeaveFaction(UUID faction_id)
	{

	}

	void UniverseSimulation::InviteEntityToFaction(UUID faction_id, UUID entity_id)
	{

	}

	void UniverseSimulation::KickEntityFromFaction(UUID faction_id, UUID entity_id)
	{

	}

	void UniverseSimulation::AddChildFactionFaction(UUID parent_faction_id, UUID child_faction_id)
	{

	}

	void UniverseSimulation::RemoveChildFaction(UUID faction_id)
	{

	}

	void UniverseSimulation::InviteChildFaction(UUID parent_faction_id, UUID child_faction_id)
	{

	}

	void UniverseSimulation::KickChildFaction(UUID faction_id)
	{

	}
}