#include "UniverseSimulation.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetInventoryInfo(UUID inventory_id)
	{
		return {};
	}

	UniverseSimulation::UUID UniverseSimulation::GetInventory()
	{
		return {};
	}

	UniverseSimulation::UUID UniverseSimulation::GetInventoryItemEntity(UUID inventory_id, uint64_t item_index)
	{
		return {};
	}

	void UniverseSimulation::TrashInventoryItem(UUID inventory_id, uint64_t item_index)
	{

	}

	void UniverseSimulation::MoveInventoryItem(UUID inventory_id, uint64_t from_item_index, uint64_t to_item_index)
	{

	}

	void UniverseSimulation::TransferInventoryItem(UUID from_inventory_id, uint64_t from_item_index, UUID to_inventory_id, uint64_t to_item_index)
	{

	}

	void UniverseSimulation::InteractWithInventoryItem(UUID inventory_id, uint64_t item_index, const godot::Dictionary& interaction_info)
	{

	}
}