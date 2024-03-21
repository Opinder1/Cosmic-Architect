#include "UniverseSimulation.h"

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
#define INITIALIZE_SIGNAL(name) name = godot::StringName{ #name }

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

		ADD_SIGNAL(godot::MethodInfo(k_signals->fragment_added));
		ADD_SIGNAL(godot::MethodInfo(k_signals->fragment_removed));
		ADD_SIGNAL(godot::MethodInfo(k_signals->fragment_loaded));
		ADD_SIGNAL(godot::MethodInfo(k_signals->fragment_unloaded));

		// ####### Account #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->account_create_request_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->account_login_request_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->account_delete_request_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->account_logout_response));

		// ####### Friends #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->account_invite_friend_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->account_friend_request_received));
		ADD_SIGNAL(godot::MethodInfo(k_signals->account_remove_friend_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->account_friend_unfriended));

		// ####### Chat #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->chat_message_received));
		ADD_SIGNAL(godot::MethodInfo(k_signals->chat_channel_info_response));

		// ####### Players #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->player_joined_fragment));
		ADD_SIGNAL(godot::MethodInfo(k_signals->player_left_fragment));

		// ####### Party #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->party_create_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->party_invited));
		ADD_SIGNAL(godot::MethodInfo(k_signals->party_invitation_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->party_kicked));
		ADD_SIGNAL(godot::MethodInfo(k_signals->party_player_joined));
		ADD_SIGNAL(godot::MethodInfo(k_signals->party_player_left));

		// ####### Entity #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->entity_info_request_response));

		// ####### Volume (is entity) #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->volume_block_place_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->volume_block_fill_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->new_volume_block_place_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->volume_block_interact_response));

		// ####### Galaxy Region #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_region_info_request_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_region_entered));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_region_exited));

		// ####### Galaxy Object (is volume) #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_object_info_request_response));

		// ####### Currency #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->currency_withdraw_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->currency_deposit_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->currency_convert_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->currency_pay_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->currency_buy_good_response));

		// ####### Internet #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->internet_start_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->internet_stop_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->internet_url_request_response));

		// ####### Faction Roles #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_role_add_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_role_remove_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_role_modify_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_permission_add_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_permission_remove_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_set_entity_role_response));

		// ####### Faction (is entity) #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_join_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_leave_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_entity_invite_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_invite_received));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_entity_kick_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_child_add_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_child_remove_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_child_invite_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_kick_response));

		// ####### Player Faction (is faction) #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_request_join_response));

		// ####### Level #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->levelup_available));
		ADD_SIGNAL(godot::MethodInfo(k_signals->complete_levelup_response));

		// ####### Looking at #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->hovered_over_entity));
		ADD_SIGNAL(godot::MethodInfo(k_signals->hovered_over_volume));
		ADD_SIGNAL(godot::MethodInfo(k_signals->hovered_over_block));

		// ####### Inventory #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->inventory_trashed_item_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->inventory_moved_item_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->inventory_transfer_item_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->inventory_interact_item_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->inventory_force_closed));

		// ####### Interact #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->entity_store_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->block_hold_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->entity_drop_response));

		ADD_SIGNAL(godot::MethodInfo(k_signals->equip_from_world_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->equip_from_inventory_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->drop_equip_to_world_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->unequip_to_inventory_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->set_left_hand_equip_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->set_right_hand_equip_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->use_equip_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->toggle_equip_response));

		ADD_SIGNAL(godot::MethodInfo(k_signals->ride_entity_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->change_ride_attachment_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->exit_ride_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->ride_force_exited));

		ADD_SIGNAL(godot::MethodInfo(k_signals->entity_interact_response));

		// ####### Vehicle Control #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->vehicle_accelerate_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->vehicle_deccelerate_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->vehicle_control_activate_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->vehicle_control_toggle_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->vehicle_set_setting_response));

		// ####### Abilities #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->activate_ability_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->toggle_ability_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->player_set_setting_response));

		// ####### Magic #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->use_spell_response_response));

		// ####### Testing #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->test_signal));

#undef ENUM_PROPERTY
	}

	void UniverseSimulation::CleanupSignals()
	{
		k_signals.reset();
	}
}