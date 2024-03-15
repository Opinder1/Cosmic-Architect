#pragma once

#include "FlecsWorld.h"

#include <robin_hood/robin_hood.h>

namespace voxel_world
{
	class UniverseSimulation : public FlecsWorld
	{
		GDCLASS(UniverseSimulation, FlecsWorld);

	public:
		using UUID = godot::Color;
		using UUIDVector = godot::PackedColorArray;

		struct UUIDHash
		{
			size_t operator()(const UUID&) const;
		};

		enum class LoadState
		{
			Unloaded,
			Loading,
			Loaded,
			Unloading
		};

	public:
		UniverseSimulation();
		~UniverseSimulation();

		// ####### Galaxy #######

		// Start a galaxy locally
		void StartLocalGalaxy(const godot::String& galaxy_path);

		// Start a fragment of a galaxy network locally
		void StartLocalFragment(const godot::String& fragment_path, const godot::String& fragment_type);

		// Attempt to connect to a galaxy network hosted remotely
		// When connected to a remote galaxy we may need to login
		void StartRemoteGalaxy(const godot::String& galaxy_path);

		// Stop the galaxy running
		void StopGalaxy();

		LoadState GetGalaxyLoadState();

		// ####### Account #######

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
		godot::PackedInt64Array GetChatChannels();
		godot::Array GetChatChannelHistory(UUID channel_id);
		godot::Array GetPrivateChatHistory(UUID account_id);

		// ####### Fragments #######

		godot::Dictionary GetFragmentInfo(UUID fragment_id);
		UUID GetCurrentFragment();
		void EnterFragment(UUID fragment_id, const godot::Dictionary& method); // We will have checked we can do so before hand

		// ###### Galaxy #######

		godot::Dictionary GetGalaxyInfo(UUID galaxy_id);

		// ####### Galaxy Region #######

		godot::Dictionary GetGalaxyRegionInfo(UUID galaxy_region_id);

		// ####### Players #######

		godot::Dictionary GetPlayerInfo(UUID player_id);

		// ####### Party #######

		void CreateParty();
		void InviteToParty(UUID player_id);
		void AcceptInvite(UUID player_id);
		void KickFromParty();
		UUIDVector GetPlayersInParty();
		UUID GetPartyChatChannel();

		// ####### Entity #######

		godot::Dictionary GetEntityInfo(UUID entity_id);

		// ####### Inventory #######

		godot::Dictionary GetInventoryInfo(UUID inventory_id);
		UUID GetInventory();
		UUID GetInventoryItemEntity(UUID inventory_id, uint64_t item_index);
		void TrashInventoryItem(UUID inventory_id, uint64_t item_index);
		void MoveInventoryItem(UUID inventory_id, uint64_t from_item_index, uint64_t to_item_index);
		void TransferInventoryItem(UUID from_inventory_id, uint64_t from_item_index, UUID to_inventory_id, uint64_t to_item_index);
		void InteractWithInventoryItem(UUID inventory_id, uint64_t item_index, const godot::Dictionary& interaction_info);

		// ####### Volume (is entity) #######

		godot::Dictionary GetVolumeInfo(UUID volume_id);
		godot::Dictionary GetBlockInfo(UUID volume_id, const godot::Vector4i& position);
		void PlaceBlock(UUID volume_id, const godot::Vector4i& position, UUID block_id, const godot::Dictionary& block_data);
		void FillBlocks(UUID volume_id, const godot::Vector4i& position_first, const godot::Vector4i& position_second, UUID block_id, uint32_t block_data);
		void PlaceBlockInNewVolume(const godot::Vector4& fragment_position, UUID block_id, const godot::Dictionary& block_data);
		godot::Vector4i GetEntityPositionInVolume(UUID volume_id, UUID entity_id);
		godot::Vector4i FragmentPositionToVolumePosition(UUID volume_id, const godot::Vector4& fragment_position);
		godot::Vector4 VolumePositionToFragmentPosition(UUID volume_id, const godot::Vector4i& volume_position);

		// ####### Galaxy Object (is volume) #######

		// ####### Currency #######

		godot::Dictionary GetCurrencyInfo(UUID currency_id);
		godot::Dictionary GetBankInfo(UUID bank_id);
		godot::Dictionary GetBankInterfaceInfo(UUID bank_interface_id);
		UUID GetBankOfInterface(UUID bank_interface_id);
		UUIDVector GetOwnedCurrencies();
		double GetBalance(UUID currency_id);
		void Withdraw(UUID currency_id, double amount, UUID bank_interface_id);
		void Deposit(UUID currency_id, double amount, UUID bank_interface_id);
		void Convert(UUID from_currency_id, UUID to_currency_id, double amount, UUID bank_interface_id);
		void PayEntity(UUID currency_id, UUID entity_id, double amount, UUID bank_interface_id);
		godot::Dictionary GetGoodInfo(UUID good_id); 
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
		bool EntityHasPermission(UUID faction_id, UUID entity_id, UUID permission_id);
		void SetEntityRole(UUID faction_id, UUID entity_id, UUID role_id); // Only can do this if we have permissions
		void AddPermissionToRole(UUID faction_id, UUID role_id, UUID permission_id); // Only can do this if we have permissions
		void RemovePermissionFromRole(UUID faction_id, UUID role_id, UUID permission_id); // Only can do this if we have permissions

		// ####### Faction (is entity) #######

		UUID GetPlayerFaction(); // Is a faction which only players are part of. Player factions are children of this
		godot::Dictionary GetFactionInfo(UUID faction_id);
		uint64_t GetNumberOfChildFactions(UUID faction_id);
		uint64_t GetNumberOfFactionMembers(UUID faction_id);
		double GetFactionBalance(UUID faction_id, UUID currency_id); // Only if we have access to view the bank account
		UUID GetFactionInternet(UUID faction_id); // Faction may not have an internet
		UUID GetFactionChatChannel(UUID faction_id);

		// ####### Civilization (is faction) #######

		uint64_t GetNumberOfChildCivilizations(UUID civilization_id);
		bool CivilizationOwnsPlanet(UUID planet_id);
		bool CivilizationOwnsGalaxyRegion(UUID galaxy_region_id);
		UUID GetPlanetOwner(UUID planet_id);
		UUID GetGalaxyRegionOwner(UUID galaxy_region_id);

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

		// ####### Interact with entity #######

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
		void UseEquip(UUID entity_id);
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

		void ActivateAbility(UUID ability_id);
		void ToggleAbility(UUID ability_id, bool toggled);
		void SetPlayerSetting(UUID setting_id, const godot::Variant& value);

		// ####### Magic #######

		godot::Dictionary GetSpellInfo(UUID spell_id);
		void UseSpell(uint64_t spell_index, const godot::Dictionary& params);

		// ####### Testing #######

		uint64_t CreateInstance(godot::RID mesh, godot::RID scenario);
		void SetInstancePos(uint64_t instance_id, const godot::Vector3& pos);
		bool DeleteInstance(uint64_t instance_id);

	protected:
		static void _bind_methods();

	private:
		godot::String m_path;
		godot::String m_fragment_type;

		godot::Signal m_remote_on_connect;
		godot::Signal m_remote_on_disconnect;

		robin_hood::unordered_flat_map<UUID, godot::Dictionary, UUIDHash> m_info_cache;
	};
}