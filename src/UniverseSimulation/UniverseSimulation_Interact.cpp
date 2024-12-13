#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	void UniverseSimulation::StoreEntity(const UUID& entity_id, const UUID& inventory_id)
	{
		if (DeferCommand<&UniverseSimulation::StoreEntity>(entity_id, inventory_id))
		{
			return;
		}
	}

	void UniverseSimulation::HoldBlock(const UUID& volume_id, const godot::Vector4i& position)
	{
		if (DeferCommand<&UniverseSimulation::HoldBlock>(volume_id, position))
		{
			return;
		}
	}

	void UniverseSimulation::HoldEntity(const UUID& entity_id)
	{
		if (DeferCommand<&UniverseSimulation::HoldEntity>(entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::DropHeldEntity()
	{
		if (DeferCommand<&UniverseSimulation::DropHeldEntity>())
		{
			return;
		}
	}

	void UniverseSimulation::EquipItemFromWorld(const UUID& entity_id)
	{
		if (DeferCommand<&UniverseSimulation::EquipItemFromWorld>(entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::EquipItemFromInventory(const UUID& entity_id, const UUID& inventory_id, uint64_t item_index)
	{
		if (DeferCommand<&UniverseSimulation::EquipItemFromInventory>(entity_id, inventory_id, item_index))
		{
			return;
		}
	}

	void UniverseSimulation::DropEquipToWorld(const UUID& entity_id)
	{
		if (DeferCommand<&UniverseSimulation::DropEquipToWorld>(entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::UnequipItemToInventory(const UUID& entity_id, const UUID& inventory_id, uint64_t item_index)
	{
		if (DeferCommand<&UniverseSimulation::UnequipItemToInventory>(entity_id, inventory_id, item_index))
		{
			return;
		}
	}

	void UniverseSimulation::SetLeftHandEquip(const UUID& entity_id)
	{
		if (DeferCommand<&UniverseSimulation::SetLeftHandEquip>(entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::SetRightHandEquip(const UUID& entity_id)
	{
		if (DeferCommand<&UniverseSimulation::SetRightHandEquip>(entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::UseEquip(const UUID& entity_id, uint64_t hand)
	{
		if (DeferCommand<&UniverseSimulation::UseEquip>(entity_id, hand))
		{
			return;
		}
	}

	void UniverseSimulation::ToggleEquip(const UUID& entity_id, bool toggled)
	{
		if (DeferCommand<&UniverseSimulation::ToggleEquip>(entity_id, toggled))
		{
			return;
		}
	}

	void UniverseSimulation::RideEntity(const UUID& entity_id, uint64_t attachment_point)
	{
		if (DeferCommand<&UniverseSimulation::RideEntity>(entity_id, attachment_point))
		{
			return;
		}
	}

	void UniverseSimulation::ChangeAttachmentPoint(uint64_t new_attachment_point)
	{
		if (DeferCommand<&UniverseSimulation::ChangeAttachmentPoint>(new_attachment_point))
		{
			return;
		}
	}

	void UniverseSimulation::ExitEntity(const UUID& entity_id)
	{
		if (DeferCommand<&UniverseSimulation::ExitEntity>(entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::InteractWithEntity(const UUID& entity_id, const godot::Dictionary& interaction_info)
	{
		if (DeferCommand<&UniverseSimulation::InteractWithEntity>(entity_id, interaction_info))
		{
			return;
		}
	}
}