#include "UniverseSimulation.h"
#include "Universe.h"

#include "Universe/UniverseComponents.h"
#include "Universe/UniverseModule.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialModule.h"

#include "Voxel/Voxel.h"
#include "Voxel/VoxelModule.h"

#include "Util/Debug.h"
#include "Util/PropertyMacros.h"

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/thread.hpp>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/type_info.hpp>

namespace voxel_game
{
	std::optional<godot::StringName> UniverseSimulation::k_emit_signal;
	std::optional<const UniverseSimulation::CommandStrings> UniverseSimulation::k_commands;
	std::optional<const UniverseSimulation::SignalStrings> UniverseSimulation::k_signals;

	size_t UniverseSimulation::UUIDHash::operator()(const UUID& uuid) const
	{
		static_assert(sizeof(size_t[2]) == sizeof(UUID));

		size_t* arr = (size_t*)&uuid;

		return arr[0] ^ arr[1];
	}

	UniverseSimulation::UniverseSimulation()
	{}

	UniverseSimulation::~UniverseSimulation()
	{
		DEBUG_ASSERT(!IsThreaded(), "The simulation should have been stopped before destroying it");
		m_thread.join();
	}

	bool UniverseSimulation::IsThreaded()
	{
		return m_thread.joinable();
	}

	godot::Ref<Universe> UniverseSimulation::GetUniverse()
	{
		return m_universe;
	}

	godot::Dictionary UniverseSimulation::GetGalaxyInfo()
	{
		return m_cache.Read().galaxy_info;
	}

	void UniverseSimulation::Initialize(const godot::Ref<Universe>& universe, const godot::String& path, const godot::String& fragment_type, ServerType server_type)
	{
		DEBUG_ASSERT(!m_universe.is_valid(), "We can't initialize a simulation twice");

		m_universe = universe;

		m_commands = CommandQueue::MakeQueue();
		m_emitted_signals = CommandQueue::MakeObjectQueue(this);

		m_world.reset();

		m_world.set_target_fps(20);
		m_world.set_threads(godot::OS::get_singleton()->get_processor_count());

		m_world.import<flecs::monitor>();
		m_world.import<UniverseModule>();
		m_world.import<SpatialModule>();

		m_world.add<flecs::Rest>();

		m_universe_entity = m_world.entity().add<UniverseComponent>();

		m_galaxy_entity = m_world.entity()
			.add(flecs::Parent, m_universe_entity)
			.add<GalaxyComponent>()
			.add<UniverseObjectComponent>()
			.set([&](SimulatedGalaxyComponent& simulated_galaxy)
		{
			simulated_galaxy.name = "Test";
			simulated_galaxy.path = path;
			simulated_galaxy.fragment_type = fragment_type;
			simulated_galaxy.is_remote = server_type == SERVER_TYPE_REMOTE;
		});
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
			m_thread = std::thread(&UniverseSimulation::ThreadFunc, this);
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
	}

	void UniverseSimulation::ThreadFunc()
	{
		CommandBuffer command_buffer;

		while (m_galaxy_load_state.load(std::memory_order_acquire) != LOAD_STATE_UNLOADED)
		{
			{
				std::lock_guard lock(m_commands_mutex);
				m_commands->PopCommandBuffer(command_buffer);
			}

			CommandQueue::ProcessCommands(get_instance_id(), command_buffer);

			m_world.progress();

			// TODO Fill caches

			m_cache.EndWrite();

			// Flush signals to be executed on main thread
			m_emitted_signals->Flush();
		}
	}

	bool UniverseSimulation::Progress(real_t delta)
	{
		bool ret;

		if (IsThreaded())
		{
			ret = true;
		}
		else
		{
			ret = m_world.progress(static_cast<ecs_ftime_t>(delta));

			CommandBuffer command_buffer;
			m_emitted_signals->PopCommandBuffer(command_buffer);

			CommandQueue::ProcessCommands(get_instance_id(), command_buffer);

			// TODO Fill caches
		}

		m_cache.StartRead();

		return ret;
	}

	void UniverseSimulation::BindMethods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("get_universe"), &UniverseSimulation::GetUniverse);
		godot::ClassDB::bind_method(godot::D_METHOD("get_galaxy_info"), &UniverseSimulation::GetGalaxyInfo);
		godot::ClassDB::bind_method(godot::D_METHOD("initialize", "universe", "path", "fragment_type", "remote"), &UniverseSimulation::Initialize);
		godot::ClassDB::bind_method(godot::D_METHOD("start_simulation"), &UniverseSimulation::StartSimulation);
		godot::ClassDB::bind_method(godot::D_METHOD("stop_simulation"), &UniverseSimulation::StopSimulation);
		godot::ClassDB::bind_method(godot::D_METHOD("progress", "delta"), &UniverseSimulation::Progress);

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

		// ####### Testing #######

		ADD_SIGNAL(godot::MethodInfo(k_signals->test_signal));
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