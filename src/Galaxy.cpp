#include "UniverseSimulation.h"
#include "Callbacks.h"
#include "Signals.h"

#include "Components.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetGalaxyInfo()
	{
		return {};
	}

	void UniverseSimulation::StartLocalGalaxy(const godot::String& galaxy_path, const godot::Callable& loaded_callback)
	{
		m_world.reset();
		m_path = galaxy_path;
		m_callbacks->start_local_galaxy.push_back(loaded_callback);

		emit_signal("galaxy_started");
	}

	void UniverseSimulation::StartLocalFragment(const godot::String& fragment_path, const godot::String& fragment_type)
	{
		m_world.reset();
		m_path = fragment_path;
		m_fragment_type = fragment_type;
	}

	void UniverseSimulation::StartRemoteGalaxy(const godot::String& galaxy_path)
	{
		m_world.reset();
		m_path = galaxy_path;
	}

	void UniverseSimulation::StopGalaxy()
	{
		m_world.reset();
		m_path = godot::String();
	}

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