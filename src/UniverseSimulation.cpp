#include "UniverseSimulation.h"
#include "Components.h"

#include "Util/Debug.h"

#include <godot_cpp/core/class_db.hpp>

namespace voxel_world
{
	size_t UniverseSimulation::UUIDHash::operator()(const UUID& uuid) const
	{
		const uint64_t(&arr)[2] = reinterpret_cast<const uint64_t(&)[2]>(uuid);

		return arr[0] ^ arr[1];
	}

	UniverseSimulation::UniverseSimulation()
	{}

	UniverseSimulation::~UniverseSimulation()
	{}

	void UniverseSimulation::StartLocalGalaxy(const godot::String& galaxy_path)
	{
		m_world.reset();
		m_path = galaxy_path;
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

	void UniverseSimulation::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("start_local_galaxy", "galaxy_path"), &UniverseSimulation::StartLocalGalaxy);
		godot::ClassDB::bind_method(godot::D_METHOD("start_local_fragment", "fragment_path", "fragment_type"), &UniverseSimulation::StartLocalFragment);
		godot::ClassDB::bind_method(godot::D_METHOD("start_remote_galaxy", "galaxy_path"), &UniverseSimulation::StartRemoteGalaxy);
		godot::ClassDB::bind_method(godot::D_METHOD("stop_galaxy"), &UniverseSimulation::StopGalaxy);

		godot::ClassDB::bind_method(godot::D_METHOD("create_instance", "mesh", "scenario"), &UniverseSimulation::CreateInstance);
		godot::ClassDB::bind_method(godot::D_METHOD("set_instance_pos", "instance_id", "pos"), &UniverseSimulation::SetInstancePos);
		godot::ClassDB::bind_method(godot::D_METHOD("delete_instance", "instance_id"), &UniverseSimulation::DeleteInstance);
	}
}