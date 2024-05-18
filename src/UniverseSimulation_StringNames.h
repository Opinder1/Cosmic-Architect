#include "UniverseSimulation.h"

namespace voxel_game
{
	struct UniverseSimulation::CommandStrings
	{
		CommandStrings();

		godot::StringName get_universe;
		godot::StringName get_galaxy_info;
		godot::StringName start_renderer;
		godot::StringName start_simulation;
		godot::StringName stop_simulation;
		godot::StringName progress;

		// ####### Fragments (admin only) #######

		godot::StringName get_fragment_info;
		godot::StringName get_current_fragment;
		godot::StringName enter_fragment;

		// ####### Account #######

		godot::StringName get_account_info;
		godot::StringName create_account;
		godot::StringName account_login;
		godot::StringName saved_session_login;
		godot::StringName clear_saved_session;
		godot::StringName delete_account;
		godot::StringName logout_account;

		// ####### Friends #######

		godot::StringName get_friends;
		godot::StringName invite_friend;
		godot::StringName accept_friend_invite;
		godot::StringName remove_friend;

		// ####### Chat #######

		godot::StringName get_channel_info;
		godot::StringName send_message_to_channel;
		godot::StringName send_message_to_player;
		godot::StringName get_chat_channel_history;
		godot::StringName get_private_chat_history;

		// ####### Players #######

		godot::StringName get_player_info;

		// ####### Party #######

		godot::StringName get_party_info;
		godot::StringName create_party;
		godot::StringName invite_to_party;
		godot::StringName accept_invite;
		godot::StringName kick_from_party;
		godot::StringName leave_party;
		godot::StringName get_players_in_party;
		godot::StringName get_party_chat_channel;

		// ####### Entity #######

		godot::StringName get_entity_info;
		godot::StringName request_entity_info;

		// ####### Volume (is entity) #######

		godot::StringName get_volume_info;
		godot::StringName get_block_info;
		godot::StringName place_block;
		godot::StringName fill_blocks;
		godot::StringName place_block_in_new_volume;
		godot::StringName interact_block;
		godot::StringName get_entity_position_in_volume;
		godot::StringName fragment_position_to_volume_position;
		godot::StringName volume_position_to_fragment_position;

		// ####### Galaxy Region #######

		godot::StringName get_galaxy_region_info;
		godot::StringName request_galaxy_region_info;
		godot::StringName get_current_galaxy_regions;

		// ####### Galaxy Object (is volume) #######

		godot::StringName get_galaxy_object_info;
		godot::StringName request_galaxy_object_info;

		// ####### Currency #######

		godot::StringName get_currency_info;
		godot::StringName get_bank_info;
		godot::StringName get_bank_interface_info;
		godot::StringName get_good_info;
		godot::StringName get_universal_currency;
		godot::StringName get_bank_of_interface;
		godot::StringName get_owned_currencies;
		godot::StringName get_balance;
		godot::StringName withdraw;
		godot::StringName deposit;
		godot::StringName convert;
		godot::StringName pay_entity;
		godot::StringName buy_good_with_currency;

		// ####### Internet #######

		godot::StringName get_internet_info;
		godot::StringName get_website_info;
		godot::StringName get_website_page_info;
		godot::StringName get_internet_websites;
		godot::StringName get_website_pages;
		godot::StringName start_internet;
		godot::StringName stop_internet;
		godot::StringName get_current_internet;
		godot::StringName get_current_internet_site;
		godot::StringName get_current_internet_page;
		godot::StringName request_internet_url;

		// ####### Faction Roles #######

		godot::StringName get_role_info;
		godot::StringName get_permission_info;
		godot::StringName get_entity_role;
		godot::StringName add_faction_role;
		godot::StringName remove_faction_role;
		godot::StringName modify_faction_role;
		godot::StringName add_permission_to_role;
		godot::StringName remove_permission_from_role;
		godot::StringName set_entity_role;
		godot::StringName entity_has_permission;

		// ####### Faction (is entity) #######

		godot::StringName get_faction_info;
		godot::StringName get_joined_factions;
		godot::StringName join_faction;
		godot::StringName leave_faction;
		godot::StringName invite_entity_to_faction;
		godot::StringName kick_entity_from_faction;
		godot::StringName add_child_faction;
		godot::StringName remove_child_faction;
		godot::StringName invite_child_faction;
		godot::StringName kick_child_faction;

		// ####### Player Faction (is faction) #######

		godot::StringName get_global_player_faction;
		godot::StringName get_player_faction;
		godot::StringName request_join_player_faction;

		// ####### Language #######

		godot::StringName get_language_info;
		godot::StringName get_language_translation;
		godot::StringName get_language_string;

		// ####### Culture #######

		godot::StringName get_culture_info;

		// ####### Level #######

		godot::StringName get_level;
		godot::StringName get_experience;
		godot::StringName complete_level_up;

		// ####### Player Control #######

		godot::StringName set_move_forwards;
		godot::StringName set_move_backwards;
		godot::StringName set_move_left;
		godot::StringName set_move_right;
		godot::StringName set_move_up;
		godot::StringName set_move_down;
		godot::StringName set_rotate_left;
		godot::StringName set_rotate_right;
		godot::StringName set_look_direction;
		godot::StringName set_sprint;
		godot::StringName set_crouching;
		godot::StringName set_prone;
		godot::StringName jump;

		// ####### Looking at #######

		godot::StringName get_looking_at_entity;
		godot::StringName get_looking_at_volume;
		godot::StringName get_looking_at_block;

		// ####### Inventory #######

		godot::StringName get_inventory_info;
		godot::StringName get_inventory;
		godot::StringName get_inventory_item_entity;
		godot::StringName trash_inventory_item;
		godot::StringName move_inventory_item;
		godot::StringName transfer_inventory_item;
		godot::StringName interact_with_inventory_item;

		// ####### Interact #######
		godot::StringName store_entity;
		godot::StringName hold_block;
		godot::StringName hold_entity;
		godot::StringName drop_held_entity;

		godot::StringName equip_item_from_world;
		godot::StringName equip_item_from_inventory;
		godot::StringName drop_equip_to_world;
		godot::StringName unequip_item_to_inventory;
		godot::StringName set_left_hand_equip;
		godot::StringName set_right_hand_equip;
		godot::StringName use_equip;
		godot::StringName toggle_equip;

		godot::StringName ride_entity;
		godot::StringName change_attachment_point;
		godot::StringName exit_entity;

		godot::StringName interact_with_entity;

		// ####### Vehicle Control #######

		godot::StringName accelerate;
		godot::StringName decelerate;
		godot::StringName activate_vehicle_control;
		godot::StringName toggle_vehicle_control;
		godot::StringName set_vehicle_setting;

		// ####### Abilities #######

		godot::StringName get_ability_info;
		godot::StringName activate_ability;
		godot::StringName toggle_ability;
		godot::StringName set_player_setting;

		// ####### Magic #######

		godot::StringName get_spell_info;
		godot::StringName use_spell;
	};

	struct UniverseSimulation::SignalStrings
	{
		SignalStrings();

		godot::StringName simulation_uninitialized;
		godot::StringName simulation_started;
		godot::StringName simulation_stopped;
		godot::StringName connected_to_remote;
		godot::StringName disonnected_from_remote;
		godot::StringName load_state_changed;

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