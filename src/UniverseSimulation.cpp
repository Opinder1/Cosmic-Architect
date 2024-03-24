#include "UniverseSimulation.h"
#include "Universe.h"

#include "Simulation/UniverseModule.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/os.hpp>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/type_info.hpp>

namespace voxel_game
{
	void StartRestServer(flecs::world& world, uint16_t port, bool monitor)
	{
		world.set<flecs::Rest>({ port, nullptr, nullptr });

		if (monitor)
		{
			world.import<flecs::monitor>();
		}

		DEBUG_PRINT_INFO("Started rest server on port " + godot::UtilityFunctions::str(port) + " with monitoring " + (monitor ? "enabled" : "disabled"));
	}

	size_t UniverseSimulation::UUIDHash::operator()(const UUID& uuid) const
	{
		static_assert(sizeof(size_t[2]) == sizeof(UUID));

		size_t* arr = (size_t*)&uuid;

		return arr[0] ^ arr[1];
	}

	UniverseSimulation::UniverseSimulation()
	{}

	UniverseSimulation::~UniverseSimulation()
	{}

	void UniverseSimulation::Initialize(const godot::Ref<Universe>& universe, const godot::String& path, const godot::String& fragment_type, bool remote)
	{
		DEBUG_ASSERT(!m_universe.is_valid(), "We can't initialize a simulation twice");

		m_universe = universe;
		m_path = path;
		m_fragment_type = fragment_type;
		m_remote = remote;
		m_directory = godot::DirAccess::open(m_path);
	}

	godot::Dictionary UniverseSimulation::GetUniverseInfo()
	{
		DEBUG_ASSERT(m_universe.is_valid(), "This universe simulations should have been instantiated by a universe");
		return m_universe->GetUniverseInfo();
	}

	godot::Dictionary UniverseSimulation::GetGalaxyInfo()
	{
		return m_galaxy_info_cache;
	}

	void UniverseSimulation::StartSimulation()
	{
		DEBUG_ASSERT(m_galaxy_load_state == LoadState::LOAD_STATE_LOADED, "This galaxy should not be loaded when we start");
		DEBUG_ASSERT(m_universe.is_valid(), "This universe simulation should have been instantiated by a universe");

		m_world.reset();

#if DEBUG
		StartRestServer(m_world, 27750, true);
#endif

		m_world.set_threads(godot::OS::get_singleton()->get_processor_count());

		m_world.import<UniverseModule>();

		m_galaxy_load_state = LOAD_STATE_LOADING;
		emit_signal(k_signals->load_state_changed, m_galaxy_load_state);
	}

	void UniverseSimulation::StopSimulation()
	{
		DEBUG_ASSERT(m_galaxy_load_state == LoadState::LOAD_STATE_LOADED, "This galaxy should have loaded if we want to start unloading");

		m_galaxy_load_state = LOAD_STATE_UNLOADING;
		emit_signal(k_signals->load_state_changed, m_galaxy_load_state);
	}

	bool UniverseSimulation::Progress(double delta)
	{
		return m_world.progress(delta);
	}

	UniverseSimulation::LoadState UniverseSimulation::GetGalaxyLoadState()
	{
		return m_galaxy_load_state;
	}

	void UniverseSimulation::BindEnums()
	{
		BIND_ENUM_CONSTANT(LOAD_STATE_LOADING);
		BIND_ENUM_CONSTANT(LOAD_STATE_LOADED);
		BIND_ENUM_CONSTANT(LOAD_STATE_UNLOADING);
		BIND_ENUM_CONSTANT(LOAD_STATE_UNLOADED);
	}

	void UniverseSimulation::BindMethods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("get_universe_info"), &UniverseSimulation::GetUniverseInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_galaxy_info"), &UniverseSimulation::GetGalaxyInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("start_simulation"), &UniverseSimulation::StartSimulation);
		godot::ClassDB::bind_method(godot::D_METHOD("stop_simulation"), &UniverseSimulation::StopSimulation);
		godot::ClassDB::bind_method(godot::D_METHOD("progress", "delta"), &UniverseSimulation::Progress);
		godot::ClassDB::bind_method(godot::D_METHOD("get_galaxy_load_state"), &UniverseSimulation::GetGalaxyLoadState);

		// ####### Fragments (admin only) #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_fragment_info", "fragment_id"), &UniverseSimulation::GetFragmentInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_current_fragment"), &UniverseSimulation::GetCurrentFragment);
		godot::ClassDB::bind_method(godot::D_METHOD("enter_fragment", "fragment_id", "method"), &UniverseSimulation::EnterFragment);

		// ####### Account #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_account_info"), &UniverseSimulation::GetAccountInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("create_account", "username", "password_hash"), &UniverseSimulation::CreateAccount);
		godot::ClassDB::bind_method(godot::D_METHOD("account_login", "username", "password_hash"), &UniverseSimulation::AccountLogin);
		godot::ClassDB::bind_method(godot::D_METHOD("saved_session_login"), &UniverseSimulation::SavedSessionLogin);
		godot::ClassDB::bind_method(godot::D_METHOD("clear_saved_session"), &UniverseSimulation::ClearSavedSession);
		godot::ClassDB::bind_method(godot::D_METHOD("delete_account"), &UniverseSimulation::DeleteAccount);
		godot::ClassDB::bind_method(godot::D_METHOD("logout_account"), &UniverseSimulation::LogoutAccount);

		// ####### Friends #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_friends"), &UniverseSimulation::GetFriends);
		godot::ClassDB::bind_method(godot::D_METHOD("invite_friend", "account_id"), &UniverseSimulation::InviteFriend);
		godot::ClassDB::bind_method(godot::D_METHOD("accept_friend_invite", "account_id"), &UniverseSimulation::AcceptFreindInvite);
		godot::ClassDB::bind_method(godot::D_METHOD("remove_friend", "account_id"), &UniverseSimulation::RemoveFriend);

		// ####### Chat #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_channel_info", "channel_id"), &UniverseSimulation::GetChannelInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("send_message_to_channel", "message", "channel_id"), &UniverseSimulation::SendMessageToChannel);
		godot::ClassDB::bind_method(godot::D_METHOD("send_message_to_player", "message", "channel_id"), &UniverseSimulation::SendMessageToPlayer);
		godot::ClassDB::bind_method(godot::D_METHOD("get_chat_channel_history", "channel_id"), &UniverseSimulation::GetChatChannelHistory);
		godot::ClassDB::bind_method(godot::D_METHOD("get_private_chat_history", "account_id"), &UniverseSimulation::GetPrivateChatHistory);

		// ####### Players #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_player_info", "player_id"), &UniverseSimulation::GetPlayerInfo);

		// ####### Party #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_party_info", "party_host_id"), &UniverseSimulation::GetPartyInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("create_party"), &UniverseSimulation::CreateParty);
		godot::ClassDB::bind_method(godot::D_METHOD("invite_to_party", "player_id"), &UniverseSimulation::InviteToParty);
		godot::ClassDB::bind_method(godot::D_METHOD("accept_invite", "player_id"), &UniverseSimulation::AcceptInvite);
		godot::ClassDB::bind_method(godot::D_METHOD("kick_from_party"), &UniverseSimulation::KickFromParty);
		godot::ClassDB::bind_method(godot::D_METHOD("leave_party"), &UniverseSimulation::LeaveParty);
		godot::ClassDB::bind_method(godot::D_METHOD("get_players_in_party"), &UniverseSimulation::GetPlayersInParty);
		godot::ClassDB::bind_method(godot::D_METHOD("get_party_chat_channel"), &UniverseSimulation::GetPartyChatChannel);

		// ####### Entity #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_entity_info", "entity_id"), &UniverseSimulation::GetEntityInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("request_entity_info", "entity_id"), &UniverseSimulation::RequestEntityInfo);

		// ####### Volume (is entity) #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_volume_info", "volume_id"), &UniverseSimulation::GetVolumeInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_block_info", "volume_id", "position"), &UniverseSimulation::GetBlockInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("place_block", "volume_id", "position", "block_id", "block_data"), &UniverseSimulation::PlaceBlock);
		godot::ClassDB::bind_method(godot::D_METHOD("fill_blocks", "volume_id", "position_first", "position_second", "block_id", "block_data"), &UniverseSimulation::FillBlocks);
		godot::ClassDB::bind_method(godot::D_METHOD("place_block_in_new_volume", "fragment_position", "block_id", "block_data"), &UniverseSimulation::PlaceBlockInNewVolume);
		godot::ClassDB::bind_method(godot::D_METHOD("interact_block", "volume_id", "position", "interaction"), &UniverseSimulation::InteractBlock);
		godot::ClassDB::bind_method(godot::D_METHOD("get_entity_position_in_volume", "volume_id", "entity_id"), &UniverseSimulation::GetEntityPositionInVolume);
		godot::ClassDB::bind_method(godot::D_METHOD("fragment_position_to_volume_position", "volume_id", "fragment_position"), &UniverseSimulation::FragmentPositionToVolumePosition);
		godot::ClassDB::bind_method(godot::D_METHOD("volume_position_to_fragment_position", "volume_id", "volume_position"), &UniverseSimulation::VolumePositionToFragmentPosition);

		// ####### Galaxy Region #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_galaxy_region_info", "galaxy_region_id"), &UniverseSimulation::GetGalaxyRegionInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("request_galaxy_region_info", "galaxy_region_id"), &UniverseSimulation::RequestGalaxyRegionInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_current_galaxy_regions"), &UniverseSimulation::GetCurrentGalaxyRegions);

		// ####### Galaxy Object (is volume) #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_galaxy_object_info", "galaxy_object_id"), &UniverseSimulation::GetGalaxyObjectInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("request_galaxy_object_info", "entity_id"), &UniverseSimulation::RequestGalaxyObjectInfo);

		// ####### Currency #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_currency_info", "currency_id"), &UniverseSimulation::GetCurrencyInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_bank_info", "bank_id"), &UniverseSimulation::GetBankInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_bank_interface_info", "bank_interface_id"), &UniverseSimulation::GetBankInterfaceInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_good_info", "good_id"), &UniverseSimulation::GetGoodInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_universal_currency"), &UniverseSimulation::GetUniversalCurrency);
		godot::ClassDB::bind_method(godot::D_METHOD("get_bank_of_interface", "bank_interface_id"), &UniverseSimulation::GetBankOfInterface);
		godot::ClassDB::bind_method(godot::D_METHOD("get_owned_currencies"), &UniverseSimulation::GetOwnedCurrencies);
		godot::ClassDB::bind_method(godot::D_METHOD("get_balance", "currency_id"), &UniverseSimulation::GetBalance);
		godot::ClassDB::bind_method(godot::D_METHOD("withdraw", "currency_id", "amount", "bank_interface_id"), &UniverseSimulation::Withdraw);
		godot::ClassDB::bind_method(godot::D_METHOD("deposit", "currency_id", "amount", "bank_interface_id"), &UniverseSimulation::Deposit);
		godot::ClassDB::bind_method(godot::D_METHOD("convert", "from_currency_id", "from_currency_id", "amount", "bank_interface_id"), &UniverseSimulation::Convert);
		godot::ClassDB::bind_method(godot::D_METHOD("pay_entity", "currency_id", "entity_id", "amount", "bank_interface_id"), &UniverseSimulation::PayEntity);
		godot::ClassDB::bind_method(godot::D_METHOD("buy_good_with_currency", "good_id", "currency_id"), &UniverseSimulation::BuyGoodWithCurrency);

		// ####### Internet #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_internet_info", "internet_id"), &UniverseSimulation::GetInternetInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_website_info", "website_id"), &UniverseSimulation::GetWebsiteInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_website_page_info", "website_page_id"), &UniverseSimulation::GetWebsitePageInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_internet_websites", "internet_id"), &UniverseSimulation::GetInternetWebsites);
		godot::ClassDB::bind_method(godot::D_METHOD("get_website_pages", "website_id"), &UniverseSimulation::GetWebsitePages);
		godot::ClassDB::bind_method(godot::D_METHOD("start_internet", "internet_id", "device_id"), &UniverseSimulation::StartInternet);
		godot::ClassDB::bind_method(godot::D_METHOD("stop_internet"), &UniverseSimulation::StopInternet);
		godot::ClassDB::bind_method(godot::D_METHOD("get_current_internet"), &UniverseSimulation::GetCurrentInternet);
		godot::ClassDB::bind_method(godot::D_METHOD("get_current_internet_site"), &UniverseSimulation::GetCurrentInternetSite);
		godot::ClassDB::bind_method(godot::D_METHOD("get_current_internet_page"), &UniverseSimulation::GetCurrentInternetPage);
		godot::ClassDB::bind_method(godot::D_METHOD("request_internet_url", "internet_url"), &UniverseSimulation::RequestInternetURL);

		// ####### Faction Roles #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_role_info", "role_id"), &UniverseSimulation::GetRoleInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_permission_info", "permission_id"), &UniverseSimulation::GetPermissionInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_entity_role", "faction_id", "entity_id"), &UniverseSimulation::GetEntityRole);
		godot::ClassDB::bind_method(godot::D_METHOD("add_faction_role", "faction_id", "role_id", "role_info"), &UniverseSimulation::AddFactionRole);
		godot::ClassDB::bind_method(godot::D_METHOD("remove_faction_role", "faction_id", "role_id"), &UniverseSimulation::RemoveFactionRole);
		godot::ClassDB::bind_method(godot::D_METHOD("modify_faction_role", "faction_id", "role_id", "role_info"), &UniverseSimulation::ModifyFactionRole);
		godot::ClassDB::bind_method(godot::D_METHOD("add_permission_to_role", "faction_id", "role_id", "permission_id"), &UniverseSimulation::AddPermissionToRole);
		godot::ClassDB::bind_method(godot::D_METHOD("remove_permission_from_role", "faction_id", "role_id", "permission_id"), &UniverseSimulation::RemovePermissionFromRole);
		godot::ClassDB::bind_method(godot::D_METHOD("set_entity_role", "faction_id", "entity_id", "role_id"), &UniverseSimulation::SetEntityRole);
		godot::ClassDB::bind_method(godot::D_METHOD("entity_has_permission", "faction_id", "entity_id", "permission_id"), &UniverseSimulation::EntityHasPermission);

		// ####### Faction (is entity) #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_faction_info", "faction_id"), &UniverseSimulation::GetFactionInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_joined_factions"), &UniverseSimulation::GetJoinedFactions);
		godot::ClassDB::bind_method(godot::D_METHOD("join_faction", "faction_id", "request_info"), &UniverseSimulation::JoinFaction);
		godot::ClassDB::bind_method(godot::D_METHOD("leave_faction", "faction_id"), &UniverseSimulation::LeaveFaction);
		godot::ClassDB::bind_method(godot::D_METHOD("invite_entity_to_faction", "faction_id", "entity_id"), &UniverseSimulation::InviteEntityToFaction);
		godot::ClassDB::bind_method(godot::D_METHOD("kick_entity_from_faction", "faction_id", "entity_id"), &UniverseSimulation::KickEntityFromFaction);
		godot::ClassDB::bind_method(godot::D_METHOD("add_child_faction", "parent_faction_id", "child_faction_id"), &UniverseSimulation::AddChildFaction);
		godot::ClassDB::bind_method(godot::D_METHOD("remove_child_faction", "faction_id"), &UniverseSimulation::RemoveChildFaction);
		godot::ClassDB::bind_method(godot::D_METHOD("invite_child_faction", "parent_faction_id", "child_faction_id"), &UniverseSimulation::InviteChildFaction);
		godot::ClassDB::bind_method(godot::D_METHOD("kick_child_faction", "faction_id"), &UniverseSimulation::KickChildFaction);

		// ####### Player Faction (is faction) #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_global_player_faction"), &UniverseSimulation::GetGlobalPlayerFaction);
		godot::ClassDB::bind_method(godot::D_METHOD("get_player_faction"), &UniverseSimulation::GetPlayerFaction);
		godot::ClassDB::bind_method(godot::D_METHOD("request_join_player_faction", "faction_id", "message"), &UniverseSimulation::RequestJoinPlayerFaction);

		// ####### Language #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_language_info", "language_id"), &UniverseSimulation::GetLanguageInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_language_translation", "language_id", "string"), &UniverseSimulation::GetLanguageTranslation);
		godot::ClassDB::bind_method(godot::D_METHOD("get_language_string", "language_id", "string_id"), &UniverseSimulation::GetLanguageString);

		// ####### Culture #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_culture_info", "culture_id"), &UniverseSimulation::GetCultureInfo);

		// ####### Level #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_level"), &UniverseSimulation::GetLevel);
		godot::ClassDB::bind_method(godot::D_METHOD("get_experience"), &UniverseSimulation::GetExperience);
		godot::ClassDB::bind_method(godot::D_METHOD("complete_level_up", "levelup_choices"), &UniverseSimulation::CompleteLevelUp);

		// ####### Player Control #######

		godot::ClassDB::bind_method(godot::D_METHOD("set_move_forwards", "is_moving"), &UniverseSimulation::SetMoveForwards);
		godot::ClassDB::bind_method(godot::D_METHOD("set_move_backwards", "is_moving"), &UniverseSimulation::SetMoveBackwards);
		godot::ClassDB::bind_method(godot::D_METHOD("set_move_left", "is_moving"), &UniverseSimulation::SetMoveLeft);
		godot::ClassDB::bind_method(godot::D_METHOD("set_move_right", "is_moving"), &UniverseSimulation::SetMoveRight);
		godot::ClassDB::bind_method(godot::D_METHOD("set_move_up", "is_moving"), &UniverseSimulation::SetMoveUp);
		godot::ClassDB::bind_method(godot::D_METHOD("set_move_down", "is_moving"), &UniverseSimulation::SetMoveDown);
		godot::ClassDB::bind_method(godot::D_METHOD("set_rotate_left", "is_rotating"), &UniverseSimulation::SetRotateLeft);
		godot::ClassDB::bind_method(godot::D_METHOD("set_rotate_right", "is_rotating"), &UniverseSimulation::SetRotateRight);
		godot::ClassDB::bind_method(godot::D_METHOD("set_look_direction", "direction"), &UniverseSimulation::SetLookDirection);
		godot::ClassDB::bind_method(godot::D_METHOD("set_sprint", "is_sprinting"), &UniverseSimulation::SetSprint);
		godot::ClassDB::bind_method(godot::D_METHOD("set_crouching", "is_crouching"), &UniverseSimulation::SetCrouching);
		godot::ClassDB::bind_method(godot::D_METHOD("set_prone", "is_prone"), &UniverseSimulation::SetProne);
		godot::ClassDB::bind_method(godot::D_METHOD("jump", "power"), &UniverseSimulation::Jump);

		// ####### Looking at #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_looking_at_entity"), &UniverseSimulation::GetLookingAtEntity);
		godot::ClassDB::bind_method(godot::D_METHOD("get_looking_at_volume"), &UniverseSimulation::GetLookingAtVolume);
		godot::ClassDB::bind_method(godot::D_METHOD("get_looking_at_block"), &UniverseSimulation::GetLookingAtBlock);

		// ####### Inventory #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_inventory_info", "inventory_id"), &UniverseSimulation::GetInventoryInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("get_inventory"), &UniverseSimulation::GetInventory);
		godot::ClassDB::bind_method(godot::D_METHOD("get_inventory_item_entity", "inventory_id", "item_index"), &UniverseSimulation::GetInventoryItemEntity);
		godot::ClassDB::bind_method(godot::D_METHOD("trash_inventory_item", "inventory_id", "item_index"), &UniverseSimulation::TrashInventoryItem);
		godot::ClassDB::bind_method(godot::D_METHOD("move_inventory_item", "inventory_id", "from_item_index", "to_item_index"), &UniverseSimulation::MoveInventoryItem);
		godot::ClassDB::bind_method(godot::D_METHOD("transfer_inventory_item", "from_inventory_id", "from_item_index", "to_inventory_id", "to_item_index"), &UniverseSimulation::TransferInventoryItem);
		godot::ClassDB::bind_method(godot::D_METHOD("interact_with_inventory_item", "inventory_id", "item_index", "interaction_info"), &UniverseSimulation::InteractWithInventoryItem);

		// ####### Interact #######
		godot::ClassDB::bind_method(godot::D_METHOD("store_entity", "entity_id", "inventory_id"), &UniverseSimulation::StoreEntity);
		godot::ClassDB::bind_method(godot::D_METHOD("hold_block", "volume_id", "position"), &UniverseSimulation::HoldBlock);
		godot::ClassDB::bind_method(godot::D_METHOD("hold_entity", "entity_id"), &UniverseSimulation::HoldEntity);
		godot::ClassDB::bind_method(godot::D_METHOD("drop_held_entity"), &UniverseSimulation::DropHeldEntity);

		godot::ClassDB::bind_method(godot::D_METHOD("equip_item_from_world", "entity_id"), &UniverseSimulation::EquipItemFromWorld);
		godot::ClassDB::bind_method(godot::D_METHOD("equip_item_from_inventory", "entity_id", "inventory_id", "item_index"), &UniverseSimulation::EquipItemFromInventory);
		godot::ClassDB::bind_method(godot::D_METHOD("drop_equip_to_world", "entity_id"), &UniverseSimulation::DropEquipToWorld);
		godot::ClassDB::bind_method(godot::D_METHOD("unequip_item_to_inventory", "entity_id", "inventory_id", "item_index"), &UniverseSimulation::UnequipItemToInventory);
		godot::ClassDB::bind_method(godot::D_METHOD("set_left_hand_equip", "entity_id"), &UniverseSimulation::SetLeftHandEquip);
		godot::ClassDB::bind_method(godot::D_METHOD("set_right_hand_equip", "entity_id"), &UniverseSimulation::SetRightHandEquip);
		godot::ClassDB::bind_method(godot::D_METHOD("use_equip", "entity_id", "hand"), &UniverseSimulation::UseEquip);
		godot::ClassDB::bind_method(godot::D_METHOD("toggle_equip", "entity_id", "toggled"), &UniverseSimulation::ToggleEquip);

		godot::ClassDB::bind_method(godot::D_METHOD("ride_entity", "entity_id", "attachment_point"), &UniverseSimulation::RideEntity);
		godot::ClassDB::bind_method(godot::D_METHOD("change_attachment_point", "new_attachment_point"), &UniverseSimulation::ChangeAttachmentPoint);
		godot::ClassDB::bind_method(godot::D_METHOD("exit_entity", "entity_id"), &UniverseSimulation::ExitEntity);

		godot::ClassDB::bind_method(godot::D_METHOD("interact_with_entity", "entity_id", "interaction_info"), &UniverseSimulation::InteractWithEntity);

		// ####### Vehicle Control #######

		godot::ClassDB::bind_method(godot::D_METHOD("accelerate", "is_accelerating"), &UniverseSimulation::Accelerate);
		godot::ClassDB::bind_method(godot::D_METHOD("decelerate", "is_decelerating"), &UniverseSimulation::Deccelerate);
		godot::ClassDB::bind_method(godot::D_METHOD("activate_vehicle_control", "system_id"), &UniverseSimulation::ActivateVehicleControl);
		godot::ClassDB::bind_method(godot::D_METHOD("toggle_vehicle_control", "system_id", "toggled"), &UniverseSimulation::ToggleVehicleControl);
		godot::ClassDB::bind_method(godot::D_METHOD("set_vehicle_setting", "system_id", "value"), &UniverseSimulation::SetVehicleSetting);

		// ####### Abilities #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_ability_info", "ability_id"), &UniverseSimulation::GetAbilityInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("activate_ability", "ability_id"), &UniverseSimulation::ActivateAbility);
		godot::ClassDB::bind_method(godot::D_METHOD("toggle_ability", "ability_id", "toggled"), &UniverseSimulation::ToggleAbility);
		godot::ClassDB::bind_method(godot::D_METHOD("set_player_setting", "setting_id", "value"), &UniverseSimulation::SetPlayerSetting);

		// ####### Magic #######

		godot::ClassDB::bind_method(godot::D_METHOD("get_spell_info", "spell_id"), &UniverseSimulation::GetSpellInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("use_spell", "spell_index", "params"), &UniverseSimulation::UseSpell);

		// ####### Testing #######

		godot::ClassDB::bind_method(godot::D_METHOD("create_instance", "mesh", "scenario"), &UniverseSimulation::CreateInstance);
		godot::ClassDB::bind_method(godot::D_METHOD("set_instance_pos", "instance_id", "pos"), &UniverseSimulation::SetInstancePos);
		godot::ClassDB::bind_method(godot::D_METHOD("delete_instance", "instance_id"), &UniverseSimulation::DeleteInstance);
	}

	void UniverseSimulation::_bind_methods()
	{
		BindEnums();
		BindMethods();
		BindSignals();
	}

	void UniverseSimulation::_cleanup_methods()
	{
		CleanupSignals();
	}
}