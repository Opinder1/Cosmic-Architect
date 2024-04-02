#include "Universe.h"
#include "UniverseSimulation.h"

#include "Util/PropertyMacros.h"
#include "Util/Debug.h"

#include <godot_cpp/variant/utility_functions.hpp>

namespace voxel_game
{
	std::unique_ptr<const Universe::Signals> Universe::k_signals;

	Universe::Signals::Signals()
	{
#define INITIALIZE_SIGNAL(name) name = godot::StringName{ #name }

		INITIALIZE_SIGNAL(connected_to_galaxy_list);
		INITIALIZE_SIGNAL(disconnected_from_galaxy_list);
		INITIALIZE_SIGNAL(galaxy_list_query_response);
		INITIALIZE_SIGNAL(galaxy_ping_response);
		INITIALIZE_SIGNAL(simulation_load_state_changed);

#undef INITIALIZE_SIGNAL
	}

	Universe::Universe()
	{}

	Universe::~Universe()
	{}

	godot::Dictionary Universe::GetUniverseInfo()
	{
		return m_universe_info_cache;
	}

	void Universe::ConnectToGalaxyList(const godot::String& ip)
	{

	}

	void Universe::DisconnectFromGalaxyList()
	{

	}

	void Universe::QueryGalaxyList(const godot::Dictionary& query)
	{

	}

	void Universe::PingGalaxy(const godot::String& ip)
	{

	}

	godot::Ref<UniverseSimulation> Universe::StartLocalGalaxy(const godot::String& galaxy_path)
	{
		godot::Ref<UniverseSimulation> simulation;

		simulation.instantiate();

		simulation->Initialize(this, galaxy_path, "full_galaxy", false);

		simulation->connect(UniverseSimulation::k_signals->load_state_changed, godot::create_custom_callable_function_pointer(this, &Universe::SimulationStateChanged).bind(simulation));
		
		return simulation;
	}

	godot::Ref<UniverseSimulation> Universe::StartLocalFragment(const godot::String& fragment_path, const godot::String& fragment_type)
	{
		godot::Ref<UniverseSimulation> simulation;

		simulation.instantiate();

		simulation->Initialize(this, fragment_path, fragment_type, false);

		return simulation;
	}

	godot::Ref<UniverseSimulation> Universe::StartRemoteGalaxy(const godot::String& galaxy_path)
	{
		godot::Ref<UniverseSimulation> simulation;

		simulation.instantiate();

		simulation->Initialize(this, galaxy_path, "full_galaxy", true);

		return simulation;
	}

	void Universe::SimulationStateChanged(uint64_t load_state, const godot::Ref<UniverseSimulation>& simulation)
	{
		emit_signal(k_signals->simulation_load_state_changed, simulation, load_state);
	}

	void Universe::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("get_universe_info"), &Universe::GetUniverseInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("connect_to_galaxy_list", "ip"), &Universe::ConnectToGalaxyList);
		godot::ClassDB::bind_method(godot::D_METHOD("disconnect_from_galaxy_list"), &Universe::DisconnectFromGalaxyList);
		godot::ClassDB::bind_method(godot::D_METHOD("query_galaxy_list", "query"), &Universe::QueryGalaxyList);
		godot::ClassDB::bind_method(godot::D_METHOD("ping_galaxy", "ip"), &Universe::PingGalaxy);

		godot::ClassDB::bind_method(godot::D_METHOD("start_local_galaxy", "galaxy_path"), &Universe::StartLocalGalaxy);
		godot::ClassDB::bind_method(godot::D_METHOD("start_local_fragment", "fragment_path", "fragment_type"), &Universe::StartLocalFragment);
		godot::ClassDB::bind_method(godot::D_METHOD("start_remote_galaxy", "galaxy_path"), &Universe::StartRemoteGalaxy);

		k_signals = std::make_unique<const Signals>();

		ADD_SIGNAL(godot::MethodInfo(k_signals->connected_to_galaxy_list));
		ADD_SIGNAL(godot::MethodInfo(k_signals->disconnected_from_galaxy_list));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_list_query_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_ping_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->simulation_load_state_changed, ENUM_PROPERTY("state", UniverseSimulation::LoadState), OBJECT_PROPERTY("simulation", UniverseSimulation)));
	}

	void Universe::_cleanup_methods()
	{
		k_signals.reset();
	}
}