#pragma once

#include "UniverseCache.h"

#include "Simulation/SimulationServer.h"

#include "Entity/EntityPoly.h"

#include "Util/PolyFactory.h"
#include "Util/GodotMemory.h"
#include "Util/SmallVector.h"
#include "Util/Debug.h"

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/vector4i.hpp>

#include <optional>
#include <thread>

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game
{
	// Simulation of a section of the universe
	class UniverseServer : public SimulationServer
	{
		GDCLASS(UniverseServer, SimulationServer);

		struct SignalStrings;

	public:
		enum ServerType
		{
			SERVER_TYPE_LOCAL,
			SERVER_TYPE_REMOTE
		};

	public:
		static UniverseServer* get_singleton();

		UniverseServer();
		~UniverseServer();

		bool CanSimulationStart() final;
		void DoSimulationLoad() final;
		void DoSimulationUnload() final;
		void DoSimulationProgress(real_t delta) final;
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
		void StartLocalGalaxy(const godot::String& path, const godot::String& fragment_type, ServerType server_type, godot::RID scenario);
		void ConnectToGalaxy(const godot::String& path, const godot::String& ip, godot::RID scenario);
		void DisconnectFromGalaxy();

		// ####### Account #######

		godot::Dictionary GetAccountInfo();
		void CreateAccount(const godot::String& username, const godot::String& password_hash);
		void AccountLogin(const godot::String& username, const godot::String& password_hash);
		void SavedSessionLogin();
		void ClearSavedSession();
		void DeleteAccount();
		void LogoutAccount();

		// ####### Fragments (admin only) #######

		godot::Dictionary GetFragmentInfo(const ID& fragment_id);
		ID GetCurrentFragment();
		void EnterFragment(const ID& fragment_id, const godot::Dictionary& method); // We will have checked we can do so before hand

		// ####### Galaxy Region #######

		godot::Dictionary GetGalaxyInfo();
		godot::Dictionary GetGalaxyRegionInfo(const ID& galaxy_region_id);
		void RequestGalaxyRegionInfo(const ID& galaxy_region_id);
		IDVector GetCurrentGalaxyRegions();

		// ####### Galaxy Object (is volume) #######

		godot::Dictionary GetGalaxyObjectInfo(const ID& galaxy_object_id);
		void RequestGalaxyObjectInfo(const ID& entity_id);

		// ####### Friends #######

		IDVector GetFriends();
		void InviteFriend(const ID& account_id);
		void AcceptFreindInvite(const ID& account_id);
		void RemoveFriend(const ID& account_id);

		// ####### Chat #######

		godot::Dictionary GetChannelInfo(const ID& channel_id);
		void SendMessageToChannel(const godot::String& message, const ID& channel_id);
		void SendMessageToPlayer(const godot::String& message, const ID& account_id);
		godot::Array GetChatChannelHistory(const ID& channel_id);
		godot::Array GetPrivateChatHistory(const ID& account_id);

		// ####### Players #######

		godot::Dictionary GetPlayerInfo(const ID& player_id);

		// ####### Party #######

		godot::Dictionary GetPartyInfo(const ID& party_host_id);
		void CreateParty();
		void InviteToParty(const ID& player_id);
		void AcceptInvite(const ID& player_id);
		void KickFromParty();
		void LeaveParty();
		IDVector GetPlayersInParty();
		ID GetPartyChatChannel();

		// ####### Entity #######

		godot::Dictionary GetEntityInfo(const ID& entity_id);
		void RequestEntityInfo(const ID& entity_id);

		// ####### Volume (is entity) #######

		godot::Dictionary GetVolumeInfo(const ID& volume_id);
		godot::Dictionary GetBlockInfo(const ID& volume_id, const godot::Vector4i& position);
		void PlaceBlock(const ID& volume_id, const godot::Vector4i& position, const ID& block_id, const godot::Dictionary& block_data);
		void FillBlocks(const ID& volume_id, const godot::Vector4i& position_first, const godot::Vector4i& position_second, const ID& block_id, uint32_t block_data);
		void PlaceBlockInNewVolume(const godot::Vector4& fragment_position, const ID& block_id, const godot::Dictionary& block_data);
		void InteractBlock(const ID& volume_id, const godot::Vector4i& position, const godot::Dictionary& interaction);
		godot::Vector4i GetEntityPositionInVolume(const ID& volume_id, const ID& entity_id);
		godot::Vector4i FragmentPositionToVolumePosition(const ID& volume_id, const godot::Vector4& fragment_position);
		godot::Vector4 VolumePositionToFragmentPosition(const ID& volume_id, const godot::Vector4i& volume_position);

		// ####### Currency #######

		godot::Dictionary GetCurrencyInfo(const ID& currency_id);
		godot::Dictionary GetBankInfo(const ID& bank_id);
		godot::Dictionary GetBankInterfaceInfo(const ID& bank_interface_id);
		godot::Dictionary GetGoodInfo(const ID& good_id);
		ID GetUniversalCurrency();
		ID GetBankOfInterface(const ID& bank_interface_id);
		IDVector GetOwnedCurrencies();
		double GetBalance(const ID& currency_id);
		void Withdraw(const ID& currency_id, real_t amount, const ID& bank_interface_id);
		void Deposit(const ID& currency_id, real_t amount, const ID& bank_interface_id);
		void Convert(const ID& from_currency_id, const ID& to_currency_id, real_t amount, const ID& bank_interface_id);
		void PayEntity(const ID& currency_id, const ID& entity_id, real_t amount, const ID& bank_interface_id);
		void BuyGoodWithCurrency(const ID& good_id, const ID& currency_id); // The currency may not be accepted

		// ####### Internet #######

		godot::Dictionary GetInternetInfo(const ID& internet_id);
		godot::Dictionary GetWebsiteInfo(const ID& website_id);
		godot::Dictionary GetWebsitePageInfo(const ID& website_page_id);
		IDVector GetInternetWebsites(const ID& internet_id);
		IDVector GetWebsitePages(const ID& website_id);
		void StartInternet(const ID& internet_id, const ID& device_id);
		void StopInternet();
		ID GetCurrentInternet();
		ID GetCurrentInternetSite();
		ID GetCurrentInternetPage();
		void RequestInternetURL(const godot::String& internet_url);

		// ####### Faction Roles #######

		godot::Dictionary GetRoleInfo(const ID& role_id);
		godot::Dictionary GetPermissionInfo(const ID& permission_id);
		ID GetEntityRole(const ID& faction_id, const ID& entity_id);
		void AddFactionRole(const ID& faction_id, const ID& role_id, const godot::Dictionary& role_info);
		void RemoveFactionRole(const ID& faction_id, const ID& role_id);
		void ModifyFactionRole(const ID& faction_id, const ID& role_id, const godot::Dictionary& role_info);
		void AddPermissionToRole(const ID& faction_id, const ID& role_id, const ID& permission_id); // Only can do this if we have permissions
		void RemovePermissionFromRole(const ID& faction_id, const ID& role_id, const ID& permission_id); // Only can do this if we have permissions
		void SetEntityRole(const ID& faction_id, const ID& entity_id, const ID& role_id); // Only can do this if we have permissions
		bool EntityHasPermission(const ID& faction_id, const ID& entity_id, const ID& permission_id);

		// ####### Faction (is entity) #######

		godot::Dictionary GetFactionInfo(const ID& faction_id);
		IDVector GetJoinedFactions();
		void JoinFaction(const ID& faction_id, const godot::Dictionary& request_info);
		void LeaveFaction(const ID& faction_id);
		void InviteEntityToFaction(const ID& faction_id, const ID& entity_id);
		void KickEntityFromFaction(const ID& faction_id, const ID& entity_id);
		void AddChildFaction(const ID& parent_faction_id, const ID& child_faction_id);
		void RemoveChildFaction(const ID& faction_id);
		void InviteChildFaction(const ID& parent_faction_id, const ID& child_faction_id);
		void KickChildFaction(const ID& faction_id);

		// ####### Player Faction (is faction) #######

		ID GetGlobalPlayerFaction(); // Is a faction which only players are part of. Player factions are children of this
		ID GetPlayerFaction();
		void RequestJoinPlayerFaction(const ID& faction_id, const godot::String& message);

		// ####### Language #######

		godot::Dictionary GetLanguageInfo(const ID& language_id);
		void GetLanguageTranslation(const ID& language_id, const godot::StringName& string);
		void GetLanguageString(const ID& language_id, uint64_t string_id);

		// ####### Culture #######

		godot::Dictionary GetCultureInfo(const ID& culture_id);

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

		ID GetLookingAtEntity();
		ID GetLookingAtVolume();
		godot::Vector4i GetLookingAtBlock();

		// ####### Inventory #######

		godot::Dictionary GetInventoryInfo(const ID& inventory_id);
		ID GetInventory();
		ID GetInventoryItemEntity(const ID& inventory_id, uint64_t item_index);
		void TrashInventoryItem(const ID& inventory_id, uint64_t item_index);
		void MoveInventoryItem(const ID& inventory_id, uint64_t from_item_index, uint64_t to_item_index);
		void TransferInventoryItem(const ID& from_inventory_id, uint64_t from_item_index, const ID& to_inventory_id, uint64_t to_item_index);
		void InteractWithInventoryItem(const ID& inventory_id, uint64_t item_index, const godot::Dictionary& interaction_info);

		// ####### Interact #######

		void StoreEntity(const ID& entity_id, const ID& inventory_id);
		void HoldBlock(const ID& volume_id, const godot::Vector4i& position);
		void HoldEntity(const ID& entity_id);
		void DropHeldEntity();

		void EquipItemFromWorld(const ID& entity_id);
		void EquipItemFromInventory(const ID& entity_id, const ID& inventory_id, uint64_t item_index);
		void DropEquipToWorld(const ID& entity_id);
		void UnequipItemToInventory(const ID& entity_id, const ID& inventory_id, uint64_t item_index);
		void SetLeftHandEquip(const ID& entity_id);
		void SetRightHandEquip(const ID& entity_id);
		void UseEquip(const ID& entity_id, uint64_t hand);
		void ToggleEquip(const ID& entity_id, bool toggled);

		void RideEntity(const ID& entity_id, uint64_t attachment_point);
		void ChangeAttachmentPoint(uint64_t new_attachment_point);
		void ExitEntity(const ID& entity_id);

		void InteractWithEntity(const ID& entity_id, const godot::Dictionary& interaction_info);

		// ####### Vehicle Control #######

		void TriggerVehicleControl(const ID& control_id);
		void ToggleVehicleControl(const ID& control_id, bool toggled);
		void SetVehicleControl(const ID& control_id, const godot::Variant& value);

		// ####### Abilities #######

		godot::Dictionary GetAbilityInfo(const ID& ability_id);
		void ActivateAbility(const ID& ability_id);
		void ToggleAbility(const ID& ability_id, bool toggled);
		void SetPlayerSetting(const ID& setting_id, const godot::Variant& value);

		// ####### Magic #######

		godot::Dictionary GetSpellInfo(const ID& spell_id);
		void UseSpell(uint64_t spell_index, const godot::Dictionary& params);

		// ####### Godhood #######

		// ####### Chaos #######

	public:
		static void _bind_methods();
		static void _cleanup_methods();

	private:
		static godot::OptObj<UniverseServer> k_singleton;

		// Cached string names for optimization
		static std::optional<const SignalStrings> k_signals;

		static std::optional<Simulation> k_simulation;

		entity::Ref m_player_entity;
		entity::Ref m_dimension_entity;

		entity::Ref m_universe_entity;
		entity::Ref m_galaxy_entity;
		entity::Ref m_starsystem_entity;

		entity::Ref m_world_entity;
		entity::Ref m_spacestation_entity;
		entity::Ref m_spaceship_entity;
		entity::Ref m_vehicle_entity;

		entity::Ref m_avatar_entity;

		// Cached info to be written to by the internal thread and its contents retrieved and read by other threads
		UniverseCacheUpdater m_info_updater; 
		UniverseCache m_info_cache;
	};
}

VARIANT_ENUM_CAST(voxel_game::UniverseServer::ServerType);