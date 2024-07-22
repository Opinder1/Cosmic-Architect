#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"
#include "Universe.h"
#include "UniverseRenderInfo.h"

#include "Universe/UniverseComponents.h"
#include "Universe/UniverseModule.h"

#include "Galaxy/GalaxyComponents.h"
#include "Galaxy/GalaxyModule.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialModule.h"

#include "Voxel/VoxelComponents.h"
#include "Voxel/VoxelModule.h"

#include "Render/RenderComponents.h"
#include "Render/RenderModule.h"

#include "Physics/PhysicsComponents.h"
#include "Physics/PhysicsModule.h"

#include "Simulation/SimulationComponents.h"
#include "Simulation/SimulationModule.h"

#include "Util/Debug.h"
#include "Util/PropertyMacros.h"

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/type_info.hpp>

#define BIND_METHOD godot::ClassDB::bind_method

namespace voxel_game
{
	size_t UUIDHash::operator()(const UUID& uuid) const
	{
		static_assert(sizeof(UUID) == 16);

		uint64_t* arr = (uint64_t*)&uuid;

		return arr[0] ^ arr[1];
	}

	UniverseCache::Info UniverseCache::* UniverseCache::GetInfo(Type type)
	{
		switch (type)
		{
		case Type::Galaxy:			return &UniverseCache::galaxy_info;
		case Type::Account:			return &UniverseCache::account_info;
		case Type::Player:			return &UniverseCache::player_info;
		default:					return nullptr;
		}
	}

	UniverseCache::InfoMap UniverseCache::* UniverseCache::GetInfoMap(Type type)
	{
		switch (type)
		{
		case Type::Player:			return &UniverseCache::player_info_map;
		case Type::Fragment:		return &UniverseCache::fragment_info_map;
		case Type::ChatChannel:		return &UniverseCache::chat_channel_info_map;
		case Type::Party:			return &UniverseCache::party_info_map;
		case Type::Entity:			return &UniverseCache::entity_info_map;
		case Type::Volume:			return &UniverseCache::volume_info_map;
		case Type::GalaxyRegion:	return &UniverseCache::galaxy_region_info_map;
		case Type::GalaxyObject:	return &UniverseCache::galaxy_object_info_map;
		case Type::Currency:		return &UniverseCache::currency_info_map;
		case Type::Bank:			return &UniverseCache::bank_info_map;
		case Type::BankInterface:	return &UniverseCache::bank_interface_info_map;
		case Type::Good:			return &UniverseCache::good_info_map;
		case Type::Internet:		return &UniverseCache::internet_info_map;
		case Type::Website:			return &UniverseCache::website_info_map;
		case Type::Webpage:			return &UniverseCache::webpage_info_map;
		case Type::Role:			return &UniverseCache::role_info_map;
		case Type::Permission:		return &UniverseCache::permission_info_map;
		case Type::Faction:			return &UniverseCache::faction_info_map;
		case Type::Language:		return &UniverseCache::language_info_map;
		case Type::Culture:			return &UniverseCache::culture_info_map;
		case Type::Inventory:		return &UniverseCache::inventory_info_map;
		case Type::Ability:			return &UniverseCache::ability_info_map;
		case Type::Spell:			return &UniverseCache::spell_info_map;
		default:					return nullptr;
		}
	}

	UniverseCacheUpdater::UniverseCacheUpdater()
	{}

	void UniverseCacheUpdater::UpdateInfo(UniverseCache::Type type, const UniverseCache::Info& info)
	{
		InfoUpdate update;

		update.info = UniverseCache::GetInfo(type);
		update.key = UUID();
		update.value = info;

		if (update.info == nullptr)
		{
			DEBUG_PRINT_ERROR("Invalid universe info type");
			return;
		}

		AddInfoUpdate(std::move(update));
	}

	void UniverseCacheUpdater::UpdateInfoMap(UniverseCache::Type type, UUID id, const UniverseCache::Info& info)
	{
		InfoUpdate update;

		update.info_map = UniverseCache::GetInfoMap(type);
		update.key = id;
		update.value = info;

		if (update.info_map == nullptr)
		{
			DEBUG_PRINT_ERROR("Invalid universe info map type");
			return;
		}

		AddInfoUpdate(std::move(update));
	}

	void UniverseCacheUpdater::AddInfoUpdate(InfoUpdate&& update)
	{
		m_write.emplace_back(std::move(update));
	}

	// Write the changes to the exchange buffer
	void UniverseCacheUpdater::PublishUpdates()
	{
		if (!m_ready.load(std::memory_order_acquire))
		{
			m_read = std::move(m_write);

			m_ready.store(true, std::memory_order_release);
		}
	}

	// Obtain the latest changes made by the writer if there are any
	void UniverseCacheUpdater::RetrieveUpdates(UniverseCache& out)
	{
		std::vector<InfoUpdate> updates;

		if (m_ready.load(std::memory_order_acquire))
		{
			updates = std::move(m_read);

			m_ready.store(false, std::memory_order_release);
		}

		for (const InfoUpdate& update : updates)
		{
			if (update.key == UUID())
			{
				(out.*update.info) = update.value;
			}
			else
			{
				(out.*update.info_map)[update.key] = update.value;
			}
		}
	}

	std::optional<godot::StringName> UniverseSimulation::k_emit_signal;
	std::optional<const UniverseSimulation::CommandStrings> UniverseSimulation::k_commands;
	std::optional<const UniverseSimulation::SignalStrings> UniverseSimulation::k_signals;

	UniverseSimulation::UniverseSimulation()
	{}

	UniverseSimulation::~UniverseSimulation()
	{
		DEBUG_ASSERT(!m_universe.is_valid(), "We should have uninitialized first");

		// Join the thread just in case
		if (IsThreaded())
		{
			m_thread.join();
		}
	}

	void UniverseSimulation::Initialize(const godot::Ref<Universe>& universe, const godot::String& path, const godot::String& fragment_type, ServerType server_type)
	{
		DEBUG_ASSERT(!m_universe.is_valid(), "We can't initialize a simulation twice");

		m_universe = godot::UtilityFunctions::weakref(universe);

		m_world.reset();

		m_world.set_threads(godot::OS::get_singleton()->get_processor_count());

		m_world.import<flecs::stats>();
		m_world.import<SimulationModule>();
		m_world.import<PhysicsModule>();
		m_world.import<SpatialModule>();
		m_world.import<VoxelModule>();
		m_world.import<GalaxyModule>();
		m_world.import<UniverseModule>();

		m_world.set<flecs::Rest>({});

		m_universe_entity = m_world.entity(DEBUG_ONLY("Universe"))
			.add<UniverseComponent>()
			.add<SpatialWorld3DComponent>();

		SpatialModule::AddSpatialScaleWorkers(m_world, m_universe_entity);

		{
			SpatialLoader3DComponent spatial_loader;

			spatial_loader.dist_per_lod = 3;
			spatial_loader.min_lod = 0;
			spatial_loader.max_lod = k_max_world_scale;

			m_galaxy_entity = m_world.entity(DEBUG_ONLY("SimulatedGalaxy"))
				.child_of(m_universe_entity)
				.add<GalaxyComponent>()
				.add<SpatialWorld3DComponent>()
				.add<Position3DComponent>()
				.add<Rotation3DComponent>()
				.emplace<SpatialLoader3DComponent>(std::move(spatial_loader));
		}

		SpatialModule::AddSpatialScaleWorkers(m_world, m_galaxy_entity);
	}

	void UniverseSimulation::Uninitialize()
	{
		DEBUG_ASSERT(m_universe.is_valid(), "The simulation should have been initialized");

		if (m_galaxy_load_state.load(std::memory_order_acquire) != LOAD_STATE_UNLOADED)
		{
			DEBUG_PRINT_ERROR("This galaxy should not be loaded when we uninitialize");
		}

		// First stop our thread
		if (IsThreaded())
		{
			m_thread.join();
		}

		m_world.set_threads(0);

		m_world.reset();

		emit_signal(k_signals->simulation_uninitialized);

		// Finally disconnect from our universe
		m_universe.unref();
	}

	bool UniverseSimulation::IsThreaded()
	{
		return m_thread.joinable();
	}

	godot::Ref<Universe> UniverseSimulation::GetUniverse()
	{
		return m_universe->get_ref();
	}

	godot::Dictionary UniverseSimulation::GetGalaxyInfo()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_info_cache.galaxy_info;
	}

	void UniverseSimulation::StartRenderer(UniverseRenderInfo* render_info)
	{
		m_world.import<RenderModule>();

		flecs::entity universe_entity(m_world.c_ptr(), m_universe_entity);
			
		universe_entity.ensure<RenderScenario>().id = render_info->GetScenario();
	}

	void UniverseSimulation::StartSimulation(ThreadMode thread_mode)
	{
		if (!m_universe.is_valid())
		{
			DEBUG_PRINT_ERROR("This universe simulation should have been instantiated by a universe");
			return;
		}

		if (m_galaxy_load_state.load(std::memory_order_acquire) != LOAD_STATE_UNLOADED)
		{
			DEBUG_PRINT_ERROR("This galaxy should not be loaded when we start");
			return;
		}

		m_galaxy_load_state.store(LOAD_STATE_LOADING, std::memory_order_release);
		emit_signal(k_signals->load_state_changed, LOAD_STATE_LOADING);
		emit_signal(k_signals->simulation_started);

		if (thread_mode == THREAD_MODE_MULTI_THREADED)
		{
			m_thread = std::thread(&UniverseSimulation::ThreadLoop, this);
		}
	}

	void UniverseSimulation::StopSimulation()
	{
		LoadState load_state = m_galaxy_load_state.load(std::memory_order_acquire);

		if (load_state == LOAD_STATE_UNLOADED)
		{
			DEBUG_PRINT_ERROR("This galaxy shouldn't be unloaded if we want to start unloading");
			return;
		}

		if (load_state == LOAD_STATE_UNLOADING) // We are already unloading
		{
			return;
		}

		m_galaxy_load_state.store(LOAD_STATE_UNLOADING, std::memory_order_release);
		emit_signal(k_signals->load_state_changed, LOAD_STATE_UNLOADING);

		// TODO : Implement unloading
		m_galaxy_load_state.store(LOAD_STATE_UNLOADED, std::memory_order_release);
		emit_signal(k_signals->load_state_changed, LOAD_STATE_UNLOADED);
	}

	bool UniverseSimulation::Progress(real_t delta)
	{
		if (IsThreaded())
		{
			// If we we are threaded then get the latest info cache data
			{
				std::lock_guard lock(m_cache_mutex);
				m_info_updater.RetrieveUpdates(m_info_cache);
			}

			return true;
		}
		else
		{
			bool keep_running = m_world.progress(static_cast<ecs_ftime_t>(delta));

			// Process signals
			m_deferred_signals.ProcessCommands(get_instance_id());

			return keep_running;
		}
	}

	void UniverseSimulation::ThreadLoop()
	{
		DEBUG_ASSERT(IsThreaded(), "We should be threaded when running the thread loop");

		m_world.set_target_fps(20);

		while (m_galaxy_load_state.load(std::memory_order_acquire) != LOAD_STATE_UNLOADED)
		{
			CommandBuffer command_buffer;
			{
				std::lock_guard lock(m_commands_mutex);
				command_buffer = std::move(m_deferred_commands);
			}

			// Process the deferred commands sent by other threads
			command_buffer.ProcessCommands(get_instance_id());

			m_world.progress();

			// Publish updates to the info caches to be read on the main thread
			m_info_updater.PublishUpdates();

			// Flush signals to be executed on main thread
			CommandQueueServer::get_singleton()->AddCommands(get_instance_id(), std::move(m_deferred_signals));
		}
	}

	void UniverseSimulation::BindMethods()
	{
		BIND_METHOD(godot::D_METHOD(k_commands->is_threaded), &UniverseSimulation::IsThreaded);
		BIND_METHOD(godot::D_METHOD(k_commands->get_universe), &UniverseSimulation::GetUniverse);
		BIND_METHOD(godot::D_METHOD(k_commands->get_galaxy_info), &UniverseSimulation::GetGalaxyInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->start_renderer, "render_info"), &UniverseSimulation::StartRenderer);
		BIND_METHOD(godot::D_METHOD(k_commands->start_simulation, "thread_mode"), &UniverseSimulation::StartSimulation);
		BIND_METHOD(godot::D_METHOD(k_commands->stop_simulation), &UniverseSimulation::StopSimulation);
		BIND_METHOD(godot::D_METHOD(k_commands->progress, "delta"), &UniverseSimulation::Progress);

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

		BIND_METHOD(godot::D_METHOD(k_commands->set_move_forwards, "is_moving"), &UniverseSimulation::SetMoveForwards);
		BIND_METHOD(godot::D_METHOD(k_commands->set_move_backwards, "is_moving"), &UniverseSimulation::SetMoveBackwards);
		BIND_METHOD(godot::D_METHOD(k_commands->set_move_left, "is_moving"), &UniverseSimulation::SetMoveLeft);
		BIND_METHOD(godot::D_METHOD(k_commands->set_move_right, "is_moving"), &UniverseSimulation::SetMoveRight);
		BIND_METHOD(godot::D_METHOD(k_commands->set_move_up, "is_moving"), &UniverseSimulation::SetMoveUp);
		BIND_METHOD(godot::D_METHOD(k_commands->set_move_down, "is_moving"), &UniverseSimulation::SetMoveDown);
		BIND_METHOD(godot::D_METHOD(k_commands->set_rotate_left, "is_rotating"), &UniverseSimulation::SetRotateLeft);
		BIND_METHOD(godot::D_METHOD(k_commands->set_rotate_right, "is_rotating"), &UniverseSimulation::SetRotateRight);
		BIND_METHOD(godot::D_METHOD(k_commands->set_look_direction, "direction"), &UniverseSimulation::SetLookDirection);
		BIND_METHOD(godot::D_METHOD(k_commands->set_sprint, "is_sprinting"), &UniverseSimulation::SetSprint);
		BIND_METHOD(godot::D_METHOD(k_commands->set_crouching, "is_crouching"), &UniverseSimulation::SetCrouching);
		BIND_METHOD(godot::D_METHOD(k_commands->set_prone, "is_prone"), &UniverseSimulation::SetProne);
		BIND_METHOD(godot::D_METHOD(k_commands->jump, "power"), &UniverseSimulation::Jump);

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

		BIND_METHOD(godot::D_METHOD(k_commands->accelerate, "is_accelerating"), &UniverseSimulation::Accelerate);
		BIND_METHOD(godot::D_METHOD(k_commands->decelerate, "is_decelerating"), &UniverseSimulation::Deccelerate);
		BIND_METHOD(godot::D_METHOD(k_commands->activate_vehicle_control, "system_id"), &UniverseSimulation::ActivateVehicleControl);
		BIND_METHOD(godot::D_METHOD(k_commands->toggle_vehicle_control, "system_id", "toggled"), &UniverseSimulation::ToggleVehicleControl);
		BIND_METHOD(godot::D_METHOD(k_commands->set_vehicle_setting, "system_id", "value"), &UniverseSimulation::SetVehicleSetting);

		// ####### Abilities #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_ability_info, "ability_id"), &UniverseSimulation::GetAbilityInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->activate_ability, "ability_id"), &UniverseSimulation::ActivateAbility);
		BIND_METHOD(godot::D_METHOD(k_commands->toggle_ability, "ability_id", "toggled"), &UniverseSimulation::ToggleAbility);
		BIND_METHOD(godot::D_METHOD(k_commands->set_player_setting, "setting_id", "value"), &UniverseSimulation::SetPlayerSetting);

		// ####### Magic #######

		BIND_METHOD(godot::D_METHOD(k_commands->get_spell_info, "spell_id"), &UniverseSimulation::GetSpellInfo);
		BIND_METHOD(godot::D_METHOD(k_commands->use_spell, "spell_index", "params"), &UniverseSimulation::UseSpell);
	}

	void UniverseSimulation::BindEnums()
	{
		BIND_ENUM_CONSTANT(SERVER_TYPE_LOCAL);
		BIND_ENUM_CONSTANT(SERVER_TYPE_REMOTE);

		BIND_ENUM_CONSTANT(THREAD_MODE_SINGLE_THREADED);
		BIND_ENUM_CONSTANT(THREAD_MODE_MULTI_THREADED);

		BIND_ENUM_CONSTANT(LOAD_STATE_LOADING);
		BIND_ENUM_CONSTANT(LOAD_STATE_LOADED);
		BIND_ENUM_CONSTANT(LOAD_STATE_UNLOADING);
		BIND_ENUM_CONSTANT(LOAD_STATE_UNLOADED);
	}

	void UniverseSimulation::BindSignals()
	{
		ADD_SIGNAL(godot::MethodInfo(k_signals->simulation_uninitialized));
		ADD_SIGNAL(godot::MethodInfo(k_signals->simulation_started));
		ADD_SIGNAL(godot::MethodInfo(k_signals->simulation_stopped));
		ADD_SIGNAL(godot::MethodInfo(k_signals->connected_to_remote));
		ADD_SIGNAL(godot::MethodInfo(k_signals->disonnected_from_remote));
		ADD_SIGNAL(godot::MethodInfo(k_signals->load_state_changed, ENUM_PROPERTY("state", UniverseSimulation::LoadState)));

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

	void UniverseSimulation::_bind_methods()
	{
		k_emit_signal = godot::StringName("emit_signal", true);
		k_commands.emplace();
		k_signals.emplace();

		BindEnums();
		BindMethods();
		BindSignals();
	}

	void UniverseSimulation::_cleanup_methods()
	{
		k_commands.reset();
		k_signals.reset();
	}
}