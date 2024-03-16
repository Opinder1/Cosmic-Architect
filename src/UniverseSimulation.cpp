#include "UniverseSimulation.h"
#include "Callbacks.h"
#include "Signals.h"

#include "Util/Debug.h"

#include <godot_cpp/core/class_db.hpp>

namespace voxel_game
{
	size_t UniverseSimulation::UUIDHash::operator()(const UUID& uuid) const
	{
		static_assert(sizeof(size_t[2]) == sizeof(UUID));

		size_t* arr = (size_t*)&uuid;

		return arr[0] ^ arr[1];
	}

	UniverseSimulation::UniverseSimulation()
	{
		m_callbacks = std::make_unique<Callbacks>();
		m_signals = std::make_unique<Signals>();
	}

	UniverseSimulation::~UniverseSimulation()
	{}

	void UniverseSimulation::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("start_local_galaxy", "galaxy_path"), &UniverseSimulation::StartLocalGalaxy);
		godot::ClassDB::bind_method(godot::D_METHOD("start_local_fragment", "fragment_path", "fragment_type"), &UniverseSimulation::StartLocalFragment);
		godot::ClassDB::bind_method(godot::D_METHOD("start_remote_galaxy", "galaxy_path"), &UniverseSimulation::StartRemoteGalaxy);
		godot::ClassDB::bind_method(godot::D_METHOD("stop_galaxy"), &UniverseSimulation::StopGalaxy);

		godot::ClassDB::bind_method(godot::D_METHOD("create_instance", "mesh", "scenario"), &UniverseSimulation::CreateInstance);
		godot::ClassDB::bind_method(godot::D_METHOD("set_instance_pos", "instance_id", "pos"), &UniverseSimulation::SetInstancePos);
		godot::ClassDB::bind_method(godot::D_METHOD("delete_instance", "instance_id"), &UniverseSimulation::DeleteInstance);

		ADD_SIGNAL(godot::MethodInfo("galaxy_started"));
		ADD_SIGNAL(godot::MethodInfo("fragment_started", godot::PropertyInfo(godot::Variant::COLOR, "fragment_id")));
	}
}