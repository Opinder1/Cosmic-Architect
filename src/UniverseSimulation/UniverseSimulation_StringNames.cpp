#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

#define INITIALIZE_STRINGNAME(name) name = godot::StringName{ #name }

namespace voxel_game
{
	UniverseSimulation::CommandStrings::CommandStrings()
	{
		INITIALIZE_STRINGNAME(initialize);
		INITIALIZE_STRINGNAME(debug_command);

		// ####### Universe #######

		INITIALIZE_STRINGNAME(get_universe_info);
		INITIALIZE_STRINGNAME(connect_to_galaxy_list);
		INITIALIZE_STRINGNAME(disconnect_from_galaxy_list);
		INITIALIZE_STRINGNAME(query_galaxy_list);
		INITIALIZE_STRINGNAME(ping_remote_galaxy);

		// ####### Fragments (admin only) #######

		INITIALIZE_STRINGNAME(get_fragment_info);
		INITIALIZE_STRINGNAME(get_current_fragment);
		INITIALIZE_STRINGNAME(enter_fragment);

		// ####### Account #######

		INITIALIZE_STRINGNAME(get_account_info);
		INITIALIZE_STRINGNAME(create_account);
		INITIALIZE_STRINGNAME(account_login);
		INITIALIZE_STRINGNAME(saved_session_login);
		INITIALIZE_STRINGNAME(clear_saved_session);
		INITIALIZE_STRINGNAME(delete_account);
		INITIALIZE_STRINGNAME(logout_account);

		// ####### Friends #######

		INITIALIZE_STRINGNAME(get_friends);
		INITIALIZE_STRINGNAME(invite_friend);
		INITIALIZE_STRINGNAME(accept_friend_invite);
		INITIALIZE_STRINGNAME(remove_friend);

		// ####### Chat #######

		INITIALIZE_STRINGNAME(get_channel_info);
		INITIALIZE_STRINGNAME(send_message_to_channel);
		INITIALIZE_STRINGNAME(send_message_to_player);
		INITIALIZE_STRINGNAME(get_chat_channel_history);
		INITIALIZE_STRINGNAME(get_private_chat_history);

		// ####### Players #######

		INITIALIZE_STRINGNAME(get_player_info);

		// ####### Party #######

		INITIALIZE_STRINGNAME(get_party_info);
		INITIALIZE_STRINGNAME(create_party);
		INITIALIZE_STRINGNAME(invite_to_party);
		INITIALIZE_STRINGNAME(accept_invite);
		INITIALIZE_STRINGNAME(kick_from_party);
		INITIALIZE_STRINGNAME(leave_party);
		INITIALIZE_STRINGNAME(get_players_in_party);
		INITIALIZE_STRINGNAME(get_party_chat_channel);

		// ####### Entity #######

		INITIALIZE_STRINGNAME(get_entity_info);
		INITIALIZE_STRINGNAME(request_entity_info);

		// ####### Volume (is entity) #######

		INITIALIZE_STRINGNAME(get_volume_info);
		INITIALIZE_STRINGNAME(get_block_info);
		INITIALIZE_STRINGNAME(place_block);
		INITIALIZE_STRINGNAME(fill_blocks);
		INITIALIZE_STRINGNAME(place_block_in_new_volume);
		INITIALIZE_STRINGNAME(interact_block);
		INITIALIZE_STRINGNAME(get_entity_position_in_volume);
		INITIALIZE_STRINGNAME(fragment_position_to_volume_position);
		INITIALIZE_STRINGNAME(volume_position_to_fragment_position);

		// ####### Galaxy Region #######

		INITIALIZE_STRINGNAME(get_galaxy_info);
		INITIALIZE_STRINGNAME(get_galaxy_region_info);
		INITIALIZE_STRINGNAME(request_galaxy_region_info);
		INITIALIZE_STRINGNAME(get_current_galaxy_regions);

		// ####### Galaxy Object (is volume) #######

		INITIALIZE_STRINGNAME(get_galaxy_object_info);
		INITIALIZE_STRINGNAME(request_galaxy_object_info);

		// ####### Currency #######

		INITIALIZE_STRINGNAME(get_currency_info);
		INITIALIZE_STRINGNAME(get_bank_info);
		INITIALIZE_STRINGNAME(get_bank_interface_info);
		INITIALIZE_STRINGNAME(get_good_info);
		INITIALIZE_STRINGNAME(get_universal_currency);
		INITIALIZE_STRINGNAME(get_bank_of_interface);
		INITIALIZE_STRINGNAME(get_owned_currencies);
		INITIALIZE_STRINGNAME(get_balance);
		INITIALIZE_STRINGNAME(withdraw);
		INITIALIZE_STRINGNAME(deposit);
		INITIALIZE_STRINGNAME(convert);
		INITIALIZE_STRINGNAME(pay_entity);
		INITIALIZE_STRINGNAME(buy_good_with_currency);

		// ####### Internet #######

		INITIALIZE_STRINGNAME(get_internet_info);
		INITIALIZE_STRINGNAME(get_website_info);
		INITIALIZE_STRINGNAME(get_website_page_info);
		INITIALIZE_STRINGNAME(get_internet_websites);
		INITIALIZE_STRINGNAME(get_website_pages);
		INITIALIZE_STRINGNAME(start_internet);
		INITIALIZE_STRINGNAME(stop_internet);
		INITIALIZE_STRINGNAME(get_current_internet);
		INITIALIZE_STRINGNAME(get_current_internet_site);
		INITIALIZE_STRINGNAME(get_current_internet_page);
		INITIALIZE_STRINGNAME(request_internet_url);

		// ####### Faction Roles #######

		INITIALIZE_STRINGNAME(get_role_info);
		INITIALIZE_STRINGNAME(get_permission_info);
		INITIALIZE_STRINGNAME(get_entity_role);
		INITIALIZE_STRINGNAME(add_faction_role);
		INITIALIZE_STRINGNAME(remove_faction_role);
		INITIALIZE_STRINGNAME(modify_faction_role);
		INITIALIZE_STRINGNAME(add_permission_to_role);
		INITIALIZE_STRINGNAME(remove_permission_from_role);
		INITIALIZE_STRINGNAME(set_entity_role);
		INITIALIZE_STRINGNAME(entity_has_permission);

		// ####### Faction (is entity) #######

		INITIALIZE_STRINGNAME(get_faction_info);
		INITIALIZE_STRINGNAME(get_joined_factions);
		INITIALIZE_STRINGNAME(join_faction);
		INITIALIZE_STRINGNAME(leave_faction);
		INITIALIZE_STRINGNAME(invite_entity_to_faction);
		INITIALIZE_STRINGNAME(kick_entity_from_faction);
		INITIALIZE_STRINGNAME(add_child_faction);
		INITIALIZE_STRINGNAME(remove_child_faction);
		INITIALIZE_STRINGNAME(invite_child_faction);
		INITIALIZE_STRINGNAME(kick_child_faction);

		// ####### Player Faction (is faction) #######

		INITIALIZE_STRINGNAME(get_global_player_faction);
		INITIALIZE_STRINGNAME(get_player_faction);
		INITIALIZE_STRINGNAME(request_join_player_faction);

		// ####### Language #######

		INITIALIZE_STRINGNAME(get_language_info);
		INITIALIZE_STRINGNAME(get_language_translation);
		INITIALIZE_STRINGNAME(get_language_string);

		// ####### Culture #######

		INITIALIZE_STRINGNAME(get_culture_info);

		// ####### Level #######

		INITIALIZE_STRINGNAME(get_level);
		INITIALIZE_STRINGNAME(get_experience);
		INITIALIZE_STRINGNAME(complete_level_up);

		// ####### Player Control #######

		INITIALIZE_STRINGNAME(set_sprint);
		INITIALIZE_STRINGNAME(set_crouching);
		INITIALIZE_STRINGNAME(set_prone);
		INITIALIZE_STRINGNAME(do_walk);
		INITIALIZE_STRINGNAME(do_look);
		INITIALIZE_STRINGNAME(do_jump);

		// ####### Looking at #######

		INITIALIZE_STRINGNAME(get_looking_at_entity);
		INITIALIZE_STRINGNAME(get_looking_at_volume);
		INITIALIZE_STRINGNAME(get_looking_at_block);

		// ####### Inventory #######

		INITIALIZE_STRINGNAME(get_inventory_info);
		INITIALIZE_STRINGNAME(get_inventory);
		INITIALIZE_STRINGNAME(get_inventory_item_entity);
		INITIALIZE_STRINGNAME(trash_inventory_item);
		INITIALIZE_STRINGNAME(move_inventory_item);
		INITIALIZE_STRINGNAME(transfer_inventory_item);
		INITIALIZE_STRINGNAME(interact_with_inventory_item);

		// ####### Interact #######
		INITIALIZE_STRINGNAME(store_entity);
		INITIALIZE_STRINGNAME(hold_block);
		INITIALIZE_STRINGNAME(hold_entity);
		INITIALIZE_STRINGNAME(drop_held_entity);

		INITIALIZE_STRINGNAME(equip_item_from_world);
		INITIALIZE_STRINGNAME(equip_item_from_inventory);
		INITIALIZE_STRINGNAME(drop_equip_to_world);
		INITIALIZE_STRINGNAME(unequip_item_to_inventory);
		INITIALIZE_STRINGNAME(set_left_hand_equip);
		INITIALIZE_STRINGNAME(set_right_hand_equip);
		INITIALIZE_STRINGNAME(use_equip);
		INITIALIZE_STRINGNAME(toggle_equip);

		INITIALIZE_STRINGNAME(ride_entity);
		INITIALIZE_STRINGNAME(change_attachment_point);
		INITIALIZE_STRINGNAME(exit_entity);

		INITIALIZE_STRINGNAME(interact_with_entity);

		// ####### Vehicle Control #######

		INITIALIZE_STRINGNAME(trigger_vehicle_control);
		INITIALIZE_STRINGNAME(toggle_vehicle_control);
		INITIALIZE_STRINGNAME(set_vehicle_control);

		// ####### Abilities #######

		INITIALIZE_STRINGNAME(get_ability_info);
		INITIALIZE_STRINGNAME(activate_ability);
		INITIALIZE_STRINGNAME(toggle_ability);
		INITIALIZE_STRINGNAME(set_player_setting);

		// ####### Magic #######

		INITIALIZE_STRINGNAME(get_spell_info);
		INITIALIZE_STRINGNAME(use_spell);
	}

	UniverseSimulation::SignalStrings::SignalStrings()
	{
		// ####### Universe #######

		INITIALIZE_STRINGNAME(connected_to_galaxy_list);
		INITIALIZE_STRINGNAME(disconnected_from_galaxy_list);
		INITIALIZE_STRINGNAME(galaxy_list_query_response);
		INITIALIZE_STRINGNAME(galaxy_ping_response);

		// ####### Galaxy #######

		INITIALIZE_STRINGNAME(connected_to_remote);
		INITIALIZE_STRINGNAME(disonnected_from_remote);

		// ####### Fragments (admin only) #######

		INITIALIZE_STRINGNAME(fragment_added);
		INITIALIZE_STRINGNAME(fragment_removed);
		INITIALIZE_STRINGNAME(fragment_loaded);
		INITIALIZE_STRINGNAME(fragment_unloaded);

		// ####### Account #######

		INITIALIZE_STRINGNAME(account_create_request_response);
		INITIALIZE_STRINGNAME(account_login_request_response);
		INITIALIZE_STRINGNAME(account_delete_request_response);
		INITIALIZE_STRINGNAME(account_logout_response);

		// ####### Friends #######

		INITIALIZE_STRINGNAME(account_invite_friend_response);
		INITIALIZE_STRINGNAME(account_friend_request_received);
		INITIALIZE_STRINGNAME(account_remove_friend_response);
		INITIALIZE_STRINGNAME(account_friend_unfriended);

		// ####### Chat #######

		INITIALIZE_STRINGNAME(chat_message_received);
		INITIALIZE_STRINGNAME(chat_channel_info_response);

		// ####### Players #######

		INITIALIZE_STRINGNAME(player_joined_fragment);
		INITIALIZE_STRINGNAME(player_left_fragment);

		// ####### Party #######

		INITIALIZE_STRINGNAME(party_create_response);
		INITIALIZE_STRINGNAME(party_invited);
		INITIALIZE_STRINGNAME(party_invitation_response);
		INITIALIZE_STRINGNAME(party_kicked);
		INITIALIZE_STRINGNAME(party_player_joined);
		INITIALIZE_STRINGNAME(party_player_left);

		// ####### Entity #######

		INITIALIZE_STRINGNAME(entity_info_request_response);

		// ####### Volume (is entity) #######

		INITIALIZE_STRINGNAME(volume_block_place_response);
		INITIALIZE_STRINGNAME(volume_block_fill_response);
		INITIALIZE_STRINGNAME(new_volume_block_place_response);
		INITIALIZE_STRINGNAME(volume_block_interact_response);

		// ####### Galaxy Region #######

		INITIALIZE_STRINGNAME(galaxy_region_info_request_response);
		INITIALIZE_STRINGNAME(galaxy_region_entered);
		INITIALIZE_STRINGNAME(galaxy_region_exited);

		// ####### Galaxy Object (is volume) #######

		INITIALIZE_STRINGNAME(galaxy_object_info_request_response);

		// ####### Currency #######

		INITIALIZE_STRINGNAME(currency_withdraw_response);
		INITIALIZE_STRINGNAME(currency_deposit_response);
		INITIALIZE_STRINGNAME(currency_convert_response);
		INITIALIZE_STRINGNAME(currency_pay_response);
		INITIALIZE_STRINGNAME(currency_buy_good_response);

		// ####### Internet #######

		INITIALIZE_STRINGNAME(internet_start_response);
		INITIALIZE_STRINGNAME(internet_stop_response);
		INITIALIZE_STRINGNAME(internet_url_request_response);

		// ####### Faction Roles #######

		INITIALIZE_STRINGNAME(faction_role_add_response);
		INITIALIZE_STRINGNAME(faction_role_remove_response);
		INITIALIZE_STRINGNAME(faction_role_modify_response);
		INITIALIZE_STRINGNAME(faction_permission_add_response);
		INITIALIZE_STRINGNAME(faction_permission_remove_response);
		INITIALIZE_STRINGNAME(faction_set_entity_role_response);

		// ####### Faction (is entity) #######

		INITIALIZE_STRINGNAME(faction_join_response);
		INITIALIZE_STRINGNAME(faction_leave_response);
		INITIALIZE_STRINGNAME(faction_entity_invite_response);
		INITIALIZE_STRINGNAME(faction_invite_received);
		INITIALIZE_STRINGNAME(faction_entity_kick_response);
		INITIALIZE_STRINGNAME(faction_child_add_response);
		INITIALIZE_STRINGNAME(faction_child_remove_response);
		INITIALIZE_STRINGNAME(faction_child_invite_response);
		INITIALIZE_STRINGNAME(faction_kick_response);

		// ####### Player Faction (is faction) #######

		INITIALIZE_STRINGNAME(faction_request_join_response);

		// ####### Level #######

		INITIALIZE_STRINGNAME(levelup_available);
		INITIALIZE_STRINGNAME(complete_levelup_response);

		// ####### Looking at #######

		INITIALIZE_STRINGNAME(hovered_over_entity);
		INITIALIZE_STRINGNAME(hovered_over_volume);
		INITIALIZE_STRINGNAME(hovered_over_block);

		// ####### Inventory #######

		INITIALIZE_STRINGNAME(inventory_trashed_item_response);
		INITIALIZE_STRINGNAME(inventory_moved_item_response);
		INITIALIZE_STRINGNAME(inventory_transfer_item_response);
		INITIALIZE_STRINGNAME(inventory_interact_item_response);
		INITIALIZE_STRINGNAME(inventory_force_closed);

		// ####### Interact #######

		INITIALIZE_STRINGNAME(entity_store_response);
		INITIALIZE_STRINGNAME(block_hold_response);
		INITIALIZE_STRINGNAME(entity_drop_response);

		INITIALIZE_STRINGNAME(equip_from_world_response);
		INITIALIZE_STRINGNAME(equip_from_inventory_response);
		INITIALIZE_STRINGNAME(drop_equip_to_world_response);
		INITIALIZE_STRINGNAME(unequip_to_inventory_response);
		INITIALIZE_STRINGNAME(set_left_hand_equip_response);
		INITIALIZE_STRINGNAME(set_right_hand_equip_response);
		INITIALIZE_STRINGNAME(use_equip_response);
		INITIALIZE_STRINGNAME(toggle_equip_response);

		INITIALIZE_STRINGNAME(ride_entity_response);
		INITIALIZE_STRINGNAME(change_ride_attachment_response);
		INITIALIZE_STRINGNAME(exit_ride_response);
		INITIALIZE_STRINGNAME(ride_force_exited);

		INITIALIZE_STRINGNAME(entity_interact_response);

		// ####### Vehicle Control #######

		INITIALIZE_STRINGNAME(vehicle_accelerate_response);
		INITIALIZE_STRINGNAME(vehicle_deccelerate_response);
		INITIALIZE_STRINGNAME(vehicle_control_activate_response);
		INITIALIZE_STRINGNAME(vehicle_control_toggle_response);
		INITIALIZE_STRINGNAME(vehicle_set_setting_response);

		// ####### Abilities #######

		INITIALIZE_STRINGNAME(activate_ability_response);
		INITIALIZE_STRINGNAME(toggle_ability_response);
		INITIALIZE_STRINGNAME(player_set_setting_response);

		// ####### Magic #######

		INITIALIZE_STRINGNAME(use_spell_response_response);
	}
}