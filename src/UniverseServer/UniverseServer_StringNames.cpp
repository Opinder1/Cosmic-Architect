#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

#define INITIALIZE_STRINGNAME(name) name = godot::StringName{ #name }

namespace voxel_game
{
	UniverseServer::SignalStrings::SignalStrings()
	{
		// ####### Universe #######

		INITIALIZE_STRINGNAME(connected_to_galaxy_list);
		INITIALIZE_STRINGNAME(disconnected_from_galaxy_list);
		INITIALIZE_STRINGNAME(galaxy_list_query_response);
		INITIALIZE_STRINGNAME(galaxy_ping_response);
		INITIALIZE_STRINGNAME(connected_to_galaxy);
		INITIALIZE_STRINGNAME(disconnected_from_galaxy);

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