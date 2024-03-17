#pragma once

#include "UniverseSimulation.h"

#include <godot_cpp/variant/string_name.hpp>

namespace voxel_game
{
	struct UniverseSimulation::Signals
	{
		Signals();

		// ####### Universe #######

		godot::StringName connected_to_galaxy_list;
		godot::StringName disconnected_from_galaxy_list;
		godot::StringName galaxy_list_query_response;
		godot::StringName galaxy_ping_response;

		// ####### Galaxy #######

		godot::StringName local_galaxy_started;
		godot::StringName local_fragment_started;
		godot::StringName local_galaxy_stopped;
		godot::StringName connected_to_remote_galaxy;
		godot::StringName disonnected_from_remote_galaxy;
		godot::StringName galaxy_load_state_changed;

		// ####### Account #######

		godot::StringName account_create_request_response;
		godot::StringName account_login_request_response;
		godot::StringName account_delete_request_response;
		godot::StringName account_logout_response;

		// ####### Friends #######

		godot::StringName account_invite_friend_response;
		godot::StringName account_invite_response;
		godot::StringName account_remove_friend_response;
		godot::StringName account_friend_unfriended;

		// ####### Chat #######

		godot::StringName chat_message_received;
		godot::StringName chat_channel_info_response;

		// ####### Galaxy Region #######

		// ####### Fragments #######

		// ####### Players #######

		// ####### Party #######
	};
}