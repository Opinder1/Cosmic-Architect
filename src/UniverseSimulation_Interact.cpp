#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	void UniverseSimulation::StoreEntity(const UUID& entity_id, const UUID& inventory_id)
	{
		if (DeferCommand(k_commands->store_entity, entity_id, inventory_id))
		{
			return;
		}
	}

	void UniverseSimulation::HoldBlock(const UUID& volume_id, const godot::Vector4i& position)
	{
		if (DeferCommand(k_commands->hold_block, volume_id, position))
		{
			return;
		}
	}

	void UniverseSimulation::HoldEntity(const UUID& entity_id)
	{
		if (DeferCommand(k_commands->hold_entity, entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::DropHeldEntity()
	{
		if (DeferCommand(k_commands->drop_held_entity))
		{
			return;
		}
	}

	void UniverseSimulation::EquipItemFromWorld(const UUID& entity_id)
	{
		if (DeferCommand(k_commands->equip_item_from_world, entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::EquipItemFromInventory(const UUID& entity_id, const UUID& inventory_id, uint64_t item_index)
	{
		if (DeferCommand(k_commands->equip_item_from_inventory, entity_id, inventory_id, item_index))
		{
			return;
		}
	}

	void UniverseSimulation::DropEquipToWorld(const UUID& entity_id)
	{
		if (DeferCommand(k_commands->drop_equip_to_world, entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::UnequipItemToInventory(const UUID& entity_id, const UUID& inventory_id, uint64_t item_index)
	{
		if (DeferCommand(k_commands->unequip_item_to_inventory, entity_id, inventory_id, item_index))
		{
			return;
		}
	}

	void UniverseSimulation::SetLeftHandEquip(const UUID& entity_id)
	{
		if (DeferCommand(k_commands->set_left_hand_equip, entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::SetRightHandEquip(const UUID& entity_id)
	{
		if (DeferCommand(k_commands->set_right_hand_equip, entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::UseEquip(const UUID& entity_id, uint64_t hand)
	{
		if (DeferCommand(k_commands->use_equip, entity_id, hand))
		{
			return;
		}
	}

	void UniverseSimulation::ToggleEquip(const UUID& entity_id, bool toggled)
	{
		if (DeferCommand(k_commands->toggle_equip, entity_id, toggled))
		{
			return;
		}
	}

	void UniverseSimulation::RideEntity(const UUID& entity_id, uint64_t attachment_point)
	{
		if (DeferCommand(k_commands->ride_entity, entity_id, attachment_point))
		{
			return;
		}
	}

	void UniverseSimulation::ChangeAttachmentPoint(uint64_t new_attachment_point)
	{
		if (DeferCommand(k_commands->change_attachment_point, new_attachment_point))
		{
			return;
		}
	}

	void UniverseSimulation::ExitEntity(const UUID& entity_id)
	{
		if (DeferCommand(k_commands->exit_entity, entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::InteractWithEntity(const UUID& entity_id, const godot::Dictionary& interaction_info)
	{
		if (DeferCommand(k_commands->interact_with_entity, entity_id, interaction_info))
		{
			return;
		}
	}
}