#pragma once

#include "CommandQueue.h"

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

#include <optional>
#include <thread>

namespace voxel_game
{
	class CommandQueue;
	class Universe;

	using UUID = godot::Color;
	using UUIDVector = godot::PackedColorArray;

	struct UUIDHash
	{
		size_t operator()(const UUID&) const;
	};

	struct UniverseCache
	{
		using Info = godot::Dictionary;
		using InfoMap = robin_hood::unordered_flat_map<UUID, Info, UUIDHash>;

		enum class Type
		{
			Galaxy,
			Account,
			Player,
			Fragment,
			ChatChannel,
			Party,
			Entity,
			Volume,
			GalaxyRegion,
			GalaxyObject,
			Currency,
			Bank,
			BankInterface,
			Good,
			Internet,
			Website,
			Webpage,
			Role,
			Permission,
			Faction,
			Language,
			Culture,
			Inventory,
			Ability,
			Spell
		};

		Info galaxy_info;
		Info account_info;
		Info player_info;

		InfoMap player_info_map;
		InfoMap fragment_info_map;
		InfoMap chat_channel_info_map;
		InfoMap party_info_map;
		InfoMap entity_info_map;
		InfoMap volume_info_map;
		InfoMap galaxy_region_info_map;
		InfoMap galaxy_object_info_map;
		InfoMap currency_info_map;
		InfoMap bank_info_map;
		InfoMap bank_interface_info_map;
		InfoMap good_info_map;
		InfoMap internet_info_map;
		InfoMap website_info_map;
		InfoMap webpage_info_map;
		InfoMap role_info_map;
		InfoMap permission_info_map;
		InfoMap faction_info_map;
		InfoMap language_info_map;
		InfoMap culture_info_map;
		InfoMap inventory_info_map;
		InfoMap ability_info_map;
		InfoMap spell_info_map;

		static Info UniverseCache::* GetInfo(Type type);

		static InfoMap UniverseCache::* GetInfoMap(Type type);
	};

	class UniverseCacheUpdater
	{
		struct InfoUpdate
		{
			union
			{
				UniverseCache::Info UniverseCache::* info = nullptr;
				UniverseCache::InfoMap UniverseCache::* info_map;
			};
			UUID key;
			UniverseCache::Info value;
		};

	public:
		UniverseCacheUpdater();

		void UpdateInfo(UniverseCache::Type type, const UniverseCache::Info& info);

		void UpdateInfoMap(UniverseCache::Type type, UUID id, const UniverseCache::Info& info);

		// Write the changes to the exchange buffer
		void PublishUpdates();

		// Obtain the latest changes made by the writer if there are any
		void ApplyUpdates(UniverseCache& cache);

	private:
		void AddInfoUpdate(InfoUpdate&& update);

	private:
		std::vector<InfoUpdate> m_write;
		std::vector<InfoUpdate> m_read;

		std::atomic_bool m_ready{ false };
	};

	// Simulation of a section of the universe
	class UniverseSimulation : public godot::RefCounted
	{
		GDCLASS(UniverseSimulation, godot::RefCounted);

		struct CommandStrings;
		struct SignalStrings;

	public:
		enum ServerType
		{
			SERVER_TYPE_LOCAL,
			SERVER_TYPE_REMOTE
		};

		enum ThreadMode
		{
			THREAD_MODE_SINGLE_THREADED,
			THREAD_MODE_MULTI_THREADED
		};

		enum LoadState
		{
			LOAD_STATE_LOADING,
			LOAD_STATE_LOADED,
			LOAD_STATE_UNLOADING,
			LOAD_STATE_UNLOADED
		};

		// Cached string names for optimization
		static std::optional<godot::StringName> k_emit_signal;
		static std::optional<const CommandStrings> k_commands;
		static std::optional<const SignalStrings> k_signals;

	public:
		UniverseSimulation();
		~UniverseSimulation();

		bool IsThreaded();

		godot::Ref<Universe> GetUniverse();
		godot::Dictionary GetGalaxyInfo();
		void Initialize(const godot::Ref<Universe>& universe, const godot::String& path, const godot::String& fragment_type, ServerType server_type);
		void StartSimulation(ThreadMode thread_mode);
		void StopSimulation();
		bool Progress(real_t delta);

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
		void Withdraw(UUID currency_id, real_t amount, UUID bank_interface_id);
		void Deposit(UUID currency_id, real_t amount, UUID bank_interface_id);
		void Convert(UUID from_currency_id, UUID to_currency_id, real_t amount, UUID bank_interface_id);
		void PayEntity(UUID currency_id, UUID entity_id, real_t amount, UUID bank_interface_id);
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
		template<class... Args>
		void QueueSignal(const godot::StringName& signal, const Args&... p_args);

		void ThreadLoop();

		static void BindEnums();
		static void BindMethods();
		static void BindSignals();

	private:
		godot::Ref<Universe> m_universe;

		std::atomic<LoadState> m_galaxy_load_state = LOAD_STATE_UNLOADED;

		flecs::world m_world;
		flecs::entity_t m_universe_entity = 0;
		flecs::entity_t m_galaxy_entity = 0;

		std::thread m_thread;

		// Commands deferred to be processed when the internal thread is ready
		tkrzw::SpinMutex m_commands_mutex;
		CommandBuffer m_deferred_commands;

		// Signals deferred to be emitted when finished progressing
		CommandBuffer m_deferred_signals;

		// Cache buffer to be written to by the internal thread
		UniverseCacheUpdater m_info_updater;

		// Cache buffer to be read from by commands
		tkrzw::SpinSharedMutex m_cache_mutex;
		UniverseCache m_info_cache;
	};

	template<class... Args>
	void UniverseSimulation::QueueSignal(const godot::StringName& signal, const Args&... p_args)
	{
		CommandBuffer::AddCommand(m_deferred_signals, *k_emit_signal, signal, p_args...);
	}

#define SIM_DEFER_COMMAND(command, ...) \
	if (IsThreaded() && std::this_thread::get_id() != m_thread.get_id()) \
	{ \
		std::lock_guard lock(m_commands_mutex); \
		CommandBuffer::AddCommand(m_deferred_commands, command, __VA_ARGS__); \
		return; \
	}
}

VARIANT_ENUM_CAST(voxel_game::UniverseSimulation::ServerType);
VARIANT_ENUM_CAST(voxel_game::UniverseSimulation::ThreadMode);
VARIANT_ENUM_CAST(voxel_game::UniverseSimulation::LoadState);