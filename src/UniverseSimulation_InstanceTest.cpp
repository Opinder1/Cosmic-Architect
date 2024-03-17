#include "UniverseSimulation.h"

#include "Components.h"

namespace voxel_game
{
	uint64_t UniverseSimulation::CreateInstance(godot::RID mesh, godot::RID scenario)
	{
		auto entity = m_world.entity();

		entity.emplace<Scenario>(scenario);
		entity.emplace<Mesh>(mesh);
		entity.add<Instance>();
		entity.add<Position>();

		return entity.id();
	}

	void UniverseSimulation::SetInstancePos(uint64_t instance_id, const godot::Vector3& pos)
	{
		auto entity = m_world.entity(instance_id);

		entity.get_mut<Position>()->position = pos;
		entity.modified<Position>();
	}

	bool UniverseSimulation::DeleteInstance(uint64_t instance_id)
	{
		auto entity = m_world.entity(instance_id);

		entity.destruct();

		return entity.is_alive();
	}
}