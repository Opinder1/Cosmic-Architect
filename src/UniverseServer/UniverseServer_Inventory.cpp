#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetInventoryInfo(const ID& inventory_id)
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

	ID UniverseServer::GetInventory()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("inventory");
	}

	ID UniverseServer::GetInventoryItemEntity(const ID& inventory_id, uint64_t item_index)
	{
		godot::Dictionary inventory = GetInventoryInfo(inventory_id);

		if (inventory.is_empty())
		{
			return ID{};
		}

		godot::Array items = inventory.find_key("items");

		if (items.size() <= item_index)
		{
			return ID{};
		}

		return items[item_index];
	}

	void UniverseServer::TrashInventoryItem(const ID& inventory_id, uint64_t item_index)
	{
		if (DeferCommand<&UniverseServer::TrashInventoryItem>(inventory_id, item_index))
		{
			return;
		}
	}

	void UniverseServer::MoveInventoryItem(const ID& inventory_id, uint64_t from_item_index, uint64_t to_item_index)
	{
		if (DeferCommand<&UniverseServer::MoveInventoryItem>(inventory_id, from_item_index, to_item_index))
		{
			return;
		}
	}

	void UniverseServer::TransferInventoryItem(const ID& from_inventory_id, uint64_t from_item_index, const ID& to_inventory_id, uint64_t to_item_index)
	{
		if (DeferCommand<&UniverseServer::TransferInventoryItem>(from_inventory_id, from_item_index, to_inventory_id, to_item_index))
		{
			return;
		}
	}

	void UniverseServer::InteractWithInventoryItem(const ID& inventory_id, uint64_t item_index, const godot::Dictionary& interaction_info)
	{
		if (DeferCommand<&UniverseServer::InteractWithInventoryItem>(inventory_id, item_index, interaction_info))
		{
			return;
		}
	}
}