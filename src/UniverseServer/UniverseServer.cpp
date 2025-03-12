#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

#include "Universe/UniverseModule.h"

#include "Galaxy/GalaxyModule.h"
#include "GalaxyRender/GalaxyRenderModule.h"

#include "Spatial3D/SpatialModule.h"

#include "Voxel/VoxelModule.h"

#include "VoxelRender/VoxelRenderModule.h"

#include "Render/RenderModule.h"
#include "Render/RenderComponents.h"

#include "Simulation/SimulationModule.h"

#include "Physics3D/PhysicsModule.h"
#include "Physics3D/PhysicsComponents.h"

#include "Loading/LoadingModule.h"
#include "Loading/LoadingComponents.h"

#include "Commands/CommandServer.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/os.hpp>
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

	void UniverseServer::SetRenderScenario(godot::RID scenario)
	{
		m_scenario = scenario;
	}

	bool UniverseServer::CanSimulationStart()
	{
		return true;
	}

	void UniverseServer::DoSimulationLoad()
	{
		m_world = flecs::world();

		m_world.set_threads(godot::OS::get_singleton()->get_processor_count());

		m_world.set_target_fps(k_simulation_ticks_per_second);

		// Import modules
#if defined(DEBUG_ENABLED)
		m_world.import<flecs::stats>();
		m_world.set<flecs::Rest>({});
#endif

		m_world.import<loading::Module>();
		m_world.import<sim::Module>();
		m_world.import<physics3d::Module>();
		m_world.import<spatial3d::Module>();
		m_world.import<voxel::Module>();
		m_world.import<galaxy::Module>();
		m_world.import<universe::Module>();

		if (rendering::IsEnabled())
		{
			m_world.import<rendering::Module>();
			m_world.import<galaxyrender::Module>();
			m_world.import<voxelrender::Module>();
		}

		// Create the universe

		m_path = godot::ProjectSettings::get_singleton()->get_setting("voxel_game/universe/path");

		m_universe_entity = universe::CreateNewUniverse(m_world, m_path);

#if defined(DEBUG_ENABLED)
		m_info_updater.SetWriterThread(std::this_thread::get_id());
#endif
	}

	void UniverseServer::DoSimulationUnload()
	{
#if defined(DEBUG_ENABLED)
		m_info_updater.SetWriterThread(std::thread::id{}); // We may not start in thread mode next time
#endif

		m_world.set_threads(0);

		m_world = flecs::world();
	}

	bool UniverseServer::DoSimulationProgress(real_t delta)
	{
		EASY_FUNCTION();

		if (IsThreaded())
		{
			// If we we are threaded then get the latest info cache data
			m_info_updater.RetrieveUpdates(m_info_cache);

			return true; // We always keep running when threaded as the thread will stop at its own pace
		}
		else
		{
			return m_world.progress(static_cast<ecs_ftime_t>(delta));
		}
	}

	void UniverseServer::DoSimulationThreadProgress()
	{
		EASY_FUNCTION();

		m_world.progress();

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

			godot::Transform3D transform = args[0];

			flecs::entity(m_world, m_galaxy_entity).set<physics3d::CPosition>({ transform.origin });
		}
		else
		{
			DEBUG_PRINT_WARN(godot::vformat("Unknown debug command: %s", command));
		}
	}
#endif

	void UniverseServer::_bind_methods()
	{
		k_signals.emplace();

		BIND_ENUM_CONSTANT(SERVER_TYPE_LOCAL);
		BIND_ENUM_CONSTANT(SERVER_TYPE_REMOTE);

#if defined(DEBUG_ENABLED)
		BIND_METHOD(godot::D_METHOD("debug_command", "command", "arguments"), &UniverseServer::DebugCommand);
#endif

		// ####### Universe #######

		BIND_METHOD(godot::D_METHOD("get_universe_info"), &UniverseServer::GetUniverseInfo);
		BIND_METHOD(godot::D_METHOD("connect_to_galaxy_list", "ip"), &UniverseServer::ConnectToGalaxyList);
		BIND_METHOD(godot::D_METHOD("disconnect_from_galaxy_list"), &UniverseServer::DisconnectFromGalaxyList);
		BIND_METHOD(godot::D_METHOD("query_galaxy_list", "query"), &UniverseServer::QueryGalaxyList);
		BIND_METHOD(godot::D_METHOD("ping_remote_galaxy", "ip"), &UniverseServer::PingRemoteGalaxy);
		BIND_METHOD(godot::D_METHOD("start_local_galaxy", "path", "fragment_type", "server_type"), &UniverseServer::StartLocalGalaxy);
		BIND_METHOD(godot::D_METHOD("connect_to_galaxy", "path", "ip"), &UniverseServer::ConnectToGalaxy);
		BIND_METHOD(godot::D_METHOD("disconnect_from_galaxy"), &UniverseServer::DisconnectFromGalaxy);

		// ####### Fragments (admin only) #######

		BIND_METHOD(godot::D_METHOD("get_fragment_info", "fragment_id"), &UniverseServer::GetFragmentInfo);
		BIND_METHOD(godot::D_METHOD("get_current_fragment"), &UniverseServer::GetCurrentFragment);
		BIND_METHOD(godot::D_METHOD("enter_fragment", "fragment_id", "method"), &UniverseServer::EnterFragment);

		// ####### Account #######

		BIND_METHOD(godot::D_METHOD("get_account_info"), &UniverseServer::GetAccountInfo);
		BIND_METHOD(godot::D_METHOD("create_account", "username", "password_hash"), &UniverseServer::CreateAccount);
		BIND_METHOD(godot::D_METHOD("account_login", "username", "password_hash"), &UniverseServer::AccountLogin);
		BIND_METHOD(godot::D_METHOD("saved_session_login"), &UniverseServer::SavedSessionLogin);
		BIND_METHOD(godot::D_METHOD("clear_saved_session"), &UniverseServer::ClearSavedSession);
		BIND_METHOD(godot::D_METHOD("delete_account"), &UniverseServer::DeleteAccount);
		BIND_METHOD(godot::D_METHOD("logout_account"), &UniverseServer::LogoutAccount);

		// ####### Friends #######

		BIND_METHOD(godot::D_METHOD("get_friends"), &UniverseServer::GetFriends);
		BIND_METHOD(godot::D_METHOD("invite_friend", "account_id"), &UniverseServer::InviteFriend);
		BIND_METHOD(godot::D_METHOD("accept_friend_invite", "account_id"), &UniverseServer::AcceptFreindInvite);
		BIND_METHOD(godot::D_METHOD("remove_friend", "account_id"), &UniverseServer::RemoveFriend);

		// ####### Chat #######

		BIND_METHOD(godot::D_METHOD("get_channel_info", "channel_id"), &UniverseServer::GetChannelInfo);
		BIND_METHOD(godot::D_METHOD("send_message_to_channel", "message", "channel_id"), &UniverseServer::SendMessageToChannel);
		BIND_METHOD(godot::D_METHOD("send_message_to_player", "message", "channel_id"), &UniverseServer::SendMessageToPlayer);
		BIND_METHOD(godot::D_METHOD("get_chat_channel_history", "channel_id"), &UniverseServer::GetChatChannelHistory);
		BIND_METHOD(godot::D_METHOD("get_private_chat_history", "account_id"), &UniverseServer::GetPrivateChatHistory);

		// ####### Players #######

		BIND_METHOD(godot::D_METHOD("get_player_info", "player_id"), &UniverseServer::GetPlayerInfo);

		// ####### Party #######

		BIND_METHOD(godot::D_METHOD("get_party_info", "party_host_id"), &UniverseServer::GetPartyInfo);
		BIND_METHOD(godot::D_METHOD("create_party"), &UniverseServer::CreateParty);
		BIND_METHOD(godot::D_METHOD("invite_to_party", "player_id"), &UniverseServer::InviteToParty);
		BIND_METHOD(godot::D_METHOD("accept_invite", "player_id"), &UniverseServer::AcceptInvite);
		BIND_METHOD(godot::D_METHOD("kick_from_party"), &UniverseServer::KickFromParty);
		BIND_METHOD(godot::D_METHOD("leave_party"), &UniverseServer::LeaveParty);
		BIND_METHOD(godot::D_METHOD("get_players_in_party"), &UniverseServer::GetPlayersInParty);
		BIND_METHOD(godot::D_METHOD("get_party_chat_channel"), &UniverseServer::GetPartyChatChannel);

		// ####### Entity #######

		BIND_METHOD(godot::D_METHOD("get_entity_info", "entity_id"), &UniverseServer::GetEntityInfo);
		BIND_METHOD(godot::D_METHOD("request_entity_info", "entity_id"), &UniverseServer::RequestEntityInfo);

		// ####### Volume (is entity) #######

		BIND_METHOD(godot::D_METHOD("get_volume_info", "volume_id"), &UniverseServer::GetVolumeInfo);
		BIND_METHOD(godot::D_METHOD("get_block_info", "volume_id", "position"), &UniverseServer::GetBlockInfo);
		BIND_METHOD(godot::D_METHOD("place_block", "volume_id", "position", "block_id", "block_data"), &UniverseServer::PlaceBlock);
		BIND_METHOD(godot::D_METHOD("fill_blocks", "volume_id", "position_first", "position_second", "block_id", "block_data"), &UniverseServer::FillBlocks);
		BIND_METHOD(godot::D_METHOD("place_block_in_new_volume", "fragment_position", "block_id", "block_data"), &UniverseServer::PlaceBlockInNewVolume);
		BIND_METHOD(godot::D_METHOD("interact_block", "volume_id", "position", "interaction"), &UniverseServer::InteractBlock);
		BIND_METHOD(godot::D_METHOD("get_entity_position_in_volume", "volume_id", "entity_id"), &UniverseServer::GetEntityPositionInVolume);
		BIND_METHOD(godot::D_METHOD("fragment_position_to_volume_position", "volume_id", "fragment_position"), &UniverseServer::FragmentPositionToVolumePosition);
		BIND_METHOD(godot::D_METHOD("volume_position_to_fragment_position", "volume_id", "volume_position"), &UniverseServer::VolumePositionToFragmentPosition);

		// ####### Galaxy Region #######

		BIND_METHOD(godot::D_METHOD("get_galaxy_info"), &UniverseServer::GetGalaxyInfo);
		BIND_METHOD(godot::D_METHOD("get_galaxy_region_info", "galaxy_region_id"), &UniverseServer::GetGalaxyRegionInfo);
		BIND_METHOD(godot::D_METHOD("request_galaxy_region_info", "galaxy_region_id"), &UniverseServer::RequestGalaxyRegionInfo);
		BIND_METHOD(godot::D_METHOD("get_current_galaxy_regions"), &UniverseServer::GetCurrentGalaxyRegions);

		// ####### Galaxy Object (is volume) #######

		BIND_METHOD(godot::D_METHOD("get_galaxy_object_info", "galaxy_object_id"), &UniverseServer::GetGalaxyObjectInfo);
		BIND_METHOD(godot::D_METHOD("request_galaxy_object_info", "entity_id"), &UniverseServer::RequestGalaxyObjectInfo);

		// ####### Currency #######

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

		// ####### Internet #######

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

		// ####### Faction Roles #######

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

		// ####### Faction (is entity) #######

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

		// ####### Player Faction (is faction) #######

		BIND_METHOD(godot::D_METHOD("get_global_player_faction"), &UniverseServer::GetGlobalPlayerFaction);
		BIND_METHOD(godot::D_METHOD("get_player_faction"), &UniverseServer::GetPlayerFaction);
		BIND_METHOD(godot::D_METHOD("request_join_player_faction", "faction_id", "message"), &UniverseServer::RequestJoinPlayerFaction);

		// ####### Language #######

		BIND_METHOD(godot::D_METHOD("get_language_info", "language_id"), &UniverseServer::GetLanguageInfo);
		BIND_METHOD(godot::D_METHOD("get_language_translation", "language_id", "string"), &UniverseServer::GetLanguageTranslation);
		BIND_METHOD(godot::D_METHOD("get_language_string", "language_id", "string_id"), &UniverseServer::GetLanguageString);

		// ####### Culture #######

		BIND_METHOD(godot::D_METHOD("get_culture_info", "culture_id"), &UniverseServer::GetCultureInfo);

		// ####### Level #######

		BIND_METHOD(godot::D_METHOD("get_level"), &UniverseServer::GetLevel);
		BIND_METHOD(godot::D_METHOD("get_experience"), &UniverseServer::GetExperience);
		BIND_METHOD(godot::D_METHOD("complete_level_up", "levelup_choices"), &UniverseServer::CompleteLevelUp);

		// ####### Player Control #######

		BIND_METHOD(godot::D_METHOD("set_sprint", "is_sprinting"), &UniverseServer::SetSprint);
		BIND_METHOD(godot::D_METHOD("set_crouching", "is_crouching"), &UniverseServer::SetCrouching);
		BIND_METHOD(godot::D_METHOD("set_prone", "is_prone"), &UniverseServer::SetProne);
		BIND_METHOD(godot::D_METHOD("do_walk", "velocity"), &UniverseServer::DoWalk);
		BIND_METHOD(godot::D_METHOD("do_look", "direction"), &UniverseServer::DoLook);
		BIND_METHOD(godot::D_METHOD("do_jump", "power"), &UniverseServer::DoJump);

		// ####### Looking at #######

		BIND_METHOD(godot::D_METHOD("get_looking_at_entity"), &UniverseServer::GetLookingAtEntity);
		BIND_METHOD(godot::D_METHOD("get_looking_at_volume"), &UniverseServer::GetLookingAtVolume);
		BIND_METHOD(godot::D_METHOD("get_looking_at_block"), &UniverseServer::GetLookingAtBlock);

		// ####### Inventory #######

		BIND_METHOD(godot::D_METHOD("get_inventory_info", "inventory_id"), &UniverseServer::GetInventoryInfo);
		BIND_METHOD(godot::D_METHOD("get_inventory"), &UniverseServer::GetInventory);
		BIND_METHOD(godot::D_METHOD("get_inventory_item_entity", "inventory_id", "item_index"), &UniverseServer::GetInventoryItemEntity);
		BIND_METHOD(godot::D_METHOD("trash_inventory_item", "inventory_id", "item_index"), &UniverseServer::TrashInventoryItem);
		BIND_METHOD(godot::D_METHOD("move_inventory_item", "inventory_id", "from_item_index", "to_item_index"), &UniverseServer::MoveInventoryItem);
		BIND_METHOD(godot::D_METHOD("transfer_inventory_item", "from_inventory_id", "from_item_index", "to_inventory_id", "to_item_index"), &UniverseServer::TransferInventoryItem);
		BIND_METHOD(godot::D_METHOD("interact_with_inventory_item", "inventory_id", "item_index", "interaction_info"), &UniverseServer::InteractWithInventoryItem);

		// ####### Interact #######
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

		// ####### Vehicle Control #######

		BIND_METHOD(godot::D_METHOD("trigger_vehicle_control", "control_id"), &UniverseServer::TriggerVehicleControl);
		BIND_METHOD(godot::D_METHOD("toggle_vehicle_control", "control_id", "toggled"), &UniverseServer::ToggleVehicleControl);
		BIND_METHOD(godot::D_METHOD("set_vehicle_control", "control_id", "value"), &UniverseServer::SetVehicleControl);

		// ####### Abilities #######

		BIND_METHOD(godot::D_METHOD("get_ability_info", "ability_id"), &UniverseServer::GetAbilityInfo);
		BIND_METHOD(godot::D_METHOD("activate_ability", "ability_id"), &UniverseServer::ActivateAbility);
		BIND_METHOD(godot::D_METHOD("toggle_ability", "ability_id", "toggled"), &UniverseServer::ToggleAbility);
		BIND_METHOD(godot::D_METHOD("set_player_setting", "setting_id", "value"), &UniverseServer::SetPlayerSetting);

		// ####### Magic #######

		BIND_METHOD(godot::D_METHOD("get_spell_info", "spell_id"), &UniverseServer::GetSpellInfo);
		BIND_METHOD(godot::D_METHOD("use_spell", "spell_index", "params"), &UniverseServer::UseSpell);

		// ####### Universe #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->connected_to_galaxy_list));
		ADD_SIGNAL(godot::MethodInfo(k_signals->disconnected_from_galaxy_list));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_list_query_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->galaxy_ping_response));
		ADD_SIGNAL(godot::MethodInfo(k_signals->connected_to_galaxy));
		ADD_SIGNAL(godot::MethodInfo(k_signals->disconnected_from_galaxy));

		// ####### Galaxy #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->connected_to_remote));
		ADD_SIGNAL(godot::MethodInfo(k_signals->disonnected_from_remote));

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

		k_singleton.instantiate();
	}

	void UniverseServer::_cleanup_methods()
	{
		k_singleton.reset();

		k_signals.reset();
	}
}