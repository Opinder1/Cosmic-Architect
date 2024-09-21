#include "Universe.h"

#include "UniverseSimulation/UniverseSimulation.h"
#include "UniverseSimulation/UniverseSimulation_StringNames.h"

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

	void Universe::_bind_methods()
	{
		k_signals.emplace();

		BIND_METHOD(godot::D_METHOD("get_universe_info"), &Universe::GetUniverseInfo);
		BIND_METHOD(godot::D_METHOD("connect_to_galaxy_list", "ip"), &Universe::ConnectToGalaxyList);
		BIND_METHOD(godot::D_METHOD("disconnect_from_galaxy_list"), &Universe::DisconnectFromGalaxyList);
		BIND_METHOD(godot::D_METHOD("query_galaxy_list", "query"), &Universe::QueryGalaxyList);
		BIND_METHOD(godot::D_METHOD("ping_galaxy", "ip"), &Universe::PingGalaxy);

		ADD_SIGNAL(godot::MethodInfo(k_signals->connected_to_galaxy_list));
		ADD_SIGNAL(godot::MethodInfo(k_signals->disconnected_from_galaxy_list));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_list_query_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_ping_response));
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
	}
}