#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetInventoryInfo(UUID inventory_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(inventory_id);
	}

	UniverseSimulation::UUID UniverseSimulation::GetInventory()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.player_info.find_key("inventory");
	}

	UniverseSimulation::UUID UniverseSimulation::GetInventoryItemEntity(UUID inventory_id, uint64_t item_index)
	{
		std::shared_lock lock(m_cache_mutex);

		godot::Dictionary inventory = GetCacheEntry(inventory_id);

		if (inventory.is_empty())
		{
			return UUID{};
		}

		godot::Array items = inventory.find_key("items");

		if (items.size() <= item_index)
		{
			return UUID{};
		}

		return items[item_index];
	}

	void UniverseSimulation::TrashInventoryItem(UUID inventory_id, uint64_t item_index)
	{
		SIM_DEFER_COMMAND(k_commands->trash_inventory_item, inventory_id, item_index);
	}

	void UniverseSimulation::MoveInventoryItem(UUID inventory_id, uint64_t from_item_index, uint64_t to_item_index)
	{
		SIM_DEFER_COMMAND(k_commands->move_inventory_item, inventory_id, from_item_index, to_item_index);
	}

	void UniverseSimulation::TransferInventoryItem(UUID from_inventory_id, uint64_t from_item_index, UUID to_inventory_id, uint64_t to_item_index)
	{
		SIM_DEFER_COMMAND(k_commands->transfer_inventory_item, from_inventory_id, from_item_index, to_inventory_id, to_item_index);
	}

	void UniverseSimulation::InteractWithInventoryItem(UUID inventory_id, uint64_t item_index, const godot::Dictionary& interaction_info)
	{
		SIM_DEFER_COMMAND(k_commands->interact_with_inventory_item, inventory_id, item_index, interaction_info);
	}
}