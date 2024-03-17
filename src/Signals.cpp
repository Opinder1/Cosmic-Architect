#include "Signals.h"

namespace voxel_game
{
	godot::PropertyInfo MakeEnumProperty(const godot::StringName& name, const godot::String& enum_string)
	{
		godot::PropertyInfo info;

		info.type = godot::Variant::INT;
		info.name = name;
		info.class_name = godot::String();
		info.hint = godot::PROPERTY_HINT_NONE;
		info.hint_string = godot::enum_qualified_name_to_class_info_name(enum_string);
		info.usage = godot::PROPERTY_USAGE_DEFAULT | godot::PROPERTY_USAGE_CLASS_IS_ENUM;

		return info;
	}
	
	std::unique_ptr<const UniverseSimulation::Signals> UniverseSimulation::k_signals;

	UniverseSimulation::Signals::Signals()
	{
#define INITIALIZE_SIGNAL(name) name = godot::StringName{ #name, true }

		// ####### Universe #######

		INITIALIZE_SIGNAL(connected_to_galaxy_list);
		INITIALIZE_SIGNAL(disconnected_from_galaxy_list);
		INITIALIZE_SIGNAL(galaxy_list_query_response);
		INITIALIZE_SIGNAL(galaxy_ping_response);

		// ####### Galaxy #######

		INITIALIZE_SIGNAL(local_galaxy_started);
		INITIALIZE_SIGNAL(local_fragment_started);
		INITIALIZE_SIGNAL(local_galaxy_stopped);
		INITIALIZE_SIGNAL(connected_to_remote_galaxy);
		INITIALIZE_SIGNAL(disonnected_from_remote_galaxy);
		INITIALIZE_SIGNAL(galaxy_load_state_changed);

		// ####### Fragments (admin only) #######

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

#undef INITIALIZE_SIGNAL
	}

	void UniverseSimulation::BindSignals()
	{
#define ENUM_PROPERTY(name, enum_) MakeEnumProperty(name, #enum_)

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
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_load_state_changed, ENUM_PROPERTY("state", UniverseSimulation::LoadState)));
		
		// ####### Fragments (admin only) #######

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

#undef ENUM_PROPERTY
	}
}