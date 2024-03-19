#include "UniverseSimulation.h"

#include "Components.h"

#include <godot_cpp/variant/utility_functions.hpp>

namespace voxel_game
{
	uint64_t UniverseSimulation::CreateInstance(godot::RID mesh, godot::RID scenario)
	{
		godot::String name = godot::vformat("Instance %d", rand());

		auto entity = m_world.entity(name.utf8());

		entity.emplace<Instance>();
		entity.emplace<Scenario>(scenario);
		entity.emplace<Mesh>(mesh);
		entity.add<Position>();

		//godot::UtilityFunctions::print(godot::vformat("Created instance %s with %d %d giving %d", name, mesh.get_id(), scenario.get_id(), entity));

		return entity.id();
	}

	void UniverseSimulation::SetInstancePos(uint64_t instance_id, const godot::Vector3& pos)
	{
		auto entity = m_world.entity(instance_id);

		entity.get_mut<Position>()->position = pos;
		entity.modified<Position>();

		//godot::UtilityFunctions::print(godot::vformat("Set instance %d position to (%f, %f, %f)", instance_id, pos.x, pos.y, pos.z));
	}

	bool UniverseSimulation::DeleteInstance(uint64_t instance_id)
	{
		auto entity = m_world.entity(instance_id);

		entity.destruct();

		//godot::UtilityFunctions::print(godot::vformat("deleted entity %d", instance_id));

		return entity.is_alive();
	}
}