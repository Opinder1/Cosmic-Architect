#include "UniverseSimulation.h"

#define INITIALIZE_SIGNAL(name) name = godot::StringName{ #name }

namespace voxel_game
{
	UniverseSimulation::CommandStrings::CommandStrings()
	{
		INITIALIZE_SIGNAL(get_universe);
		INITIALIZE_SIGNAL(get_galaxy_info);
		INITIALIZE_SIGNAL(start_simulation);
		INITIALIZE_SIGNAL(stop_simulation);
		INITIALIZE_SIGNAL(progress);

		// ####### Fragments (admin only) #######

		INITIALIZE_SIGNAL(get_fragment_info);
		INITIALIZE_SIGNAL(get_current_fragment);
		INITIALIZE_SIGNAL(enter_fragment);

		// ####### Account #######

		INITIALIZE_SIGNAL(get_account_info);
		INITIALIZE_SIGNAL(create_account);
		INITIALIZE_SIGNAL(account_login);
		INITIALIZE_SIGNAL(saved_session_login);
		INITIALIZE_SIGNAL(clear_saved_session);
		INITIALIZE_SIGNAL(delete_account);
		INITIALIZE_SIGNAL(logout_account);

		// ####### Friends #######

		INITIALIZE_SIGNAL(get_friends);
		INITIALIZE_SIGNAL(invite_friend);
		INITIALIZE_SIGNAL(accept_friend_invite);
		INITIALIZE_SIGNAL(remove_friend);

		// ####### Chat #######

		INITIALIZE_SIGNAL(get_channel_info);
		INITIALIZE_SIGNAL(send_message_to_channel);
		INITIALIZE_SIGNAL(send_message_to_player);
		INITIALIZE_SIGNAL(get_chat_channel_history);
		INITIALIZE_SIGNAL(get_private_chat_history);

		// ####### Players #######

		INITIALIZE_SIGNAL(get_player_info);

		// ####### Party #######

		INITIALIZE_SIGNAL(get_party_info);
		INITIALIZE_SIGNAL(create_party);
		INITIALIZE_SIGNAL(invite_to_party);
		INITIALIZE_SIGNAL(accept_invite);
		INITIALIZE_SIGNAL(kick_from_party);
		INITIALIZE_SIGNAL(leave_party);
		INITIALIZE_SIGNAL(get_players_in_party);
		INITIALIZE_SIGNAL(get_party_chat_channel);

		// ####### Entity #######

		INITIALIZE_SIGNAL(get_entity_info);
		INITIALIZE_SIGNAL(request_entity_info);

		// ####### Volume (is entity) #######

		INITIALIZE_SIGNAL(get_volume_info);
		INITIALIZE_SIGNAL(get_block_info);
		INITIALIZE_SIGNAL(place_block);
		INITIALIZE_SIGNAL(fill_blocks);
		INITIALIZE_SIGNAL(place_block_in_new_volume);
		INITIALIZE_SIGNAL(interact_block);
		INITIALIZE_SIGNAL(get_entity_position_in_volume);
		INITIALIZE_SIGNAL(fragment_position_to_volume_position);
		INITIALIZE_SIGNAL(volume_position_to_fragment_position);

		// ####### Galaxy Region #######

		INITIALIZE_SIGNAL(get_galaxy_region_info);
		INITIALIZE_SIGNAL(request_galaxy_region_info);
		INITIALIZE_SIGNAL(get_current_galaxy_regions);

		// ####### Galaxy Object (is volume) #######

		INITIALIZE_SIGNAL(get_galaxy_object_info);
		INITIALIZE_SIGNAL(request_galaxy_object_info);

		// ####### Currency #######

		INITIALIZE_SIGNAL(get_currency_info);
		INITIALIZE_SIGNAL(get_bank_info);
		INITIALIZE_SIGNAL(get_bank_interface_info);
		INITIALIZE_SIGNAL(get_good_info);
		INITIALIZE_SIGNAL(get_universal_currency);
		INITIALIZE_SIGNAL(get_bank_of_interface);
		INITIALIZE_SIGNAL(get_owned_currencies);
		INITIALIZE_SIGNAL(get_balance);
		INITIALIZE_SIGNAL(withdraw);
		INITIALIZE_SIGNAL(deposit);
		INITIALIZE_SIGNAL(convert);
		INITIALIZE_SIGNAL(pay_entity);
		INITIALIZE_SIGNAL(buy_good_with_currency);

		// ####### Internet #######

		INITIALIZE_SIGNAL(get_internet_info);
		INITIALIZE_SIGNAL(get_website_info);
		INITIALIZE_SIGNAL(get_website_page_info);
		INITIALIZE_SIGNAL(get_internet_websites);
		INITIALIZE_SIGNAL(get_website_pages);
		INITIALIZE_SIGNAL(start_internet);
		INITIALIZE_SIGNAL(stop_internet);
		INITIALIZE_SIGNAL(get_current_internet);
		INITIALIZE_SIGNAL(get_current_internet_site);
		INITIALIZE_SIGNAL(get_current_internet_page);
		INITIALIZE_SIGNAL(request_internet_url);

		// ####### Faction Roles #######

		INITIALIZE_SIGNAL(get_role_info);
		INITIALIZE_SIGNAL(get_permission_info);
		INITIALIZE_SIGNAL(get_entity_role);
		INITIALIZE_SIGNAL(add_faction_role);
		INITIALIZE_SIGNAL(remove_faction_role);
		INITIALIZE_SIGNAL(modify_faction_role);
		INITIALIZE_SIGNAL(add_permission_to_role);
		INITIALIZE_SIGNAL(remove_permission_from_role);
		INITIALIZE_SIGNAL(set_entity_role);
		INITIALIZE_SIGNAL(entity_has_permission);

		// ####### Faction (is entity) #######

		INITIALIZE_SIGNAL(get_faction_info);
		INITIALIZE_SIGNAL(get_joined_factions);
		INITIALIZE_SIGNAL(join_faction);
		INITIALIZE_SIGNAL(leave_faction);
		INITIALIZE_SIGNAL(invite_entity_to_faction);
		INITIALIZE_SIGNAL(kick_entity_from_faction);
		INITIALIZE_SIGNAL(add_child_faction);
		INITIALIZE_SIGNAL(remove_child_faction);
		INITIALIZE_SIGNAL(invite_child_faction);
		INITIALIZE_SIGNAL(kick_child_faction);

		// ####### Player Faction (is faction) #######

		INITIALIZE_SIGNAL(get_global_player_faction);
		INITIALIZE_SIGNAL(get_player_faction);
		INITIALIZE_SIGNAL(request_join_player_faction);

		// ####### Language #######

		INITIALIZE_SIGNAL(get_language_info);
		INITIALIZE_SIGNAL(get_language_translation);
		INITIALIZE_SIGNAL(get_language_string);

		// ####### Culture #######

		INITIALIZE_SIGNAL(get_culture_info);

		// ####### Level #######

		INITIALIZE_SIGNAL(get_level);
		INITIALIZE_SIGNAL(get_experience);
		INITIALIZE_SIGNAL(complete_level_up);

		// ####### Player Control #######

		INITIALIZE_SIGNAL(set_move_forwards);
		INITIALIZE_SIGNAL(set_move_backwards);
		INITIALIZE_SIGNAL(set_move_left);
		INITIALIZE_SIGNAL(set_move_right);
		INITIALIZE_SIGNAL(set_move_up);
		INITIALIZE_SIGNAL(set_move_down);
		INITIALIZE_SIGNAL(set_rotate_left);
		INITIALIZE_SIGNAL(set_rotate_right);
		INITIALIZE_SIGNAL(set_look_direction);
		INITIALIZE_SIGNAL(set_sprint);
		INITIALIZE_SIGNAL(set_crouching);
		INITIALIZE_SIGNAL(set_prone);
		INITIALIZE_SIGNAL(jump);

		// ####### Looking at #######

		INITIALIZE_SIGNAL(get_looking_at_entity);
		INITIALIZE_SIGNAL(get_looking_at_volume);
		INITIALIZE_SIGNAL(get_looking_at_block);

		// ####### Inventory #######

		INITIALIZE_SIGNAL(get_inventory_info);
		INITIALIZE_SIGNAL(get_inventory);
		INITIALIZE_SIGNAL(get_inventory_item_entity);
		INITIALIZE_SIGNAL(trash_inventory_item);
		INITIALIZE_SIGNAL(move_inventory_item);
		INITIALIZE_SIGNAL(transfer_inventory_item);
		INITIALIZE_SIGNAL(interact_with_inventory_item);

		// ####### Interact #######
		INITIALIZE_SIGNAL(store_entity);
		INITIALIZE_SIGNAL(hold_block);
		INITIALIZE_SIGNAL(hold_entity);
		INITIALIZE_SIGNAL(drop_held_entity);

		INITIALIZE_SIGNAL(equip_item_from_world);
		INITIALIZE_SIGNAL(equip_item_from_inventory);
		INITIALIZE_SIGNAL(drop_equip_to_world);
		INITIALIZE_SIGNAL(unequip_item_to_inventory);
		INITIALIZE_SIGNAL(set_left_hand_equip);
		INITIALIZE_SIGNAL(set_right_hand_equip);
		INITIALIZE_SIGNAL(use_equip);
		INITIALIZE_SIGNAL(toggle_equip);

		INITIALIZE_SIGNAL(ride_entity);
		INITIALIZE_SIGNAL(change_attachment_point);
		INITIALIZE_SIGNAL(exit_entity);

		INITIALIZE_SIGNAL(interact_with_entity);

		// ####### Vehicle Control #######

		INITIALIZE_SIGNAL(accelerate);
		INITIALIZE_SIGNAL(decelerate);
		INITIALIZE_SIGNAL(activate_vehicle_control);
		INITIALIZE_SIGNAL(toggle_vehicle_control);
		INITIALIZE_SIGNAL(set_vehicle_setting);

		// ####### Abilities #######

		INITIALIZE_SIGNAL(get_ability_info);
		INITIALIZE_SIGNAL(activate_ability);
		INITIALIZE_SIGNAL(toggle_ability);
		INITIALIZE_SIGNAL(set_player_setting);

		// ####### Magic #######

		INITIALIZE_SIGNAL(get_spell_info);
		INITIALIZE_SIGNAL(use_spell);

		// ####### Testing #######

		INITIALIZE_SIGNAL(create_instance);
		INITIALIZE_SIGNAL(set_instance_pos);
		INITIALIZE_SIGNAL(delete_instance);
	}

	UniverseSimulation::SignalStrings::SignalStrings()
	{
		INITIALIZE_SIGNAL(simulation_started);
		INITIALIZE_SIGNAL(simulation_stopped);
		INITIALIZE_SIGNAL(connected_to_remote);
		INITIALIZE_SIGNAL(disonnected_from_remote);
		INITIALIZE_SIGNAL(load_state_changed);

		// ####### Fragments (admin only) #######

		INITIALIZE_SIGNAL(fragment_added);
		INITIALIZE_SIGNAL(fragment_removed);
		INITIALIZE_SIGNAL(fragment_loaded);
		INITIALIZE_SIGNAL(fragment_unloaded);

		// ####### Account #######

		INITIALIZE_SIGNAL(account_create_request_response);
		INITIALIZE_SIGNAL(account_login_request_response);
		INITIALIZE_SIGNAL(account_delete_request_response);
		INITIALIZE_SIGNAL(account_logout_response);

		// ####### Friends #######

		INITIALIZE_SIGNAL(account_invite_friend_response);
		INITIALIZE_SIGNAL(account_friend_request_received);
		INITIALIZE_SIGNAL(account_remove_friend_response);
		INITIALIZE_SIGNAL(account_friend_unfriended);

		// ####### Chat #######

		INITIALIZE_SIGNAL(chat_message_received);
		INITIALIZE_SIGNAL(chat_channel_info_response);

		// ####### Players #######

		INITIALIZE_SIGNAL(player_joined_fragment);
		INITIALIZE_SIGNAL(player_left_fragment);

		// ####### Party #######

		INITIALIZE_SIGNAL(party_create_response);
		INITIALIZE_SIGNAL(party_invited);
		INITIALIZE_SIGNAL(party_invitation_response);
		INITIALIZE_SIGNAL(party_kicked);
		INITIALIZE_SIGNAL(party_player_joined);
		INITIALIZE_SIGNAL(party_player_left);

		// ####### Entity #######

		INITIALIZE_SIGNAL(entity_info_request_response);

		// ####### Volume (is entity) #######

		INITIALIZE_SIGNAL(volume_block_place_response);
		INITIALIZE_SIGNAL(volume_block_fill_response);
		INITIALIZE_SIGNAL(new_volume_block_place_response);
		INITIALIZE_SIGNAL(volume_block_interact_response);

		// ####### Galaxy Region #######

		INITIALIZE_SIGNAL(galaxy_region_info_request_response);
		INITIALIZE_SIGNAL(galaxy_region_entered);
		INITIALIZE_SIGNAL(galaxy_region_exited);

		// ####### Galaxy Object (is volume) #######

		INITIALIZE_SIGNAL(galaxy_object_info_request_response);

		// ####### Currency #######

		INITIALIZE_SIGNAL(currency_withdraw_response);
		INITIALIZE_SIGNAL(currency_deposit_response);
		INITIALIZE_SIGNAL(currency_convert_response);
		INITIALIZE_SIGNAL(currency_pay_response);
		INITIALIZE_SIGNAL(currency_buy_good_response);

		// ####### Internet #######

		INITIALIZE_SIGNAL(internet_start_response);
		INITIALIZE_SIGNAL(internet_stop_response);
		INITIALIZE_SIGNAL(internet_url_request_response);

		// ####### Faction Roles #######

		INITIALIZE_SIGNAL(faction_role_add_response);
		INITIALIZE_SIGNAL(faction_role_remove_response);
		INITIALIZE_SIGNAL(faction_role_modify_response);
		INITIALIZE_SIGNAL(faction_permission_add_response);
		INITIALIZE_SIGNAL(faction_permission_remove_response);
		INITIALIZE_SIGNAL(faction_set_entity_role_response);

		// ####### Faction (is entity) #######

		INITIALIZE_SIGNAL(faction_join_response);
		INITIALIZE_SIGNAL(faction_leave_response);
		INITIALIZE_SIGNAL(faction_entity_invite_response);
		INITIALIZE_SIGNAL(faction_invite_received);
		INITIALIZE_SIGNAL(faction_entity_kick_response);
		INITIALIZE_SIGNAL(faction_child_add_response);
		INITIALIZE_SIGNAL(faction_child_remove_response);
		INITIALIZE_SIGNAL(faction_child_invite_response);
		INITIALIZE_SIGNAL(faction_kick_response);

		// ####### Player Faction (is faction) #######

		INITIALIZE_SIGNAL(faction_request_join_response);

		// ####### Level #######

		INITIALIZE_SIGNAL(levelup_available);
		INITIALIZE_SIGNAL(complete_levelup_response);

		// ####### Looking at #######

		INITIALIZE_SIGNAL(hovered_over_entity);
		INITIALIZE_SIGNAL(hovered_over_volume);
		INITIALIZE_SIGNAL(hovered_over_block);

		// ####### Inventory #######

		INITIALIZE_SIGNAL(inventory_trashed_item_response);
		INITIALIZE_SIGNAL(inventory_moved_item_response);
		INITIALIZE_SIGNAL(inventory_transfer_item_response);
		INITIALIZE_SIGNAL(inventory_interact_item_response);
		INITIALIZE_SIGNAL(inventory_force_closed);

		// ####### Interact #######

		INITIALIZE_SIGNAL(entity_store_response);
		INITIALIZE_SIGNAL(block_hold_response);
		INITIALIZE_SIGNAL(entity_drop_response);

		INITIALIZE_SIGNAL(equip_from_world_response);
		INITIALIZE_SIGNAL(equip_from_inventory_response);
		INITIALIZE_SIGNAL(drop_equip_to_world_response);
		INITIALIZE_SIGNAL(unequip_to_inventory_response);
		INITIALIZE_SIGNAL(set_left_hand_equip_response);
		INITIALIZE_SIGNAL(set_right_hand_equip_response);
		INITIALIZE_SIGNAL(use_equip_response);
		INITIALIZE_SIGNAL(toggle_equip_response);

		INITIALIZE_SIGNAL(ride_entity_response);
		INITIALIZE_SIGNAL(change_ride_attachment_response);
		INITIALIZE_SIGNAL(exit_ride_response);
		INITIALIZE_SIGNAL(ride_force_exited);

		INITIALIZE_SIGNAL(entity_interact_response);

		// ####### Vehicle Control #######

		INITIALIZE_SIGNAL(vehicle_accelerate_response);
		INITIALIZE_SIGNAL(vehicle_deccelerate_response);
		INITIALIZE_SIGNAL(vehicle_control_activate_response);
		INITIALIZE_SIGNAL(vehicle_control_toggle_response);
		INITIALIZE_SIGNAL(vehicle_set_setting_response);

		// ####### Abilities #######

		INITIALIZE_SIGNAL(activate_ability_response);
		INITIALIZE_SIGNAL(toggle_ability_response);
		INITIALIZE_SIGNAL(player_set_setting_response);

		// ####### Magic #######

		INITIALIZE_SIGNAL(use_spell_response_response);

		// ####### Testing #######

		INITIALIZE_SIGNAL(test_signal);
	}
}