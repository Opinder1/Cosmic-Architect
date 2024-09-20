#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetInventoryInfo(const UUID& inventory_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		
		auto it = m_info_cache.inventory_info_map.find(inventory_id);

		if (it != m_info_cache.inventory_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	UUID UniverseSimulation::GetInventory()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("inventory");
	}

	UUID UniverseSimulation::GetInventoryItemEntity(const UUID& inventory_id, uint64_t item_index)
	{
		godot::Dictionary inventory = GetInventoryInfo(inventory_id);

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

	void UniverseSimulation::TrashInventoryItem(const UUID& inventory_id, uint64_t item_index)
	{
		if (DeferCommand(k_commands->trash_inventory_item, inventory_id, item_index))
		{
			return;
		}
	}

	void UniverseSimulation::MoveInventoryItem(const UUID& inventory_id, uint64_t from_item_index, uint64_t to_item_index)
	{
		if (DeferCommand(k_commands->move_inventory_item, inventory_id, from_item_index, to_item_index))
		{
			return;
		}
	}

	void UniverseSimulation::TransferInventoryItem(const UUID& from_inventory_id, uint64_t from_item_index, const UUID& to_inventory_id, uint64_t to_item_index)
	{
		if (DeferCommand(k_commands->transfer_inventory_item, from_inventory_id, from_item_index, to_inventory_id, to_item_index))
		{
			return;
		}
	}

	void UniverseSimulation::InteractWithInventoryItem(const UUID& inventory_id, uint64_t item_index, const godot::Dictionary& interaction_info)
	{
		if (DeferCommand(k_commands->interact_with_inventory_item, inventory_id, item_index, interaction_info))
		{
			return;
		}
	}
}