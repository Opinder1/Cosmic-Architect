#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/vector4i.hpp>

#include <robin_hood/robin_hood.h>

#include <flecs/flecs.h>

#include <TKRZW/tkrzw_thread_util.h>

namespace voxel_game
{
	class CommandQueue;
	class Universe;

	class UniverseSimulation : public godot::RefCounted
	{
		GDCLASS(UniverseSimulation, godot::RefCounted);

	public:
		struct Signals;

		using UUID = godot::Color;
		using UUIDVector = godot::PackedColorArray;

		struct UUIDHash
		{
			size_t operator()(const UUID&) const;
		};

		using InfoMap = robin_hood::unordered_flat_map<UUID, godot::Dictionary, UUIDHash>;

		enum LoadState
		{
			LOAD_STATE_LOADING,
			LOAD_STATE_LOADED,
			LOAD_STATE_UNLOADING,
			LOAD_STATE_UNLOADED
		};

		static std::unique_ptr<const Signals> k_signals;

	public:
		UniverseSimulation();
		~UniverseSimulation();

		void Initialize(const godot::Ref<Universe>& universe, const godot::String& path, const godot::String& fragment_type, bool remote);

		godot::Ref<Universe> GetUniverse();
		godot::Dictionary GetGalaxyInfo();
		void StartSimulation();
		void StopSimulation();
		bool Progress(double delta);

		// ####### Fragments (admin only) #######

		godot::Dictionary GetFragmentInfo(UUID fragment_id);
		UUID GetCurrentFragment();
		void EnterFragment(UUID fragment_id, const godot::Dictionary& method); // We will have checked we can do so before hand

		// ####### Account #######

		godot::Dictionary GetAccountInfo();
		void CreateAccount(const godot::String& username, const godot::String& password_hash);
		void AccountLogin(const godot::String& username, const godot::String& password_hash);
		void SavedSessionLogin();
		void ClearSavedSession();
		void DeleteAccount();
		void LogoutAccount();

		// ####### Friends #######

		UUIDVector GetFriends();
		void InviteFriend(UUID account_id);
		void AcceptFreindInvite(UUID account_id);
		void RemoveFriend(UUID account_id);

		// ####### Chat #######

		godot::Dictionary GetChannelInfo(UUID channel_id);
		void SendMessageToChannel(const godot::String& message, UUID channel_id);
		void SendMessageToPlayer(const godot::String& message, UUID account_id);
		godot::Array GetChatChannelHistory(UUID channel_id);
		godot::Array GetPrivateChatHistory(UUID account_id);

		// ####### Players #######

		godot::Dictionary GetPlayerInfo(UUID player_id);

		// ####### Party #######

		godot::Dictionary GetPartyInfo(UUID party_host_id);
		void CreateParty();
		void InviteToParty(UUID player_id);
		void AcceptInvite(UUID player_id);
		void KickFromParty();
		void LeaveParty();
		UUIDVector GetPlayersInParty();
		UUID GetPartyChatChannel();

		// ####### Entity #######

		godot::Dictionary GetEntityInfo(UUID entity_id);
		void RequestEntityInfo(UUID entity_id);

		// ####### Volume (is entity) #######

		godot::Dictionary GetVolumeInfo(UUID volume_id);
		godot::Dictionary GetBlockInfo(UUID volume_id, const godot::Vector4i& position);
		void PlaceBlock(UUID volume_id, const godot::Vector4i& position, UUID block_id, const godot::Dictionary& block_data);
		void FillBlocks(UUID volume_id, const godot::Vector4i& position_first, const godot::Vector4i& position_second, UUID block_id, uint32_t block_data);
		void PlaceBlockInNewVolume(const godot::Vector4& fragment_position, UUID block_id, const godot::Dictionary& block_data);
		void InteractBlock(UUID volume_id, const godot::Vector4i& position, const godot::Dictionary& interaction);
		godot::Vector4i GetEntityPositionInVolume(UUID volume_id, UUID entity_id);
		godot::Vector4i FragmentPositionToVolumePosition(UUID volume_id, const godot::Vector4& fragment_position);
		godot::Vector4 VolumePositionToFragmentPosition(UUID volume_id, const godot::Vector4i& volume_position);

		// ####### Galaxy Region #######

		godot::Dictionary GetGalaxyRegionInfo(UUID galaxy_region_id);
		void RequestGalaxyRegionInfo(UUID galaxy_region_id);
		UUIDVector GetCurrentGalaxyRegions();

		// ####### Galaxy Object (is volume) #######

		godot::Dictionary GetGalaxyObjectInfo(UUID galaxy_object_id);
		void RequestGalaxyObjectInfo(UUID entity_id);

		// ####### Currency #######

		godot::Dictionary GetCurrencyInfo(UUID currency_id);
		godot::Dictionary GetBankInfo(UUID bank_id);
		godot::Dictionary GetBankInterfaceInfo(UUID bank_interface_id);
		godot::Dictionary GetGoodInfo(UUID good_id);
		UUID GetUniversalCurrency();
		UUID GetBankOfInterface(UUID bank_interface_id);
		UUIDVector GetOwnedCurrencies();
		double GetBalance(UUID currency_id);
		void Withdraw(UUID currency_id, double amount, UUID bank_interface_id);
		void Deposit(UUID currency_id, double amount, UUID bank_interface_id);
		void Convert(UUID from_currency_id, UUID to_currency_id, double amount, UUID bank_interface_id);
		void PayEntity(UUID currency_id, UUID entity_id, double amount, UUID bank_interface_id);
		void BuyGoodWithCurrency(UUID good_id, UUID currency_id); // The currency may not be accepted

		// ####### Internet #######

		godot::Dictionary GetInternetInfo(UUID internet_id);
		godot::Dictionary GetWebsiteInfo(UUID website_id);
		godot::Dictionary GetWebsitePageInfo(UUID website_page_id);
		UUIDVector GetInternetWebsites(UUID internet_id);
		UUIDVector GetWebsitePages(UUID website_id);
		void StartInternet(UUID internet_id, UUID device_id);
		void StopInternet();
		UUID GetCurrentInternet();
		UUID GetCurrentInternetSite();
		UUID GetCurrentInternetPage();
		void RequestInternetURL(const godot::String& internet_url);

		// ####### Faction Roles #######

		godot::Dictionary GetRoleInfo(UUID role_id);
		godot::Dictionary GetPermissionInfo(UUID permission_id);
		UUID GetEntityRole(UUID faction_id, UUID entity_id);
		void AddFactionRole(UUID faction_id, UUID role_id, const godot::Dictionary& role_info);
		void RemoveFactionRole(UUID faction_id, UUID role_id);
		void ModifyFactionRole(UUID faction_id, UUID role_id, const godot::Dictionary& role_info);
		void AddPermissionToRole(UUID faction_id, UUID role_id, UUID permission_id); // Only can do this if we have permissions
		void RemovePermissionFromRole(UUID faction_id, UUID role_id, UUID permission_id); // Only can do this if we have permissions
		void SetEntityRole(UUID faction_id, UUID entity_id, UUID role_id); // Only can do this if we have permissions
		bool EntityHasPermission(UUID faction_id, UUID entity_id, UUID permission_id);
		
		// ####### Faction (is entity) #######

		godot::Dictionary GetFactionInfo(UUID faction_id);
		UUIDVector GetJoinedFactions();
		void JoinFaction(UUID faction_id, const godot::Dictionary& request_info);
		void LeaveFaction(UUID faction_id);
		void InviteEntityToFaction(UUID faction_id, UUID entity_id);
		void KickEntityFromFaction(UUID faction_id, UUID entity_id);
		void AddChildFaction(UUID parent_faction_id, UUID child_faction_id);
		void RemoveChildFaction(UUID faction_id);
		void InviteChildFaction(UUID parent_faction_id, UUID child_faction_id);
		void KickChildFaction(UUID faction_id);
		// uint64_t GetNumberOfChildFactions(UUID faction_id);
		// uint64_t GetNumberOfFactionMembers(UUID faction_id);
		// double GetFactionBalance(UUID faction_id, UUID currency_id); // Only if we have access to view the bank account
		// UUID GetFactionInternet(UUID faction_id); // Faction may not have an internet
		// UUID GetFactionChatChannel(UUID faction_id);

		// ####### Player Faction (is faction) #######

		UUID GetGlobalPlayerFaction(); // Is a faction which only players are part of. Player factions are children of this
		UUID GetPlayerFaction();
		void RequestJoinPlayerFaction(UUID faction_id, const godot::String& message);

		// ####### Language #######

		godot::Dictionary GetLanguageInfo(UUID language_id);
		void GetLanguageTranslation(UUID language_id, const godot::StringName& string);
		void GetLanguageString(UUID language_id, uint64_t string_id);

		// ####### Culture #######

		godot::Dictionary GetCultureInfo(UUID culture_id);

		// ####### Religion (is culture) #######
		
		// ####### Buisiness (is culture) #######
		
		// ####### Politics (is culture) #######
		
		// ####### Economics (is culture) #######

		// ####### Civilization (is faction) #######

		// ####### Level #######

		uint16_t GetLevel();
		uint64_t GetExperience();
		void CompleteLevelUp(const godot::Dictionary& levelup_choices);

		// ####### Player Control #######

		void SetMoveForwards(bool is_moving);
		void SetMoveBackwards(bool is_moving);
		void SetMoveLeft(bool is_moving);
		void SetMoveRight(bool is_moving);
		void SetMoveUp(bool is_moving);
		void SetMoveDown(bool is_moving);
		void SetRotateLeft(bool is_rotating);
		void SetRotateRight(bool is_rotating);
		void SetLookDirection(const godot::Quaternion& direction);
		void SetSprint(bool is_sprinting);
		void SetCrouching(bool is_crouching);
		void SetProne(bool is_prone);
		void Jump(double power);

		// ####### Looking at #######

		UUID GetLookingAtEntity();
		UUID GetLookingAtVolume();
		godot::Vector4i GetLookingAtBlock();

		// ####### Inventory #######

		godot::Dictionary GetInventoryInfo(UUID inventory_id);
		UUID GetInventory();
		UUID GetInventoryItemEntity(UUID inventory_id, uint64_t item_index);
		void TrashInventoryItem(UUID inventory_id, uint64_t item_index);
		void MoveInventoryItem(UUID inventory_id, uint64_t from_item_index, uint64_t to_item_index);
		void TransferInventoryItem(UUID from_inventory_id, uint64_t from_item_index, UUID to_inventory_id, uint64_t to_item_index);
		void InteractWithInventoryItem(UUID inventory_id, uint64_t item_index, const godot::Dictionary& interaction_info);

		// ####### Interact #######

		void StoreEntity(UUID entity_id, UUID inventory_id);
		void HoldBlock(UUID volume_id, const godot::Vector4i& position);
		void HoldEntity(UUID entity_id);
		void DropHeldEntity();

		void EquipItemFromWorld(UUID entity_id);
		void EquipItemFromInventory(UUID entity_id, UUID inventory_id, uint64_t item_index);
		void DropEquipToWorld(UUID entity_id);
		void UnequipItemToInventory(UUID entity_id, UUID inventory_id, uint64_t item_index);
		void SetLeftHandEquip(UUID entity_id);
		void SetRightHandEquip(UUID entity_id);
		void UseEquip(UUID entity_id, uint64_t hand);
		void ToggleEquip(UUID entity_id, bool toggled);

		void RideEntity(UUID entity_id, uint64_t attachment_point);
		void ChangeAttachmentPoint(uint64_t new_attachment_point);
		void ExitEntity(UUID entity_id);

		void InteractWithEntity(UUID entity_id, const godot::Dictionary& interaction_info);

		// ####### Vehicle Control #######
		
		void Accelerate(bool is_accelerating);
		void Deccelerate(bool is_decelerating);
		void ActivateVehicleControl(UUID system_id);
		void ToggleVehicleControl(UUID system_id, bool toggled);
		void SetVehicleSetting(UUID setting_id, const godot::Variant& value);

		// ####### Abilities #######

		godot::Dictionary GetAbilityInfo(UUID ability_id);
		void ActivateAbility(UUID ability_id);
		void ToggleAbility(UUID ability_id, bool toggled);
		void SetPlayerSetting(UUID setting_id, const godot::Variant& value);

		// ####### Magic #######

		godot::Dictionary GetSpellInfo(UUID spell_id);
		void UseSpell(uint64_t spell_index, const godot::Dictionary& params);

		// ####### Godhood #######

		// ####### Chaos #######

		// ####### Testing #######

		uint64_t CreateInstance(godot::RID mesh, godot::RID scenario);
		void SetInstancePos(uint64_t instance_id, const godot::Vector3& pos);
		bool DeleteInstance(uint64_t instance_id);

	public:
		static void _bind_methods();
		static void _cleanup_methods();

	private:
		void ThreadFunc();

		template<class... Args>
		void QueueSignal(const godot::StringName& signal, Args... p_args)
		{
			m_emitted_signals->RegisterCommand("emit_signal", signal, p_args);
		}

		static void BindEnums();
		static void BindMethods();
		static void BindSignals();

		static void CleanupSignals();

	private:
		godot::Ref<Universe> m_universe;

		flecs::world m_world;
		flecs::entity_t m_universe_entity = 0;
		flecs::entity_t m_galaxy_entity = 0;

		godot::Ref<CommandQueue> m_commands;
		godot::Ref<CommandQueue> m_emitted_signals;

		std::thread m_thread;
		tkrzw::SpinSharedMutex m_mutex; // Protect state and info caches

		LoadState m_galaxy_load_state = LOAD_STATE_UNLOADED;

		godot::Dictionary m_galaxy_info_cache;
		godot::Dictionary m_account_info_cache;
		godot::Dictionary m_player_info_cache;
		InfoMap m_info_cache;
	};

	struct UniverseSimulation::Signals
	{
		Signals();

		godot::StringName simulation_started;
		godot::StringName simulation_stopped;
		godot::StringName connected_to_remote;
		godot::StringName disonnected_from_remote;
		godot::StringName load_state_changed;

		// ####### Fragments (admin only) #######

		godot::StringName fragment_added;
		godot::StringName fragment_removed;
		godot::StringName fragment_loaded;
		godot::StringName fragment_unloaded;

		// ####### Account #######

		godot::StringName account_create_request_response;
		godot::StringName account_login_request_response;
		godot::StringName account_delete_request_response;
		godot::StringName account_logout_response;

		// ####### Friends #######

		godot::StringName account_invite_friend_response;
		godot::StringName account_friend_request_received;
		godot::StringName account_remove_friend_response;
		godot::StringName account_friend_unfriended;

		// ####### Chat #######

		godot::StringName chat_message_received;
		godot::StringName chat_channel_info_response;

		// ####### Players #######

		godot::StringName player_joined_fragment;
		godot::StringName player_left_fragment;

		// ####### Party #######

		godot::StringName party_create_response;
		godot::StringName party_invited;
		godot::StringName party_invitation_response;
		godot::StringName party_kicked;
		godot::StringName party_player_joined;
		godot::StringName party_player_left;

		// ####### Entity #######

		godot::StringName entity_info_request_response;

		// ####### Volume (is entity) #######

		godot::StringName volume_block_place_response;
		godot::StringName volume_block_fill_response;
		godot::StringName new_volume_block_place_response;
		godot::StringName volume_block_interact_response;

		// ####### Galaxy Region #######

		godot::StringName galaxy_region_info_request_response;
		godot::StringName galaxy_region_entered;
		godot::StringName galaxy_region_exited;

		// ####### Galaxy Object (is volume) #######

		godot::StringName galaxy_object_info_request_response;

		// ####### Currency #######

		godot::StringName currency_withdraw_response;
		godot::StringName currency_deposit_response;
		godot::StringName currency_convert_response;
		godot::StringName currency_pay_response;
		godot::StringName currency_buy_good_response;

		// ####### Internet #######

		godot::StringName internet_start_response;
		godot::StringName internet_stop_response;
		godot::StringName internet_url_request_response;

		// ####### Faction Roles #######

		godot::StringName faction_role_add_response;
		godot::StringName faction_role_remove_response;
		godot::StringName faction_role_modify_response;
		godot::StringName faction_permission_add_response;
		godot::StringName faction_permission_remove_response;
		godot::StringName faction_set_entity_role_response;

		// ####### Faction (is entity) #######

		godot::StringName faction_join_response;
		godot::StringName faction_leave_response;
		godot::StringName faction_entity_invite_response;
		godot::StringName faction_invite_received;
		godot::StringName faction_entity_kick_response;
		godot::StringName faction_child_add_response;
		godot::StringName faction_child_remove_response;
		godot::StringName faction_child_invite_response;
		godot::StringName faction_kick_response;

		// ####### Player Faction (is faction) #######

		godot::StringName faction_request_join_response;

		// ####### Level #######

		godot::StringName levelup_available;
		godot::StringName complete_levelup_response;

		// ####### Looking at #######

		godot::StringName hovered_over_entity;
		godot::StringName hovered_over_volume;
		godot::StringName hovered_over_block;

		// ####### Inventory #######

		godot::StringName inventory_trashed_item_response;
		godot::StringName inventory_moved_item_response;
		godot::StringName inventory_transfer_item_response;
		godot::StringName inventory_interact_item_response;
		godot::StringName inventory_force_closed;

		// ####### Interact #######

		godot::StringName entity_store_response;
		godot::StringName block_hold_response;
		godot::StringName entity_drop_response;

		godot::StringName equip_from_world_response;
		godot::StringName equip_from_inventory_response;
		godot::StringName drop_equip_to_world_response;
		godot::StringName unequip_to_inventory_response;
		godot::StringName set_left_hand_equip_response;
		godot::StringName set_right_hand_equip_response;
		godot::StringName use_equip_response;
		godot::StringName toggle_equip_response;

		godot::StringName ride_entity_response;
		godot::StringName change_ride_attachment_response;
		godot::StringName exit_ride_response;
		godot::StringName ride_force_exited;

		godot::StringName entity_interact_response;

		// ####### Vehicle Control #######

		godot::StringName vehicle_accelerate_response;
		godot::StringName vehicle_deccelerate_response;
		godot::StringName vehicle_control_activate_response;
		godot::StringName vehicle_control_toggle_response;
		godot::StringName vehicle_set_setting_response;

		// ####### Abilities #######

		godot::StringName activate_ability_response;
		godot::StringName toggle_ability_response;
		godot::StringName player_set_setting_response;

		// ####### Magic #######

		godot::StringName use_spell_response_response;

		// ####### Testing #######

		godot::StringName test_signal;
	};
}

VARIANT_ENUM_CAST(voxel_game::UniverseSimulation::LoadState);