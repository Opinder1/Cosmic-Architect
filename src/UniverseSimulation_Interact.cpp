#include "UniverseSimulation.h"

namespace voxel_game
{
	void UniverseSimulation::StoreEntity(UUID entity_id, UUID inventory_id)
	{

	}

	void UniverseSimulation::HoldBlock(UUID volume_id, const godot::Vector4i& position)
	{

	}

	void UniverseSimulation::HoldEntity(UUID entity_id)
	{

	}

	void UniverseSimulation::DropHeldEntity()
	{

	}

	void UniverseSimulation::EquipItemFromWorld(UUID entity_id)
	{

	}

	void UniverseSimulation::EquipItemFromInventory(UUID entity_id, UUID inventory_id, uint64_t item_index)
	{

	}

	void UniverseSimulation::DropEquipToWorld(UUID entity_id)
	{

	}

	void UniverseSimulation::UnequipItemToInventory(UUID entity_id, UUID inventory_id, uint64_t item_index)
	{

	}

	void UniverseSimulation::SetLeftHandEquip(UUID entity_id)
	{

	}

	void UniverseSimulation::SetRightHandEquip(UUID entity_id)
	{

	}

	void UniverseSimulation::UseEquip(UUID entity_id, uint64_t hand)
	{

	}

	void UniverseSimulation::ToggleEquip(UUID entity_id, bool toggled)
	{

	}

	void UniverseSimulation::RideEntity(UUID entity_id, uint64_t attachment_point)
	{

	}

	void UniverseSimulation::ChangeAttachmentPoint(uint64_t new_attachment_point)
	{

	}

	void UniverseSimulation::ExitEntity(UUID entity_id)
	{

	}

	void UniverseSimulation::InteractWithEntity(UUID entity_id, const godot::Dictionary& interaction_info)
	{

	}
}