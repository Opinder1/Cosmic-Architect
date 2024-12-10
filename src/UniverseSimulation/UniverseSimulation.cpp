#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"
#include "Universe.h"

#include "Universe/UniverseComponents.h"
#include "Universe/UniverseModule.h"

#include "Galaxy/GalaxyComponents.h"
#include "Galaxy/GalaxyModule.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialModule.h"

#include "Voxel/VoxelComponents.h"
#include "Voxel/VoxelModule.h"

#include "Render/RenderComponents.h"
#include "Render/TreeComponents.h"
#include "Render/RenderModule.h"

#include "Physics3D/PhysicsComponents.h"
#include "Physics3D/PhysicsModule.h"

#include "Simulation/SimulationComponents.h"
#include "Simulation/SimulationModule.h"

#include "Loading/LoadingComponents.h"
#include "Loading/LoadingModule.h"

#include "Util/CommandServer.h"
#include "Util/Debug.h"

#include <godot_cpp/classes/os.hpp>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/type_info.hpp>

#include <easy/profiler.h>

#define BIND_METHOD godot::ClassDB::bind_method

namespace voxel_game
{
	const size_t k_simulation_ticks_per_second = 20;

	flecs::entity CreateNewUniverse(flecs::world& world, const godot::String& path, godot::RID scenario)
	{
		// Create the universe
		flecs::entity universe_entity(world, DEBUG_ONLY("Universe"));

		universe_entity.add<universe::World>();
		universe_entity.add<spatial3d::World>();
		universe_entity.emplace<sim::Path>(path);

		spatial3d::AddScaleMarkers(universe_entity);

		if (scenario.is_valid())
		{
			universe_entity.ensure<rendering::Scenario>().id = scenario;

			universe_entity.add<rendering::TreeNode>();
		}

		return universe_entity;
	}

	flecs::entity CreateNewSimulatedGalaxy(flecs::world& world, flecs::entity_t universe_entity, godot::RID scenario)
	{
		// Create the simulated galaxy
		flecs::entity galaxy_entity(world, DEBUG_ONLY("SimulatedGalaxy"));

		galaxy_entity.child_of(universe_entity);
		galaxy_entity.add<galaxy::World>();
		galaxy_entity.add<spatial3d::World>();
		galaxy_entity.add<physics3d::Position>();
		galaxy_entity.add<physics3d::Rotation>();

		// We want the simulated galaxy to load all galaxies around it
		spatial3d::Loader& spatial_loader = galaxy_entity.ensure<spatial3d::Loader>();

		spatial_loader.dist_per_lod = 3;
		spatial_loader.min_lod = 0;
		spatial_loader.max_lod = spatial3d::k_max_world_scale;

		spatial3d::AddScaleMarkers(galaxy_entity);

		if (scenario.is_valid())
		{
			galaxy_entity.add<rendering::TreeNode>();
		}

		return galaxy_entity;
	}

	std::optional<godot::StringName> UniverseSimulation::k_emit_signal;
	std::optional<const UniverseSimulation::CommandStrings> UniverseSimulation::k_commands;
	std::optional<const UniverseSimulation::SignalStrings> UniverseSimulation::k_signals;

	UniverseSimulation::UniverseSimulation()
	{}

	UniverseSimulation::~UniverseSimulation()
	{
		WaitUntilStopped();
	}

	void UniverseSimulation::Initialize(const godot::Ref<Universe>& universe, const godot::String& path, const godot::String& fragment_type, ServerType server_type, godot::RID scenario)
	{
		DEBUG_ASSERT(m_universe.is_null(), "We can't initialize a simulation twice");

		if (universe.is_null())
		{
			DEBUG_PRINT_ERROR("The universe must be valid");
			return;
		}

		m_universe = universe;

		m_fragment_type = fragment_type;

		m_world.reset();

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

		if (server_type == ServerType::SERVER_TYPE_REMOTE)
		{
			// Import networking
		}

		if (scenario.is_valid())
		{
			m_world.import<rendering::Module>();
		}

		m_world.emplace<loading::EntityLoader>(m_world);

		// Create the universe and simulated galaxy

		m_universe_entity = CreateNewUniverse(m_world, path, scenario);

		m_galaxy_entity = CreateNewSimulatedGalaxy(m_world, m_universe_entity, scenario);
	}

	bool UniverseSimulation::CanSimulationStart()
	{
		if (m_universe.is_null())
		{
			DEBUG_PRINT_ERROR("This universe simulation should have been instantiated by a universe");
			return false;
		}

		return true;
	}

	void UniverseSimulation::DoSimulationLoad()
	{
#if defined(DEBUG_ENABLED)
		m_info_updater.SetThreads(m_owner_id, std::this_thread::get_id());

		m_world.get_mut<loading::EntityLoader>()->SetProgressThread(std::this_thread::get_id());
#endif
	}

	void UniverseSimulation::DoSimulationUnload()
	{
		DEBUG_ASSERT(m_universe.is_valid(), "The simulation should have been initialized");

#if defined(DEBUG_ENABLED)
		m_world.get_mut<loading::EntityLoader>()->SetProgressThread(std::thread::id{});

		m_info_updater.SetThreads(std::thread::id{}, std::thread::id{});
#endif

		m_world.set_threads(0);

		m_world.reset();
	}

	bool UniverseSimulation::DoSimulationProgress(real_t delta)
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

	void UniverseSimulation::DoSimulationThreadProgress()
	{
		EASY_FUNCTION();

		m_world.progress();

		// Publish updates to the info caches to be read on the main thread
		m_info_updater.PublishUpdates();
	}

	godot::Ref<Universe> UniverseSimulation::GetUniverse()
	{
		return m_universe;
	}

#if defined(DEBUG_ENABLED)
	void UniverseSimulation::DebugCommand(const godot::StringName& command, const godot::Array& args)
	{
		if (DeferCommand(k_commands->debug_command, command, args))
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

			flecs::entity(m_world, m_galaxy_entity).set<physics3d::Position>({ transform.origin });
		}
		else
		{
			DEBUG_PRINT_WARN(godot::vformat("Unknown debug command: %s", command));
		}
	}
#endif

	void UniverseSimulation::_bind_methods()
	{
		k_emit_signal = godot::StringName("emit_signal", true);
		k_commands.emplace();
		k_signals.emplace();

		BIND_ENUM_CONSTANT(SERVER_TYPE_LOCAL);
		BIND_ENUM_CONSTANT(SERVER_TYPE_REMOTE);

		BIND_METHOD(godot::D_METHOD(k_commands->initialize, "universe", "path", "fragment_type", "server_type", "scenario"), &UniverseSimulation::Initialize);

		// ####### Universe #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_universe), &UniverseSimulation::GetUniverse);
#if defined(DEBUG_ENABLED)
		BIND_METHOD(godot::D_METHOD(k_commands->debug_command, "command", "arguments"), &UniverseSimulation::DebugCommand);
#endif

		// ####### Fragments (admin only) #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_fragment_info, "fragment_id"), &UniverseSimulation::GetFragmentInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_current_fragment), &UniverseSimulation::GetCurrentFragment);
		BIND_METHOD(godot::D_METHOD(k_commands->enter_fragment, "fragment_id", "method"), &UniverseSimulation::EnterFragment);

		// ####### Account #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_account_info), &UniverseSimulation::GetAccountInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->create_account, "username", "password_hash"), &UniverseSimulation::CreateAccount);
		BIND_METHOD(godot::D_METHOD(k_commands->account_login, "username", "password_hash"), &UniverseSimulation::AccountLogin);
		BIND_METHOD(godot::D_METHOD(k_commands->saved_session_login), &UniverseSimulation::SavedSessionLogin);
		BIND_METHOD(godot::D_METHOD(k_commands->clear_saved_session), &UniverseSimulation::ClearSavedSession);
		BIND_METHOD(godot::D_METHOD(k_commands->delete_account), &UniverseSimulation::DeleteAccount);
		BIND_METHOD(godot::D_METHOD(k_commands->logout_account), &UniverseSimulation::LogoutAccount);

		// ####### Friends #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_friends), &UniverseSimulation::GetFriends);
		BIND_METHOD(godot::D_METHOD(k_commands->invite_friend, "account_id"), &UniverseSimulation::InviteFriend);
		BIND_METHOD(godot::D_METHOD(k_commands->accept_friend_invite, "account_id"), &UniverseSimulation::AcceptFreindInvite);
		BIND_METHOD(godot::D_METHOD(k_commands->remove_friend, "account_id"), &UniverseSimulation::RemoveFriend);

		// ####### Chat #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_channel_info, "channel_id"), &UniverseSimulation::GetChannelInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->send_message_to_channel, "message", "channel_id"), &UniverseSimulation::SendMessageToChannel);
		BIND_METHOD(godot::D_METHOD(k_commands->send_message_to_player, "message", "channel_id"), &UniverseSimulation::SendMessageToPlayer);
		BIND_METHOD(godot::D_METHOD(k_commands->get_chat_channel_history, "channel_id"), &UniverseSimulation::GetChatChannelHistory);
		BIND_METHOD(godot::D_METHOD(k_commands->get_private_chat_history, "account_id"), &UniverseSimulation::GetPrivateChatHistory);

		// ####### Players #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_player_info, "player_id"), &UniverseSimulation::GetPlayerInfo);

		// ####### Party #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_party_info, "party_host_id"), &UniverseSimulation::GetPartyInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->create_party), &UniverseSimulation::CreateParty);
		BIND_METHOD(godot::D_METHOD(k_commands->invite_to_party, "player_id"), &UniverseSimulation::InviteToParty);
		BIND_METHOD(godot::D_METHOD(k_commands->accept_invite, "player_id"), &UniverseSimulation::AcceptInvite);
		BIND_METHOD(godot::D_METHOD(k_commands->kick_from_party), &UniverseSimulation::KickFromParty);
		BIND_METHOD(godot::D_METHOD(k_commands->leave_party), &UniverseSimulation::LeaveParty);
		BIND_METHOD(godot::D_METHOD(k_commands->get_players_in_party), &UniverseSimulation::GetPlayersInParty);
		BIND_METHOD(godot::D_METHOD(k_commands->get_party_chat_channel), &UniverseSimulation::GetPartyChatChannel);

		// ####### Entity #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_entity_info, "entity_id"), &UniverseSimulation::GetEntityInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->request_entity_info, "entity_id"), &UniverseSimulation::RequestEntityInfo);

		// ####### Volume (is entity) #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_volume_info, "volume_id"), &UniverseSimulation::GetVolumeInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_block_info, "volume_id", "position"), &UniverseSimulation::GetBlockInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->place_block, "volume_id", "position", "block_id", "block_data"), &UniverseSimulation::PlaceBlock);
		BIND_METHOD(godot::D_METHOD(k_commands->fill_blocks, "volume_id", "position_first", "position_second", "block_id", "block_data"), &UniverseSimulation::FillBlocks);
		BIND_METHOD(godot::D_METHOD(k_commands->place_block_in_new_volume, "fragment_position", "block_id", "block_data"), &UniverseSimulation::PlaceBlockInNewVolume);
		BIND_METHOD(godot::D_METHOD(k_commands->interact_block, "volume_id", "position", "interaction"), &UniverseSimulation::InteractBlock);
		BIND_METHOD(godot::D_METHOD(k_commands->get_entity_position_in_volume, "volume_id", "entity_id"), &UniverseSimulation::GetEntityPositionInVolume);
		BIND_METHOD(godot::D_METHOD(k_commands->fragment_position_to_volume_position, "volume_id", "fragment_position"), &UniverseSimulation::FragmentPositionToVolumePosition);
		BIND_METHOD(godot::D_METHOD(k_commands->volume_position_to_fragment_position, "volume_id", "volume_position"), &UniverseSimulation::VolumePositionToFragmentPosition);

		// ####### Galaxy Region #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_galaxy_info), &UniverseSimulation::GetGalaxyInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_galaxy_region_info, "galaxy_region_id"), &UniverseSimulation::GetGalaxyRegionInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->request_galaxy_region_info, "galaxy_region_id"), &UniverseSimulation::RequestGalaxyRegionInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_current_galaxy_regions), &UniverseSimulation::GetCurrentGalaxyRegions);

		// ####### Galaxy Object (is volume) #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_galaxy_object_info, "galaxy_object_id"), &UniverseSimulation::GetGalaxyObjectInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->request_galaxy_object_info, "entity_id"), &UniverseSimulation::RequestGalaxyObjectInfo);

		// ####### Currency #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_currency_info, "currency_id"), &UniverseSimulation::GetCurrencyInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_bank_info, "bank_id"), &UniverseSimulation::GetBankInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_bank_interface_info, "bank_interface_id"), &UniverseSimulation::GetBankInterfaceInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_good_info, "good_id"), &UniverseSimulation::GetGoodInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_universal_currency), &UniverseSimulation::GetUniversalCurrency);
		BIND_METHOD(godot::D_METHOD(k_commands->get_bank_of_interface, "bank_interface_id"), &UniverseSimulation::GetBankOfInterface);
		BIND_METHOD(godot::D_METHOD(k_commands->get_owned_currencies), &UniverseSimulation::GetOwnedCurrencies);
		BIND_METHOD(godot::D_METHOD(k_commands->get_balance, "currency_id"), &UniverseSimulation::GetBalance);
		BIND_METHOD(godot::D_METHOD(k_commands->withdraw, "currency_id", "amount", "bank_interface_id"), &UniverseSimulation::Withdraw);
		BIND_METHOD(godot::D_METHOD(k_commands->deposit, "currency_id", "amount", "bank_interface_id"), &UniverseSimulation::Deposit);
		BIND_METHOD(godot::D_METHOD(k_commands->convert, "from_currency_id", "from_currency_id", "amount", "bank_interface_id"), &UniverseSimulation::Convert);
		BIND_METHOD(godot::D_METHOD(k_commands->pay_entity, "currency_id", "entity_id", "amount", "bank_interface_id"), &UniverseSimulation::PayEntity);
		BIND_METHOD(godot::D_METHOD(k_commands->buy_good_with_currency, "good_id", "currency_id"), &UniverseSimulation::BuyGoodWithCurrency);

		// ####### Internet #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_internet_info, "internet_id"), &UniverseSimulation::GetInternetInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_website_info, "website_id"), &UniverseSimulation::GetWebsiteInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_website_page_info, "website_page_id"), &UniverseSimulation::GetWebsitePageInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_internet_websites, "internet_id"), &UniverseSimulation::GetInternetWebsites);
		BIND_METHOD(godot::D_METHOD(k_commands->get_website_pages, "website_id"), &UniverseSimulation::GetWebsitePages);
		BIND_METHOD(godot::D_METHOD(k_commands->start_internet, "internet_id", "device_id"), &UniverseSimulation::StartInternet);
		BIND_METHOD(godot::D_METHOD(k_commands->stop_internet), &UniverseSimulation::StopInternet);
		BIND_METHOD(godot::D_METHOD(k_commands->get_current_internet), &UniverseSimulation::GetCurrentInternet);
		BIND_METHOD(godot::D_METHOD(k_commands->get_current_internet_site), &UniverseSimulation::GetCurrentInternetSite);
		BIND_METHOD(godot::D_METHOD(k_commands->get_current_internet_page), &UniverseSimulation::GetCurrentInternetPage);
		BIND_METHOD(godot::D_METHOD(k_commands->request_internet_url, "internet_url"), &UniverseSimulation::RequestInternetURL);

		// ####### Faction Roles #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_role_info, "role_id"), &UniverseSimulation::GetRoleInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_permission_info, "permission_id"), &UniverseSimulation::GetPermissionInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_entity_role, "faction_id", "entity_id"), &UniverseSimulation::GetEntityRole);
		BIND_METHOD(godot::D_METHOD(k_commands->add_faction_role, "faction_id", "role_id", "role_info"), &UniverseSimulation::AddFactionRole);
		BIND_METHOD(godot::D_METHOD(k_commands->remove_faction_role, "faction_id", "role_id"), &UniverseSimulation::RemoveFactionRole);
		BIND_METHOD(godot::D_METHOD(k_commands->modify_faction_role, "faction_id", "role_id", "role_info"), &UniverseSimulation::ModifyFactionRole);
		BIND_METHOD(godot::D_METHOD(k_commands->add_permission_to_role, "faction_id", "role_id", "permission_id"), &UniverseSimulation::AddPermissionToRole);
		BIND_METHOD(godot::D_METHOD(k_commands->remove_permission_from_role, "faction_id", "role_id", "permission_id"), &UniverseSimulation::RemovePermissionFromRole);
		BIND_METHOD(godot::D_METHOD(k_commands->set_entity_role, "faction_id", "entity_id", "role_id"), &UniverseSimulation::SetEntityRole);
		BIND_METHOD(godot::D_METHOD(k_commands->entity_has_permission, "faction_id", "entity_id", "permission_id"), &UniverseSimulation::EntityHasPermission);

		// ####### Faction (is entity) #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_faction_info, "faction_id"), &UniverseSimulation::GetFactionInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_joined_factions), &UniverseSimulation::GetJoinedFactions);
		BIND_METHOD(godot::D_METHOD(k_commands->join_faction, "faction_id", "request_info"), &UniverseSimulation::JoinFaction);
		BIND_METHOD(godot::D_METHOD(k_commands->leave_faction, "faction_id"), &UniverseSimulation::LeaveFaction);
		BIND_METHOD(godot::D_METHOD(k_commands->invite_entity_to_faction, "faction_id", "entity_id"), &UniverseSimulation::InviteEntityToFaction);
		BIND_METHOD(godot::D_METHOD(k_commands->kick_entity_from_faction, "faction_id", "entity_id"), &UniverseSimulation::KickEntityFromFaction);
		BIND_METHOD(godot::D_METHOD(k_commands->add_child_faction, "parent_faction_id", "child_faction_id"), &UniverseSimulation::AddChildFaction);
		BIND_METHOD(godot::D_METHOD(k_commands->remove_child_faction, "faction_id"), &UniverseSimulation::RemoveChildFaction);
		BIND_METHOD(godot::D_METHOD(k_commands->invite_child_faction, "parent_faction_id", "child_faction_id"), &UniverseSimulation::InviteChildFaction);
		BIND_METHOD(godot::D_METHOD(k_commands->kick_child_faction, "faction_id"), &UniverseSimulation::KickChildFaction);

		// ####### Player Faction (is faction) #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_global_player_faction), &UniverseSimulation::GetGlobalPlayerFaction);
		BIND_METHOD(godot::D_METHOD(k_commands->get_player_faction), &UniverseSimulation::GetPlayerFaction);
		BIND_METHOD(godot::D_METHOD(k_commands->request_join_player_faction, "faction_id", "message"), &UniverseSimulation::RequestJoinPlayerFaction);

		// ####### Language #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_language_info, "language_id"), &UniverseSimulation::GetLanguageInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_language_translation, "language_id", "string"), &UniverseSimulation::GetLanguageTranslation);
		BIND_METHOD(godot::D_METHOD(k_commands->get_language_string, "language_id", "string_id"), &UniverseSimulation::GetLanguageString);

		// ####### Culture #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_culture_info, "culture_id"), &UniverseSimulation::GetCultureInfo);

		// ####### Level #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_level), &UniverseSimulation::GetLevel);
		BIND_METHOD(godot::D_METHOD(k_commands->get_experience), &UniverseSimulation::GetExperience);
		BIND_METHOD(godot::D_METHOD(k_commands->complete_level_up, "levelup_choices"), &UniverseSimulation::CompleteLevelUp);

		// ####### Player Control #######

		BIND_METHOD(godot::D_METHOD(k_commands->set_sprint, "is_sprinting"), &UniverseSimulation::SetSprint);
		BIND_METHOD(godot::D_METHOD(k_commands->set_crouching, "is_crouching"), &UniverseSimulation::SetCrouching);
		BIND_METHOD(godot::D_METHOD(k_commands->set_prone, "is_prone"), &UniverseSimulation::SetProne);
		BIND_METHOD(godot::D_METHOD(k_commands->do_walk, "velocity"), &UniverseSimulation::DoWalk);
		BIND_METHOD(godot::D_METHOD(k_commands->do_look, "direction"), &UniverseSimulation::DoLook);
		BIND_METHOD(godot::D_METHOD(k_commands->do_jump, "power"), &UniverseSimulation::DoJump);

		// ####### Looking at #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_looking_at_entity), &UniverseSimulation::GetLookingAtEntity);
		BIND_METHOD(godot::D_METHOD(k_commands->get_looking_at_volume), &UniverseSimulation::GetLookingAtVolume);
		BIND_METHOD(godot::D_METHOD(k_commands->get_looking_at_block), &UniverseSimulation::GetLookingAtBlock);

		// ####### Inventory #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_inventory_info, "inventory_id"), &UniverseSimulation::GetInventoryInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->get_inventory), &UniverseSimulation::GetInventory);
		BIND_METHOD(godot::D_METHOD(k_commands->get_inventory_item_entity, "inventory_id", "item_index"), &UniverseSimulation::GetInventoryItemEntity);
		BIND_METHOD(godot::D_METHOD(k_commands->trash_inventory_item, "inventory_id", "item_index"), &UniverseSimulation::TrashInventoryItem);
		BIND_METHOD(godot::D_METHOD(k_commands->move_inventory_item, "inventory_id", "from_item_index", "to_item_index"), &UniverseSimulation::MoveInventoryItem);
		BIND_METHOD(godot::D_METHOD(k_commands->transfer_inventory_item, "from_inventory_id", "from_item_index", "to_inventory_id", "to_item_index"), &UniverseSimulation::TransferInventoryItem);
		BIND_METHOD(godot::D_METHOD(k_commands->interact_with_inventory_item, "inventory_id", "item_index", "interaction_info"), &UniverseSimulation::InteractWithInventoryItem);

		// ####### Interact #######
		BIND_METHOD(godot::D_METHOD(k_commands->store_entity, "entity_id", "inventory_id"), &UniverseSimulation::StoreEntity);
		BIND_METHOD(godot::D_METHOD(k_commands->hold_block, "volume_id", "position"), &UniverseSimulation::HoldBlock);
		BIND_METHOD(godot::D_METHOD(k_commands->hold_entity, "entity_id"), &UniverseSimulation::HoldEntity);
		BIND_METHOD(godot::D_METHOD(k_commands->drop_held_entity), &UniverseSimulation::DropHeldEntity);

		BIND_METHOD(godot::D_METHOD(k_commands->equip_item_from_world, "entity_id"), &UniverseSimulation::EquipItemFromWorld);
		BIND_METHOD(godot::D_METHOD(k_commands->equip_item_from_inventory, "entity_id", "inventory_id", "item_index"), &UniverseSimulation::EquipItemFromInventory);
		BIND_METHOD(godot::D_METHOD(k_commands->drop_equip_to_world, "entity_id"), &UniverseSimulation::DropEquipToWorld);
		BIND_METHOD(godot::D_METHOD(k_commands->unequip_item_to_inventory, "entity_id", "inventory_id", "item_index"), &UniverseSimulation::UnequipItemToInventory);
		BIND_METHOD(godot::D_METHOD(k_commands->set_left_hand_equip, "entity_id"), &UniverseSimulation::SetLeftHandEquip);
		BIND_METHOD(godot::D_METHOD(k_commands->set_right_hand_equip, "entity_id"), &UniverseSimulation::SetRightHandEquip);
		BIND_METHOD(godot::D_METHOD(k_commands->use_equip, "entity_id", "hand"), &UniverseSimulation::UseEquip);
		BIND_METHOD(godot::D_METHOD(k_commands->toggle_equip, "entity_id", "toggled"), &UniverseSimulation::ToggleEquip);

		BIND_METHOD(godot::D_METHOD(k_commands->ride_entity, "entity_id", "attachment_point"), &UniverseSimulation::RideEntity);
		BIND_METHOD(godot::D_METHOD(k_commands->change_attachment_point, "new_attachment_point"), &UniverseSimulation::ChangeAttachmentPoint);
		BIND_METHOD(godot::D_METHOD(k_commands->exit_entity, "entity_id"), &UniverseSimulation::ExitEntity);

		BIND_METHOD(godot::D_METHOD(k_commands->interact_with_entity, "entity_id", "interaction_info"), &UniverseSimulation::InteractWithEntity);

		// ####### Vehicle Control #######

		BIND_METHOD(godot::D_METHOD(k_commands->trigger_vehicle_control, "control_id"), &UniverseSimulation::TriggerVehicleControl);
		BIND_METHOD(godot::D_METHOD(k_commands->toggle_vehicle_control, "control_id", "toggled"), &UniverseSimulation::ToggleVehicleControl);
		BIND_METHOD(godot::D_METHOD(k_commands->set_vehicle_control, "control_id", "value"), &UniverseSimulation::SetVehicleControl);

		// ####### Abilities #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_ability_info, "ability_id"), &UniverseSimulation::GetAbilityInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->activate_ability, "ability_id"), &UniverseSimulation::ActivateAbility);
		BIND_METHOD(godot::D_METHOD(k_commands->toggle_ability, "ability_id", "toggled"), &UniverseSimulation::ToggleAbility);
		BIND_METHOD(godot::D_METHOD(k_commands->set_player_setting, "setting_id", "value"), &UniverseSimulation::SetPlayerSetting);

		// ####### Magic #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_spell_info, "spell_id"), &UniverseSimulation::GetSpellInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->use_spell, "spell_index", "params"), &UniverseSimulation::UseSpell);

		// ####### Universe #######

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
	}

	void UniverseSimulation::_cleanup_methods()
	{
		k_commands.reset();
		k_signals.reset();
	}
}