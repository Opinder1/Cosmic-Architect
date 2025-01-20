#pragma once

#include "UniverseCache.h"

#include "Simulation/SimulationServer.h"

#include "Util/Debug.h"
#include "Util/UUID.h"

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/vector4i.hpp>

#include <flecs/flecs.h>

#include <TKRZW/tkrzw_thread_util.h>

#include <optional>
#include <thread>

namespace voxel_game
{
	// Simulation of a section of the universe
	class UniverseSimulation : public SimulationServer
	{
		GDCLASS(UniverseSimulation, SimulationServer);

		struct CommandStrings;
		struct SignalStrings;

	public:
		enum ServerType
		{
			SERVER_TYPE_LOCAL,
			SERVER_TYPE_REMOTE
		};

		// Cached string names for optimization
		static std::optional<godot::StringName> k_emit_signal;
		static std::optional<const CommandStrings> k_commands;
		static std::optional<const SignalStrings> k_signals;

	public:
		UniverseSimulation();
		~UniverseSimulation();

		void Initialize(const godot::String& path, const godot::String& fragment_type, ServerType server_type, godot::RID scenario);

		bool CanSimulationStart() final;
		void DoSimulationLoad() final;
		void DoSimulationUnload() final;
		bool DoSimulationProgress(real_t delta) final;
		void DoSimulationThreadProgress() final;

#if defined(DEBUG_ENABLED)
		void DebugCommand(const godot::StringName& command, const godot::Array& args);
#endif

		// ####### Universe #######

		godot::Dictionary GetUniverseInfo();
		void ConnectToGalaxyList(const godot::String& ip);
		void DisconnectFromGalaxyList();
		void QueryGalaxyList(const godot::Dictionary& query);
		void PingRemoteGalaxy(const godot::String& ip);

		// ####### Fragments (admin only) #######

		godot::Dictionary GetFragmentInfo(const UUID& fragment_id);
		UUID GetCurrentFragment();
		void EnterFragment(const UUID& fragment_id, const godot::Dictionary& method); // We will have checked we can do so before hand

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
		void InviteFriend(const UUID& account_id);
		void AcceptFreindInvite(const UUID& account_id);
		void RemoveFriend(const UUID& account_id);

		// ####### Chat #######

		godot::Dictionary GetChannelInfo(const UUID& channel_id);
		void SendMessageToChannel(const godot::String& message, const UUID& channel_id);
		void SendMessageToPlayer(const godot::String& message, const UUID& account_id);
		godot::Array GetChatChannelHistory(const UUID& channel_id);
		godot::Array GetPrivateChatHistory(const UUID& account_id);

		// ####### Players #######

		godot::Dictionary GetPlayerInfo(const UUID& player_id);

		// ####### Party #######

		godot::Dictionary GetPartyInfo(const UUID& party_host_id);
		void CreateParty();
		void InviteToParty(const UUID& player_id);
		void AcceptInvite(const UUID& player_id);
		void KickFromParty();
		void LeaveParty();
		UUIDVector GetPlayersInParty();
		UUID GetPartyChatChannel();

		// ####### Entity #######

		godot::Dictionary GetEntityInfo(const UUID& entity_id);
		void RequestEntityInfo(const UUID& entity_id);

		// ####### Volume (is entity) #######

		godot::Dictionary GetVolumeInfo(const UUID& volume_id);
		godot::Dictionary GetBlockInfo(const UUID& volume_id, const godot::Vector4i& position);
		void PlaceBlock(const UUID& volume_id, const godot::Vector4i& position, const UUID& block_id, const godot::Dictionary& block_data);
		void FillBlocks(const UUID& volume_id, const godot::Vector4i& position_first, const godot::Vector4i& position_second, const UUID& block_id, uint32_t block_data);
		void PlaceBlockInNewVolume(const godot::Vector4& fragment_position, const UUID& block_id, const godot::Dictionary& block_data);
		void InteractBlock(const UUID& volume_id, const godot::Vector4i& position, const godot::Dictionary& interaction);
		godot::Vector4i GetEntityPositionInVolume(const UUID& volume_id, const UUID& entity_id);
		godot::Vector4i FragmentPositionToVolumePosition(const UUID& volume_id, const godot::Vector4& fragment_position);
		godot::Vector4 VolumePositionToFragmentPosition(const UUID& volume_id, const godot::Vector4i& volume_position);

		// ####### Galaxy Region #######

		godot::Dictionary GetGalaxyInfo();
		godot::Dictionary GetGalaxyRegionInfo(const UUID& galaxy_region_id);
		void RequestGalaxyRegionInfo(const UUID& galaxy_region_id);
		UUIDVector GetCurrentGalaxyRegions();

		// ####### Galaxy Object (is volume) #######

		godot::Dictionary GetGalaxyObjectInfo(const UUID& galaxy_object_id);
		void RequestGalaxyObjectInfo(const UUID& entity_id);

		// ####### Currency #######

		godot::Dictionary GetCurrencyInfo(const UUID& currency_id);
		godot::Dictionary GetBankInfo(const UUID& bank_id);
		godot::Dictionary GetBankInterfaceInfo(const UUID& bank_interface_id);
		godot::Dictionary GetGoodInfo(const UUID& good_id);
		UUID GetUniversalCurrency();
		UUID GetBankOfInterface(const UUID& bank_interface_id);
		UUIDVector GetOwnedCurrencies();
		double GetBalance(const UUID& currency_id);
		void Withdraw(const UUID& currency_id, real_t amount, const UUID& bank_interface_id);
		void Deposit(const UUID& currency_id, real_t amount, const UUID& bank_interface_id);
		void Convert(const UUID& from_currency_id, const UUID& to_currency_id, real_t amount, const UUID& bank_interface_id);
		void PayEntity(const UUID& currency_id, const UUID& entity_id, real_t amount, const UUID& bank_interface_id);
		void BuyGoodWithCurrency(const UUID& good_id, const UUID& currency_id); // The currency may not be accepted

		// ####### Internet #######

		godot::Dictionary GetInternetInfo(const UUID& internet_id);
		godot::Dictionary GetWebsiteInfo(const UUID& website_id);
		godot::Dictionary GetWebsitePageInfo(const UUID& website_page_id);
		UUIDVector GetInternetWebsites(const UUID& internet_id);
		UUIDVector GetWebsitePages(const UUID& website_id);
		void StartInternet(const UUID& internet_id, const UUID& device_id);
		void StopInternet();
		UUID GetCurrentInternet();
		UUID GetCurrentInternetSite();
		UUID GetCurrentInternetPage();
		void RequestInternetURL(const godot::String& internet_url);

		// ####### Faction Roles #######

		godot::Dictionary GetRoleInfo(const UUID& role_id);
		godot::Dictionary GetPermissionInfo(const UUID& permission_id);
		UUID GetEntityRole(const UUID& faction_id, const UUID& entity_id);
		void AddFactionRole(const UUID& faction_id, const UUID& role_id, const godot::Dictionary& role_info);
		void RemoveFactionRole(const UUID& faction_id, const UUID& role_id);
		void ModifyFactionRole(const UUID& faction_id, const UUID& role_id, const godot::Dictionary& role_info);
		void AddPermissionToRole(const UUID& faction_id, const UUID& role_id, const UUID& permission_id); // Only can do this if we have permissions
		void RemovePermissionFromRole(const UUID& faction_id, const UUID& role_id, const UUID& permission_id); // Only can do this if we have permissions
		void SetEntityRole(const UUID& faction_id, const UUID& entity_id, const UUID& role_id); // Only can do this if we have permissions
		bool EntityHasPermission(const UUID& faction_id, const UUID& entity_id, const UUID& permission_id);

		// ####### Faction (is entity) #######

		godot::Dictionary GetFactionInfo(const UUID& faction_id);
		UUIDVector GetJoinedFactions();
		void JoinFaction(const UUID& faction_id, const godot::Dictionary& request_info);
		void LeaveFaction(const UUID& faction_id);
		void InviteEntityToFaction(const UUID& faction_id, const UUID& entity_id);
		void KickEntityFromFaction(const UUID& faction_id, const UUID& entity_id);
		void AddChildFaction(const UUID& parent_faction_id, const UUID& child_faction_id);
		void RemoveChildFaction(const UUID& faction_id);
		void InviteChildFaction(const UUID& parent_faction_id, const UUID& child_faction_id);
		void KickChildFaction(const UUID& faction_id);

		// ####### Player Faction (is faction) #######

		UUID GetGlobalPlayerFaction(); // Is a faction which only players are part of. Player factions are children of this
		UUID GetPlayerFaction();
		void RequestJoinPlayerFaction(const UUID& faction_id, const godot::String& message);

		// ####### Language #######

		godot::Dictionary GetLanguageInfo(const UUID& language_id);
		void GetLanguageTranslation(const UUID& language_id, const godot::StringName& string);
		void GetLanguageString(const UUID& language_id, uint64_t string_id);

		// ####### Culture #######

		godot::Dictionary GetCultureInfo(const UUID& culture_id);

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

		void SetSprint(bool is_sprinting);
		void SetCrouching(bool is_crouching);
		void SetProne(bool is_prone);
		void DoWalk(const godot::Vector3& velocity);
		void DoLook(const godot::Quaternion& direction);
		void DoJump(double power);

		// ####### Looking at #######

		UUID GetLookingAtEntity();
		UUID GetLookingAtVolume();
		godot::Vector4i GetLookingAtBlock();

		// ####### Inventory #######

		godot::Dictionary GetInventoryInfo(const UUID& inventory_id);
		UUID GetInventory();
		UUID GetInventoryItemEntity(const UUID& inventory_id, uint64_t item_index);
		void TrashInventoryItem(const UUID& inventory_id, uint64_t item_index);
		void MoveInventoryItem(const UUID& inventory_id, uint64_t from_item_index, uint64_t to_item_index);
		void TransferInventoryItem(const UUID& from_inventory_id, uint64_t from_item_index, const UUID& to_inventory_id, uint64_t to_item_index);
		void InteractWithInventoryItem(const UUID& inventory_id, uint64_t item_index, const godot::Dictionary& interaction_info);

		// ####### Interact #######

		void StoreEntity(const UUID& entity_id, const UUID& inventory_id);
		void HoldBlock(const UUID& volume_id, const godot::Vector4i& position);
		void HoldEntity(const UUID& entity_id);
		void DropHeldEntity();

		void EquipItemFromWorld(const UUID& entity_id);
		void EquipItemFromInventory(const UUID& entity_id, const UUID& inventory_id, uint64_t item_index);
		void DropEquipToWorld(const UUID& entity_id);
		void UnequipItemToInventory(const UUID& entity_id, const UUID& inventory_id, uint64_t item_index);
		void SetLeftHandEquip(const UUID& entity_id);
		void SetRightHandEquip(const UUID& entity_id);
		void UseEquip(const UUID& entity_id, uint64_t hand);
		void ToggleEquip(const UUID& entity_id, bool toggled);

		void RideEntity(const UUID& entity_id, uint64_t attachment_point);
		void ChangeAttachmentPoint(uint64_t new_attachment_point);
		void ExitEntity(const UUID& entity_id);

		void InteractWithEntity(const UUID& entity_id, const godot::Dictionary& interaction_info);

		// ####### Vehicle Control #######

		void TriggerVehicleControl(const UUID& control_id);
		void ToggleVehicleControl(const UUID& control_id, bool toggled);
		void SetVehicleControl(const UUID& control_id, const godot::Variant& value);

		// ####### Abilities #######

		godot::Dictionary GetAbilityInfo(const UUID& ability_id);
		void ActivateAbility(const UUID& ability_id);
		void ToggleAbility(const UUID& ability_id, bool toggled);
		void SetPlayerSetting(const UUID& setting_id, const godot::Variant& value);

		// ####### Magic #######

		godot::Dictionary GetSpellInfo(const UUID& spell_id);
		void UseSpell(uint64_t spell_index, const godot::Dictionary& params);

		// ####### Godhood #######

		// ####### Chaos #######

	public:
		static void _bind_methods();
		static void _cleanup_methods();

	private:
		// Universe data. Not thread safe
		godot::StringName m_path;
		godot::StringName m_fragment_type;
		ServerType m_server_type = ServerType::SERVER_TYPE_LOCAL;
		godot::RID m_scenario;
		flecs::world m_world;

		// Quick access entities. Not thread safe
		flecs::entity_t m_universe_entity = 0;
		flecs::entity_t m_galaxy_entity = 0;
		flecs::entity_t m_player_entity = 0;

		// Cached info to be written to by the internal thread and its contents retrieved and read by other threads
		UniverseCacheUpdater m_info_updater; 
		UniverseCache m_info_cache;
	};
}

VARIANT_ENUM_CAST(voxel_game::UniverseSimulation::ServerType);