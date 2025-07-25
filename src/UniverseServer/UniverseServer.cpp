#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

#include "UniverseSimulation.h"
#include "Components.h"
#include "Modules.h"

#include "Commands/CommandServer.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/project_settings.hpp>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/type_info.hpp>

#include <easy/profiler.h>

#define BIND_METHOD godot::ClassDB::bind_method

namespace voxel_game
{
	const size_t k_simulation_ticks_per_second = 20;

	godot::OptObj<UniverseServer> UniverseServer::k_singleton;

	std::optional<const UniverseServer::SignalStrings> UniverseServer::k_signals;

	UniverseServer* UniverseServer::get_singleton()
	{
		return &k_singleton.value();
	}

	UniverseServer::UniverseServer()
	{}

	UniverseServer::~UniverseServer()
	{
		WaitUntilStopped();
	}

	bool UniverseServer::CanSimulationStart()
	{
		return true;
	}

	void UniverseServer::DoSimulationLoad()
	{
		m_simulation = std::make_unique<Simulation>();

		m_simulation->modules.push_back(simulation_module_schematic);
		m_simulation->modules.push_back(rendering_module_schematic);
		m_simulation->modules.push_back(debugrender_module_schematic);
		m_simulation->modules.push_back(spatial3d_module_schematic);
		m_simulation->modules.push_back(universe_module_schematic);
		m_simulation->modules.push_back(galaxy_module_schematic);

		SimulationInitialize(*m_simulation);

#if defined(DEBUG_ENABLED)
		m_info_updater.SetWriterThread(std::this_thread::get_id());
#endif
	}

	void UniverseServer::DoSimulationUnload()
	{
#if defined(DEBUG_ENABLED)
		m_info_updater.SetWriterThread(std::thread::id{}); // We may not start in thread mode next time
#endif

		SimulationUnload(*m_simulation);

		m_player_entity = entity::Ref();
		m_dimension_entity = entity::Ref();

		m_universe_entity = entity::Ref();
		m_galaxy_entity = entity::Ref();
		m_starsystem_entity = entity::Ref();

		m_world_entity = entity::Ref();
		m_spacestation_entity = entity::Ref();
		m_spaceship_entity = entity::Ref();
		m_vehicle_entity = entity::Ref();

		m_avatar_entity = entity::Ref();

		SimulationUninitialize(*m_simulation);

		m_simulation.reset();
	}

	void UniverseServer::DoSimulationProgress(real_t delta)
	{
		EASY_FUNCTION();

		if (IsThreaded())
		{
			// If we we are threaded then get the latest info cache data
			m_info_updater.RetrieveUpdates(m_info_cache);
		}
		else
		{
			SimulationUpdate(*m_simulation);

			QueueSignal(k_signals->update_debug_info, GenerateDebugInfo());
		}
	}

	void UniverseServer::DoSimulationThreadProgress()
	{
		EASY_FUNCTION();

		SimulationUpdate(*m_simulation);

		QueueSignal(k_signals->update_debug_info, GenerateDebugInfo());

		// Publish updates to the info caches to be read on the main thread
		m_info_updater.PublishUpdates();
	}

#if defined(DEBUG_ENABLED)
	void UniverseServer::DebugCommand(const godot::StringName& command, const godot::Array& args)
	{
		if (DeferCommand<&UniverseServer::DebugCommand>(command, args))
		{
			return;
		}

		if (command == godot::StringName("set_debug_camera_transform"))
		{
			if (args[0].get_type() != godot::Variant::TRANSFORM3D)
			{
				DEBUG_PRINT_WARN("The first argument of set_debug_camera_transform was not a transform");
				return;
			}

			if (!m_galaxy_entity)
			{
				return;
			}

			godot::Transform3D transform = args[0];

			m_galaxy_entity->*&CPosition::position = transform.origin;
		}
		else
		{
			DEBUG_PRINT_WARN(godot::vformat("Unknown debug command: %s", command));
		}
	}
#endif

	godot::String UniverseServer::GenerateDebugInfo()
	{
		godot::String debug_info;

		debug_info += godot::vformat("FPS: %d\n", godot::Engine::get_singleton()->get_frames_per_second());
		debug_info += godot::vformat("Frame Index: %d\n", m_simulation->frame_index);
		debug_info += "\n";

		size_t node_count = 0;
		for (spatial3d::ScalePtr scale : m_simulation->spatial_scales)
		{
			node_count += (scale->*&spatial3d::Scale::nodes).size();
		}
		debug_info += godot::vformat("Spatial Worlds: %d\n", m_simulation->spatial_worlds.size());
		debug_info += godot::vformat("Spatial Scales: %d\n", m_simulation->spatial_scales.size());
		debug_info += godot::vformat("Nodes: %d\n", node_count);
		debug_info += "\n";

		debug_info += godot::vformat("Universes: %d\n", m_simulation->universes.size());
		debug_info += godot::vformat("Galaxies: %d\n", m_simulation->galaxies.size());
		debug_info += "\n";

		return debug_info;
	}

	void UniverseServer::_bind_methods()
	{
		k_signals.emplace();

		BIND_ENUM_CONSTANT(SERVER_TYPE_LOCAL);
		BIND_ENUM_CONSTANT(SERVER_TYPE_REMOTE);

#if defined(DEBUG_ENABLED)
		BIND_METHOD(godot::D_METHOD("debug_command", "command", "arguments"), &UniverseServer::DebugCommand);
#endif
		BIND_METHOD(godot::D_METHOD("get_universe_info"), &UniverseServer::GetUniverseInfo);
		BIND_METHOD(godot::D_METHOD("connect_to_universe_list", "ip"), &UniverseServer::ConnectToUniverseList);
		BIND_METHOD(godot::D_METHOD("disconnect_from_universe_list"), &UniverseServer::DisconnectFromUniverseList);
		BIND_METHOD(godot::D_METHOD("query_universe_list", "query"), &UniverseServer::QueryUniverseList);
		BIND_METHOD(godot::D_METHOD("ping_remote_universe", "ip"), &UniverseServer::PingRemoteUniverse);
		BIND_METHOD(godot::D_METHOD("start_local_universe", "path", "fragment_type", "server_type", "scenario"), &UniverseServer::StartLocalUniverse);
		BIND_METHOD(godot::D_METHOD("connect_to_universe", "path", "ip", "scenario"), &UniverseServer::ConnectToUniverse);
		BIND_METHOD(godot::D_METHOD("disconnect_from_universe"), &UniverseServer::DisconnectFromUniverse);
		BIND_METHOD(godot::D_METHOD("get_fragment_info", "fragment_id"), &UniverseServer::GetFragmentInfo);
		BIND_METHOD(godot::D_METHOD("get_current_fragment"), &UniverseServer::GetCurrentFragment);
		BIND_METHOD(godot::D_METHOD("enter_fragment", "fragment_id", "method"), &UniverseServer::EnterFragment);
		BIND_METHOD(godot::D_METHOD("get_account_info"), &UniverseServer::GetAccountInfo);
		BIND_METHOD(godot::D_METHOD("create_account", "username", "password_hash"), &UniverseServer::CreateAccount);
		BIND_METHOD(godot::D_METHOD("account_login", "username", "password_hash"), &UniverseServer::AccountLogin);
		BIND_METHOD(godot::D_METHOD("saved_session_login"), &UniverseServer::SavedSessionLogin);
		BIND_METHOD(godot::D_METHOD("clear_saved_session"), &UniverseServer::ClearSavedSession);
		BIND_METHOD(godot::D_METHOD("delete_account"), &UniverseServer::DeleteAccount);
		BIND_METHOD(godot::D_METHOD("logout_account"), &UniverseServer::LogoutAccount);
		BIND_METHOD(godot::D_METHOD("get_friends"), &UniverseServer::GetFriends);
		BIND_METHOD(godot::D_METHOD("invite_friend", "account_id"), &UniverseServer::InviteFriend);
		BIND_METHOD(godot::D_METHOD("accept_friend_invite", "account_id"), &UniverseServer::AcceptFreindInvite);
		BIND_METHOD(godot::D_METHOD("remove_friend", "account_id"), &UniverseServer::RemoveFriend);
		BIND_METHOD(godot::D_METHOD("get_channel_info", "channel_id"), &UniverseServer::GetChannelInfo);
		BIND_METHOD(godot::D_METHOD("send_message_to_channel", "message", "channel_id"), &UniverseServer::SendMessageToChannel);
		BIND_METHOD(godot::D_METHOD("send_message_to_player", "message", "channel_id"), &UniverseServer::SendMessageToPlayer);
		BIND_METHOD(godot::D_METHOD("get_chat_channel_history", "channel_id"), &UniverseServer::GetChatChannelHistory);
		BIND_METHOD(godot::D_METHOD("get_private_chat_history", "account_id"), &UniverseServer::GetPrivateChatHistory);
		BIND_METHOD(godot::D_METHOD("get_player_info", "player_id"), &UniverseServer::GetPlayerInfo);
		BIND_METHOD(godot::D_METHOD("get_party_info", "party_host_id"), &UniverseServer::GetPartyInfo);
		BIND_METHOD(godot::D_METHOD("create_party"), &UniverseServer::CreateParty);
		BIND_METHOD(godot::D_METHOD("invite_to_party", "player_id"), &UniverseServer::InviteToParty);
		BIND_METHOD(godot::D_METHOD("accept_invite", "player_id"), &UniverseServer::AcceptInvite);
		BIND_METHOD(godot::D_METHOD("kick_from_party"), &UniverseServer::KickFromParty);
		BIND_METHOD(godot::D_METHOD("leave_party"), &UniverseServer::LeaveParty);
		BIND_METHOD(godot::D_METHOD("get_players_in_party"), &UniverseServer::GetPlayersInParty);
		BIND_METHOD(godot::D_METHOD("get_party_chat_channel"), &UniverseServer::GetPartyChatChannel);
		BIND_METHOD(godot::D_METHOD("get_entity_info", "entity_id"), &UniverseServer::GetEntityInfo);
		BIND_METHOD(godot::D_METHOD("request_entity_info", "entity_id"), &UniverseServer::RequestEntityInfo);
		BIND_METHOD(godot::D_METHOD("get_volume_info", "volume_id"), &UniverseServer::GetVolumeInfo);
		BIND_METHOD(godot::D_METHOD("get_block_info", "volume_id", "position"), &UniverseServer::GetBlockInfo);
		BIND_METHOD(godot::D_METHOD("place_block", "volume_id", "position", "block_id", "block_data"), &UniverseServer::PlaceBlock);
		BIND_METHOD(godot::D_METHOD("fill_blocks", "volume_id", "position_first", "position_second", "block_id", "block_data"), &UniverseServer::FillBlocks);
		BIND_METHOD(godot::D_METHOD("place_block_in_new_volume", "fragment_position", "block_id", "block_data"), &UniverseServer::PlaceBlockInNewVolume);
		BIND_METHOD(godot::D_METHOD("interact_block", "volume_id", "position", "interaction"), &UniverseServer::InteractBlock);
		BIND_METHOD(godot::D_METHOD("get_entity_position_in_volume", "volume_id", "entity_id"), &UniverseServer::GetEntityPositionInVolume);
		BIND_METHOD(godot::D_METHOD("fragment_position_to_volume_position", "volume_id", "fragment_position"), &UniverseServer::FragmentPositionToVolumePosition);
		BIND_METHOD(godot::D_METHOD("volume_position_to_fragment_position", "volume_id", "volume_position"), &UniverseServer::VolumePositionToFragmentPosition);
		BIND_METHOD(godot::D_METHOD("get_galaxy_info"), &UniverseServer::GetGalaxyInfo);
		BIND_METHOD(godot::D_METHOD("get_galaxy_region_info", "galaxy_region_id"), &UniverseServer::GetGalaxyRegionInfo);
		BIND_METHOD(godot::D_METHOD("request_galaxy_region_info", "galaxy_region_id"), &UniverseServer::RequestGalaxyRegionInfo);
		BIND_METHOD(godot::D_METHOD("get_current_galaxy_regions"), &UniverseServer::GetCurrentGalaxyRegions);
		BIND_METHOD(godot::D_METHOD("get_galaxy_object_info", "galaxy_object_id"), &UniverseServer::GetGalaxyObjectInfo);
		BIND_METHOD(godot::D_METHOD("request_galaxy_object_info", "entity_id"), &UniverseServer::RequestGalaxyObjectInfo);
		BIND_METHOD(godot::D_METHOD("get_currency_info", "currency_id"), &UniverseServer::GetCurrencyInfo);
		BIND_METHOD(godot::D_METHOD("get_bank_info", "bank_id"), &UniverseServer::GetBankInfo);
		BIND_METHOD(godot::D_METHOD("get_bank_interface_info", "bank_interface_id"), &UniverseServer::GetBankInterfaceInfo);
		BIND_METHOD(godot::D_METHOD("get_good_info", "good_id"), &UniverseServer::GetGoodInfo);
		BIND_METHOD(godot::D_METHOD("get_universal_currency"), &UniverseServer::GetUniversalCurrency);
		BIND_METHOD(godot::D_METHOD("get_bank_of_interface", "bank_interface_id"), &UniverseServer::GetBankOfInterface);
		BIND_METHOD(godot::D_METHOD("get_owned_currencies"), &UniverseServer::GetOwnedCurrencies);
		BIND_METHOD(godot::D_METHOD("get_balance", "currency_id"), &UniverseServer::GetBalance);
		BIND_METHOD(godot::D_METHOD("withdraw", "currency_id", "amount", "bank_interface_id"), &UniverseServer::Withdraw);
		BIND_METHOD(godot::D_METHOD("deposit", "currency_id", "amount", "bank_interface_id"), &UniverseServer::Deposit);
		BIND_METHOD(godot::D_METHOD("convert", "from_currency_id", "from_currency_id", "amount", "bank_interface_id"), &UniverseServer::Convert);
		BIND_METHOD(godot::D_METHOD("pay_entity", "currency_id", "entity_id", "amount", "bank_interface_id"), &UniverseServer::PayEntity);
		BIND_METHOD(godot::D_METHOD("buy_good_with_currency", "good_id", "currency_id"), &UniverseServer::BuyGoodWithCurrency);
		BIND_METHOD(godot::D_METHOD("get_internet_info", "internet_id"), &UniverseServer::GetInternetInfo);
		BIND_METHOD(godot::D_METHOD("get_website_info", "website_id"), &UniverseServer::GetWebsiteInfo);
		BIND_METHOD(godot::D_METHOD("get_website_page_info", "website_page_id"), &UniverseServer::GetWebsitePageInfo);
		BIND_METHOD(godot::D_METHOD("get_internet_websites", "internet_id"), &UniverseServer::GetInternetWebsites);
		BIND_METHOD(godot::D_METHOD("get_website_pages", "website_id"), &UniverseServer::GetWebsitePages);
		BIND_METHOD(godot::D_METHOD("start_internet", "internet_id", "device_id"), &UniverseServer::StartInternet);
		BIND_METHOD(godot::D_METHOD("stop_internet"), &UniverseServer::StopInternet);
		BIND_METHOD(godot::D_METHOD("get_current_internet"), &UniverseServer::GetCurrentInternet);
		BIND_METHOD(godot::D_METHOD("get_current_internet_site"), &UniverseServer::GetCurrentInternetSite);
		BIND_METHOD(godot::D_METHOD("get_current_internet_page"), &UniverseServer::GetCurrentInternetPage);
		BIND_METHOD(godot::D_METHOD("request_internet_url", "internet_url"), &UniverseServer::RequestInternetURL);
		BIND_METHOD(godot::D_METHOD("get_role_info", "role_id"), &UniverseServer::GetRoleInfo);
		BIND_METHOD(godot::D_METHOD("get_permission_info", "permission_id"), &UniverseServer::GetPermissionInfo);
		BIND_METHOD(godot::D_METHOD("get_entity_role", "faction_id", "entity_id"), &UniverseServer::GetEntityRole);
		BIND_METHOD(godot::D_METHOD("add_faction_role", "faction_id", "role_id", "role_info"), &UniverseServer::AddFactionRole);
		BIND_METHOD(godot::D_METHOD("remove_faction_role", "faction_id", "role_id"), &UniverseServer::RemoveFactionRole);
		BIND_METHOD(godot::D_METHOD("modify_faction_role", "faction_id", "role_id", "role_info"), &UniverseServer::ModifyFactionRole);
		BIND_METHOD(godot::D_METHOD("add_permission_to_role", "faction_id", "role_id", "permission_id"), &UniverseServer::AddPermissionToRole);
		BIND_METHOD(godot::D_METHOD("remove_permission_from_role", "faction_id", "role_id", "permission_id"), &UniverseServer::RemovePermissionFromRole);
		BIND_METHOD(godot::D_METHOD("set_entity_role", "faction_id", "entity_id", "role_id"), &UniverseServer::SetEntityRole);
		BIND_METHOD(godot::D_METHOD("entity_has_permission", "faction_id", "entity_id", "permission_id"), &UniverseServer::EntityHasPermission);
		BIND_METHOD(godot::D_METHOD("get_faction_info", "faction_id"), &UniverseServer::GetFactionInfo);
		BIND_METHOD(godot::D_METHOD("get_joined_factions"), &UniverseServer::GetJoinedFactions);
		BIND_METHOD(godot::D_METHOD("join_faction", "faction_id", "request_info"), &UniverseServer::JoinFaction);
		BIND_METHOD(godot::D_METHOD("leave_faction", "faction_id"), &UniverseServer::LeaveFaction);
		BIND_METHOD(godot::D_METHOD("invite_entity_to_faction", "faction_id", "entity_id"), &UniverseServer::InviteEntityToFaction);
		BIND_METHOD(godot::D_METHOD("kick_entity_from_faction", "faction_id", "entity_id"), &UniverseServer::KickEntityFromFaction);
		BIND_METHOD(godot::D_METHOD("add_child_faction", "parent_faction_id", "child_faction_id"), &UniverseServer::AddChildFaction);
		BIND_METHOD(godot::D_METHOD("remove_child_faction", "faction_id"), &UniverseServer::RemoveChildFaction);
		BIND_METHOD(godot::D_METHOD("invite_child_faction", "parent_faction_id", "child_faction_id"), &UniverseServer::InviteChildFaction);
		BIND_METHOD(godot::D_METHOD("kick_child_faction", "faction_id"), &UniverseServer::KickChildFaction);
		BIND_METHOD(godot::D_METHOD("get_global_player_faction"), &UniverseServer::GetGlobalPlayerFaction);
		BIND_METHOD(godot::D_METHOD("get_player_faction"), &UniverseServer::GetPlayerFaction);
		BIND_METHOD(godot::D_METHOD("request_join_player_faction", "faction_id", "message"), &UniverseServer::RequestJoinPlayerFaction);
		BIND_METHOD(godot::D_METHOD("get_language_info", "language_id"), &UniverseServer::GetLanguageInfo);
		BIND_METHOD(godot::D_METHOD("get_language_translation", "language_id", "string"), &UniverseServer::GetLanguageTranslation);
		BIND_METHOD(godot::D_METHOD("get_language_string", "language_id", "string_id"), &UniverseServer::GetLanguageString);
		BIND_METHOD(godot::D_METHOD("get_culture_info", "culture_id"), &UniverseServer::GetCultureInfo);
		BIND_METHOD(godot::D_METHOD("get_level"), &UniverseServer::GetLevel);
		BIND_METHOD(godot::D_METHOD("get_experience"), &UniverseServer::GetExperience);
		BIND_METHOD(godot::D_METHOD("complete_level_up", "levelup_choices"), &UniverseServer::CompleteLevelUp);
		BIND_METHOD(godot::D_METHOD("set_sprint", "is_sprinting"), &UniverseServer::SetSprint);
		BIND_METHOD(godot::D_METHOD("set_crouching", "is_crouching"), &UniverseServer::SetCrouching);
		BIND_METHOD(godot::D_METHOD("set_prone", "is_prone"), &UniverseServer::SetProne);
		BIND_METHOD(godot::D_METHOD("do_walk", "velocity"), &UniverseServer::DoWalk);
		BIND_METHOD(godot::D_METHOD("do_look", "direction"), &UniverseServer::DoLook);
		BIND_METHOD(godot::D_METHOD("do_jump", "power"), &UniverseServer::DoJump);
		BIND_METHOD(godot::D_METHOD("get_looking_at_entity"), &UniverseServer::GetLookingAtEntity);
		BIND_METHOD(godot::D_METHOD("get_looking_at_volume"), &UniverseServer::GetLookingAtVolume);
		BIND_METHOD(godot::D_METHOD("get_looking_at_block"), &UniverseServer::GetLookingAtBlock);
		BIND_METHOD(godot::D_METHOD("get_inventory_info", "inventory_id"), &UniverseServer::GetInventoryInfo);
		BIND_METHOD(godot::D_METHOD("get_inventory"), &UniverseServer::GetInventory);
		BIND_METHOD(godot::D_METHOD("get_inventory_item_entity", "inventory_id", "item_index"), &UniverseServer::GetInventoryItemEntity);
		BIND_METHOD(godot::D_METHOD("trash_inventory_item", "inventory_id", "item_index"), &UniverseServer::TrashInventoryItem);
		BIND_METHOD(godot::D_METHOD("move_inventory_item", "inventory_id", "from_item_index", "to_item_index"), &UniverseServer::MoveInventoryItem);
		BIND_METHOD(godot::D_METHOD("transfer_inventory_item", "from_inventory_id", "from_item_index", "to_inventory_id", "to_item_index"), &UniverseServer::TransferInventoryItem);
		BIND_METHOD(godot::D_METHOD("interact_with_inventory_item", "inventory_id", "item_index", "interaction_info"), &UniverseServer::InteractWithInventoryItem);
		BIND_METHOD(godot::D_METHOD("store_entity", "entity_id", "inventory_id"), &UniverseServer::StoreEntity);
		BIND_METHOD(godot::D_METHOD("hold_block", "volume_id", "position"), &UniverseServer::HoldBlock);
		BIND_METHOD(godot::D_METHOD("hold_entity", "entity_id"), &UniverseServer::HoldEntity);
		BIND_METHOD(godot::D_METHOD("drop_held_entity"), &UniverseServer::DropHeldEntity);
		BIND_METHOD(godot::D_METHOD("equip_item_from_world", "entity_id"), &UniverseServer::EquipItemFromWorld);
		BIND_METHOD(godot::D_METHOD("equip_item_from_inventory", "entity_id", "inventory_id", "item_index"), &UniverseServer::EquipItemFromInventory);
		BIND_METHOD(godot::D_METHOD("drop_equip_to_world", "entity_id"), &UniverseServer::DropEquipToWorld);
		BIND_METHOD(godot::D_METHOD("unequip_item_to_inventory", "entity_id", "inventory_id", "item_index"), &UniverseServer::UnequipItemToInventory);
		BIND_METHOD(godot::D_METHOD("set_left_hand_equip", "entity_id"), &UniverseServer::SetLeftHandEquip);
		BIND_METHOD(godot::D_METHOD("set_right_hand_equip", "entity_id"), &UniverseServer::SetRightHandEquip);
		BIND_METHOD(godot::D_METHOD("use_equip", "entity_id", "hand"), &UniverseServer::UseEquip);
		BIND_METHOD(godot::D_METHOD("toggle_equip", "entity_id", "toggled"), &UniverseServer::ToggleEquip);
		BIND_METHOD(godot::D_METHOD("ride_entity", "entity_id", "attachment_point"), &UniverseServer::RideEntity);
		BIND_METHOD(godot::D_METHOD("change_attachment_point", "new_attachment_point"), &UniverseServer::ChangeAttachmentPoint);
		BIND_METHOD(godot::D_METHOD("exit_entity", "entity_id"), &UniverseServer::ExitEntity);
		BIND_METHOD(godot::D_METHOD("interact_with_entity", "entity_id", "interaction_info"), &UniverseServer::InteractWithEntity);
		BIND_METHOD(godot::D_METHOD("trigger_vehicle_control", "control_id"), &UniverseServer::TriggerVehicleControl);
		BIND_METHOD(godot::D_METHOD("toggle_vehicle_control", "control_id", "toggled"), &UniverseServer::ToggleVehicleControl);
		BIND_METHOD(godot::D_METHOD("set_vehicle_control", "control_id", "value"), &UniverseServer::SetVehicleControl);
		BIND_METHOD(godot::D_METHOD("get_ability_info", "ability_id"), &UniverseServer::GetAbilityInfo);
		BIND_METHOD(godot::D_METHOD("activate_ability", "ability_id"), &UniverseServer::ActivateAbility);
		BIND_METHOD(godot::D_METHOD("toggle_ability", "ability_id", "toggled"), &UniverseServer::ToggleAbility);
		BIND_METHOD(godot::D_METHOD("set_player_setting", "setting_id", "value"), &UniverseServer::SetPlayerSetting);
		BIND_METHOD(godot::D_METHOD("get_spell_info", "spell_id"), &UniverseServer::GetSpellInfo);
		BIND_METHOD(godot::D_METHOD("use_spell", "spell_index", "params"), &UniverseServer::UseSpell);

		ADD_SIGNAL(godot::MethodInfo(k_signals->update_debug_info));
		ADD_SIGNAL(godot::MethodInfo(k_signals->connected_to_universe_list));
		ADD_SIGNAL(godot::MethodInfo(k_signals->disconnected_from_universe_list));
		ADD_SIGNAL(godot::MethodInfo(k_signals->universe_list_query_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->universe_ping_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->connected_to_universe));
		ADD_SIGNAL(godot::MethodInfo(k_signals->disconnected_from_universe));
		ADD_SIGNAL(godot::MethodInfo(k_signals->connected_to_remote));
		ADD_SIGNAL(godot::MethodInfo(k_signals->disonnected_from_remote));
		ADD_SIGNAL(godot::MethodInfo(k_signals->fragment_added));
		ADD_SIGNAL(godot::MethodInfo(k_signals->fragment_removed));
		ADD_SIGNAL(godot::MethodInfo(k_signals->fragment_loaded));
		ADD_SIGNAL(godot::MethodInfo(k_signals->fragment_unloaded));
		ADD_SIGNAL(godot::MethodInfo(k_signals->account_create_request_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->account_login_request_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->account_delete_request_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->account_logout_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->account_invite_friend_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->account_friend_request_received));
		ADD_SIGNAL(godot::MethodInfo(k_signals->account_remove_friend_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->account_friend_unfriended));
		ADD_SIGNAL(godot::MethodInfo(k_signals->chat_message_received));
		ADD_SIGNAL(godot::MethodInfo(k_signals->chat_channel_info_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->player_joined_fragment));
		ADD_SIGNAL(godot::MethodInfo(k_signals->player_left_fragment));
		ADD_SIGNAL(godot::MethodInfo(k_signals->party_create_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->party_invited));
		ADD_SIGNAL(godot::MethodInfo(k_signals->party_invitation_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->party_kicked));
		ADD_SIGNAL(godot::MethodInfo(k_signals->party_player_joined));
		ADD_SIGNAL(godot::MethodInfo(k_signals->party_player_left));
		ADD_SIGNAL(godot::MethodInfo(k_signals->entity_info_request_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->volume_block_place_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->volume_block_fill_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->new_volume_block_place_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->volume_block_interact_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_region_info_request_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_region_entered));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_region_exited));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_object_info_request_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->currency_withdraw_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->currency_deposit_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->currency_convert_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->currency_pay_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->currency_buy_good_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->internet_start_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->internet_stop_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->internet_url_request_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_role_add_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_role_remove_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_role_modify_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_permission_add_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_permission_remove_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_set_entity_role_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_join_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_leave_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_entity_invite_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_invite_received));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_entity_kick_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_child_add_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_child_remove_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_child_invite_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_kick_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->faction_request_join_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->levelup_available));
		ADD_SIGNAL(godot::MethodInfo(k_signals->complete_levelup_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->hovered_over_entity));
		ADD_SIGNAL(godot::MethodInfo(k_signals->hovered_over_volume));
		ADD_SIGNAL(godot::MethodInfo(k_signals->hovered_over_block));
		ADD_SIGNAL(godot::MethodInfo(k_signals->inventory_trashed_item_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->inventory_moved_item_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->inventory_transfer_item_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->inventory_interact_item_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->inventory_force_closed));
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
		ADD_SIGNAL(godot::MethodInfo(k_signals->vehicle_accelerate_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->vehicle_deccelerate_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->vehicle_control_activate_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->vehicle_control_toggle_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->vehicle_set_setting_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->activate_ability_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->toggle_ability_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->player_set_setting_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->use_spell_response_response));

		k_singleton.instantiate();
	}

	void UniverseServer::_cleanup_methods()
	{
		k_singleton.reset();
		k_signals.reset();
	}
}