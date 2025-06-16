#pragma once

#include "UniverseServer.h"

namespace voxel_game
{
	struct UniverseServer::SignalStrings
	{
		SignalStrings();

		// ####### Universe #######

		godot::StringName update_debug_info;
		godot::StringName connected_to_galaxy_list;
		godot::StringName disconnected_from_galaxy_list;
		godot::StringName galaxy_list_query_response;
		godot::StringName galaxy_ping_response;
		godot::StringName connected_to_galaxy;
		godot::StringName disconnected_from_galaxy;

		// ####### Galaxy #######

		godot::StringName connected_to_remote;
		godot::StringName disonnected_from_remote;

		// ####### Fragments (admin only) #######

		godot::StringName fragment_added;
		godot::StringName fragment_removed;
		godot::StringName fragment_loaded;
		godot::StringName fragment_unloaded;

		// ####### Account #######

		godot::StringName account_create_request_response;
		godot::StringName account_login_request_response;
		godot::StringName account_delete_request_response;
		godot::StringName account_logout_response;

		// ####### Friends #######

		godot::StringName account_invite_friend_response;
		godot::StringName account_friend_request_received;
		godot::StringName account_remove_friend_response;
		godot::StringName account_friend_unfriended;

		// ####### Chat #######

		godot::StringName chat_message_received;
		godot::StringName chat_channel_info_response;

		// ####### Players #######

		godot::StringName player_joined_fragment;
		godot::StringName player_left_fragment;

		// ####### Party #######

		godot::StringName party_create_response;
		godot::StringName party_invited;
		godot::StringName party_invitation_response;
		godot::StringName party_kicked;
		godot::StringName party_player_joined;
		godot::StringName party_player_left;

		// ####### Entity #######

		godot::StringName entity_info_request_response;

		// ####### Volume (is entity) #######

		godot::StringName volume_block_place_response;
		godot::StringName volume_block_fill_response;
		godot::StringName new_volume_block_place_response;
		godot::StringName volume_block_interact_response;

		// ####### Galaxy Region #######

		godot::StringName galaxy_region_info_request_response;
		godot::StringName galaxy_region_entered;
		godot::StringName galaxy_region_exited;

		// ####### Galaxy Object (is volume) #######

		godot::StringName galaxy_object_info_request_response;

		// ####### Currency #######

		godot::StringName currency_withdraw_response;
		godot::StringName currency_deposit_response;
		godot::StringName currency_convert_response;
		godot::StringName currency_pay_response;
		godot::StringName currency_buy_good_response;

		// ####### Internet #######

		godot::StringName internet_start_response;
		godot::StringName internet_stop_response;
		godot::StringName internet_url_request_response;

		// ####### Faction Roles #######

		godot::StringName faction_role_add_response;
		godot::StringName faction_role_remove_response;
		godot::StringName faction_role_modify_response;
		godot::StringName faction_permission_add_response;
		godot::StringName faction_permission_remove_response;
		godot::StringName faction_set_entity_role_response;

		// ####### Faction (is entity) #######

		godot::StringName faction_join_response;
		godot::StringName faction_leave_response;
		godot::StringName faction_entity_invite_response;
		godot::StringName faction_invite_received;
		godot::StringName faction_entity_kick_response;
		godot::StringName faction_child_add_response;
		godot::StringName faction_child_remove_response;
		godot::StringName faction_child_invite_response;
		godot::StringName faction_kick_response;

		// ####### Player Faction (is faction) #######

		godot::StringName faction_request_join_response;

		// ####### Level #######

		godot::StringName levelup_available;
		godot::StringName complete_levelup_response;

		// ####### Looking at #######

		godot::StringName hovered_over_entity;
		godot::StringName hovered_over_volume;
		godot::StringName hovered_over_block;

		// ####### Inventory #######

		godot::StringName inventory_trashed_item_response;
		godot::StringName inventory_moved_item_response;
		godot::StringName inventory_transfer_item_response;
		godot::StringName inventory_interact_item_response;
		godot::StringName inventory_force_closed;

		// ####### Interact #######

		godot::StringName entity_store_response;
		godot::StringName block_hold_response;
		godot::StringName entity_drop_response;

		godot::StringName equip_from_world_response;
		godot::StringName equip_from_inventory_response;
		godot::StringName drop_equip_to_world_response;
		godot::StringName unequip_to_inventory_response;
		godot::StringName set_left_hand_equip_response;
		godot::StringName set_right_hand_equip_response;
		godot::StringName use_equip_response;
		godot::StringName toggle_equip_response;

		godot::StringName ride_entity_response;
		godot::StringName change_ride_attachment_response;
		godot::StringName exit_ride_response;
		godot::StringName ride_force_exited;

		godot::StringName entity_interact_response;

		// ####### Vehicle Control #######

		godot::StringName vehicle_accelerate_response;
		godot::StringName vehicle_deccelerate_response;
		godot::StringName vehicle_control_activate_response;
		godot::StringName vehicle_control_toggle_response;
		godot::StringName vehicle_set_setting_response;

		// ####### Abilities #######

		godot::StringName activate_ability_response;
		godot::StringName toggle_ability_response;
		godot::StringName player_set_setting_response;

		// ####### Magic #######

		godot::StringName use_spell_response_response;
	};
}