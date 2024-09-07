#include "Universe.h"
#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

#include "Util/PropertyMacros.h"
#include "Util/Debug.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#define BIND_METHOD godot::ClassDB::bind_method

#define INITIALIZE_STRINGNAME(name) name = godot::StringName{ #name }

namespace std
{
	size_t hash<godot::Ref<voxel_game::UniverseSimulation>>::operator()(const godot::Ref<voxel_game::UniverseSimulation>& simulation) const
	{
		return reinterpret_cast<uintptr_t>(simulation.ptr());
	}
}

namespace voxel_game
{
	std::optional<const Universe::SignalStrings> Universe::k_signals;

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

	godot::Ref<UniverseSimulation> Universe::InitializeLocalGalaxy(const godot::String& galaxy_path)
	{
		godot::Ref<UniverseSimulation> simulation;

		simulation.instantiate();

		simulation->connect(UniverseSimulation::k_signals->load_state_changed, godot::create_custom_callable_function_pointer(this, &Universe::OnSimulationStateChanged).bind(simulation));
		simulation->connect(UniverseSimulation::k_signals->simulation_uninitialized, godot::create_custom_callable_function_pointer(this, &Universe::OnSimulationUninitialized).bind(simulation));

		simulation->Initialize(this, galaxy_path, "full_galaxy", UniverseSimulation::SERVER_TYPE_LOCAL, scenario);

		m_simulations.emplace(simulation);
		
		return simulation;
	}

	godot::Ref<UniverseSimulation> Universe::InitializeLocalFragment(const godot::String& fragment_path, const godot::String& fragment_type)
	{
		godot::Ref<UniverseSimulation> simulation;

		simulation.instantiate();

		simulation->Initialize(this, fragment_path, fragment_type, UniverseSimulation::SERVER_TYPE_LOCAL, godot::RID());

		m_simulations.emplace(simulation);

		return simulation;
	}

	godot::Ref<UniverseSimulation> Universe::InitializeRemoteGalaxy(const godot::String& galaxy_path)
	{
		godot::Ref<UniverseSimulation> simulation;

		simulation.instantiate();

		simulation->Initialize(this, galaxy_path, "full_galaxy", UniverseSimulation::SERVER_TYPE_REMOTE, godot::RID());

		m_simulations.emplace(simulation);

		return simulation;
	}

	void Universe::Uninitialize(const godot::Ref<UniverseSimulation>& simulation)
	{
		simulation->Uninitialize();
	}

	void Universe::OnSimulationStateChanged(uint64_t load_state, const godot::Ref<UniverseSimulation>& simulation)
	{
		switch (load_state)
		{
		case UniverseSimulation::LOAD_STATE_LOADING:
			emit_signal(k_signals->galaxy_simulation_started, simulation);
			break;

		case UniverseSimulation::LOAD_STATE_UNLOADING:
			emit_signal(k_signals->galaxy_simulation_stopped, simulation);
			break;
		}
	}

	void Universe::OnSimulationUninitialized(const godot::Ref<UniverseSimulation>& simulation)
	{
		m_simulations.erase(simulation);
	}

	void Universe::_bind_methods()
	{
		k_signals.emplace();

		BIND_METHOD(godot::D_METHOD("get_universe_info"), &Universe::GetUniverseInfo);
		BIND_METHOD(godot::D_METHOD("connect_to_galaxy_list", "ip"), &Universe::ConnectToGalaxyList);
		BIND_METHOD(godot::D_METHOD("disconnect_from_galaxy_list"), &Universe::DisconnectFromGalaxyList);
		BIND_METHOD(godot::D_METHOD("query_galaxy_list", "query"), &Universe::QueryGalaxyList);
		BIND_METHOD(godot::D_METHOD("ping_galaxy", "ip"), &Universe::PingGalaxy);

		BIND_METHOD(godot::D_METHOD("initialize_local_galaxy", "galaxy_path", "scenario"), &Universe::InitializeLocalGalaxy);
		BIND_METHOD(godot::D_METHOD("initialize_local_fragment", "fragment_path", "fragment_type"), &Universe::InitializeLocalFragment);
		BIND_METHOD(godot::D_METHOD("initialize_remote_galaxy", "galaxy_path"), &Universe::InitializeRemoteGalaxy);
		BIND_METHOD(godot::D_METHOD("uninitialize", "simulation"), &Universe::Uninitialize);

		ADD_SIGNAL(godot::MethodInfo(k_signals->connected_to_galaxy_list));
		ADD_SIGNAL(godot::MethodInfo(k_signals->disconnected_from_galaxy_list));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_list_query_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_ping_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_simulation_started, OBJECT_PROPERTY("simulation", UniverseSimulation)));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_simulation_stopped, OBJECT_PROPERTY("simulation", UniverseSimulation)));
	}

	void Universe::_cleanup_methods()
	{
		k_signals.reset();
	}

	Universe::SignalStrings::SignalStrings()
	{
		INITIALIZE_STRINGNAME(connected_to_galaxy_list);
		INITIALIZE_STRINGNAME(disconnected_from_galaxy_list);
		INITIALIZE_STRINGNAME(galaxy_list_query_response);
		INITIALIZE_STRINGNAME(galaxy_ping_response);
		INITIALIZE_STRINGNAME(galaxy_simulation_started);
		INITIALIZE_STRINGNAME(galaxy_simulation_stopped);
	}
}