#include "UniverseSimulation.h"
#include "Signals.h"

#include "Util/Debug.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/type_info.hpp>

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
	}

	UniverseSimulation::~UniverseSimulation()
	{}

	void UniverseSimulation::_bind_methods()
	{
		BIND_ENUM_CONSTANT(LoadState::Unloaded);
		BIND_ENUM_CONSTANT(LoadState::Loading);
		BIND_ENUM_CONSTANT(LoadState::Loaded);
		BIND_ENUM_CONSTANT(LoadState::Unloading);

		BindSignals();

		// ####### Universe #######

		godot::ClassDB::bind_method(godot::D_METHOD("start_local_galaxy", "galaxy_path"), &UniverseSimulation::StartLocalGalaxy);
		godot::ClassDB::bind_method(godot::D_METHOD("start_local_fragment", "fragment_path", "fragment_type"), &UniverseSimulation::StartLocalFragment);
		godot::ClassDB::bind_method(godot::D_METHOD("start_remote_galaxy", "galaxy_path"), &UniverseSimulation::StartRemoteGalaxy);
		godot::ClassDB::bind_method(godot::D_METHOD("stop_galaxy"), &UniverseSimulation::StopGalaxy);
		godot::ClassDB::bind_method(godot::D_METHOD("get_galaxy_load_state"), &UniverseSimulation::GetGalaxyLoadState);

		// ####### Galaxy #######

		godot::ClassDB::bind_method(godot::D_METHOD("create_instance", "mesh", "scenario"), &UniverseSimulation::CreateInstance);
		godot::ClassDB::bind_method(godot::D_METHOD("set_instance_pos", "instance_id", "pos"), &UniverseSimulation::SetInstancePos);
		godot::ClassDB::bind_method(godot::D_METHOD("delete_instance", "instance_id"), &UniverseSimulation::DeleteInstance);

		// ####### Fragments (admin only) #######

		godot::ClassDB::bind_method(godot::D_METHOD("create_instance", "fragment_id"), &UniverseSimulation::GetFragmentInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("create_instance"), &UniverseSimulation::GetCurrentFragment);
		godot::ClassDB::bind_method(godot::D_METHOD("create_instance", "fragment_id", "method"), &UniverseSimulation::EnterFragment);

		// ####### Account #######

		// ####### Friends #######

		// ####### Chat #######

		// ####### Players #######

		// ####### Party #######

		// ####### Entity #######

		// ####### Volume (is entity) #######

		// ####### Galaxy Region #######

		// ####### Galaxy Object (is volume) #######

		// ####### Currency #######

		// ####### Internet #######

		// ####### Faction Roles #######

		// ####### Faction (is entity) #######

		// ####### Player Faction (is faction) #######

		// ####### Language #######

		// ####### Culture #######

		// ####### Religion (is culture) #######

		// ####### Civilization (is faction) #######

		// ####### Level #######

		// ####### Player Control #######

		// ####### Looking at #######

		// ####### Inventory #######

		// ####### Interact with entity #######

		// ####### Vehicle Control #######

		// ####### Abilities #######

		// ####### Magic #######

		// ####### Testing #######
	}
}