#include "Signals.h"

namespace voxel_game
{
	std::unique_ptr<const UniverseSimulation::Signals> UniverseSimulation::k_signals;

	UniverseSimulation::Signals::Signals()
	{
		// ####### Universe #######

		connected_to_galaxy_list = godot::StringName{ "connected_to_galaxy_list", true };
		disconnected_from_galaxy_list = godot::StringName{ "disconnected_from_galaxy_list", true };
		galaxy_list_query_response = godot::StringName{ "galaxy_list_query_response", true };
		galaxy_ping_response = godot::StringName{ "galaxy_ping_response", true };

		// ####### Galaxy #######

		local_galaxy_started = godot::StringName{ "local_galaxy_started", true };
		local_fragment_started = godot::StringName{ "local_fragment_started", true };
		local_galaxy_stopped = godot::StringName{ "local_galaxy_stopped", true };
		connected_to_remote_galaxy = godot::StringName{ "connected_to_remote_galaxy", true };
		disonnected_from_remote_galaxy = godot::StringName{ "disonnected_from_remote_galaxy", true };
		galaxy_load_state_changed = godot::StringName{ "galaxy_load_state_changed", true };
	}

	void UniverseSimulation::BindSignals()
	{
		k_signals = std::make_unique<const Signals>();

		// ####### Universe #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->connected_to_galaxy_list));
		ADD_SIGNAL(godot::MethodInfo(k_signals->disconnected_from_galaxy_list));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_list_query_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_ping_response));

		// ####### Galaxy #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->local_galaxy_started));
		ADD_SIGNAL(godot::MethodInfo(k_signals->local_fragment_started));
		ADD_SIGNAL(godot::MethodInfo(k_signals->local_galaxy_stopped));
		ADD_SIGNAL(godot::MethodInfo(k_signals->connected_to_remote_galaxy));
		ADD_SIGNAL(godot::MethodInfo(k_signals->disonnected_from_remote_galaxy));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_load_state_changed, godot::PropertyInfo{ godot::Variant::INT, "state", godot::PROPERTY_HINT_NONE, godot::String(), godot::PROPERTY_USAGE_DEFAULT | godot::PROPERTY_USAGE_CLASS_IS_ENUM, "UniverseSimulation.LoadState" }));
	}
}