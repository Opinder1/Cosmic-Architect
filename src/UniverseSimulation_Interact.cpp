#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	void UniverseSimulation::StoreEntity(UUID entity_id, UUID inventory_id)
	{
		if (DeferCommand(k_commands->store_entity, entity_id, inventory_id))
		{
			return;
		}
	}

	void UniverseSimulation::HoldBlock(UUID volume_id, const godot::Vector4i& position)
	{
		if (DeferCommand(k_commands->hold_block, volume_id, position))
		{
			return;
		}
	}

	void UniverseSimulation::HoldEntity(UUID entity_id)
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

	void UniverseSimulation::EquipItemFromWorld(UUID entity_id)
	{
		if (DeferCommand(k_commands->equip_item_from_world, entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::EquipItemFromInventory(UUID entity_id, UUID inventory_id, uint64_t item_index)
	{
		if (DeferCommand(k_commands->equip_item_from_inventory, entity_id, inventory_id, item_index))
		{
			return;
		}
	}

	void UniverseSimulation::DropEquipToWorld(UUID entity_id)
	{
		if (DeferCommand(k_commands->drop_equip_to_world, entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::UnequipItemToInventory(UUID entity_id, UUID inventory_id, uint64_t item_index)
	{
		if (DeferCommand(k_commands->unequip_item_to_inventory, entity_id, inventory_id, item_index))
		{
			return;
		}
	}

	void UniverseSimulation::SetLeftHandEquip(UUID entity_id)
	{
		if (DeferCommand(k_commands->set_left_hand_equip, entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::SetRightHandEquip(UUID entity_id)
	{
		if (DeferCommand(k_commands->set_right_hand_equip, entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::UseEquip(UUID entity_id, uint64_t hand)
	{
		if (DeferCommand(k_commands->use_equip, entity_id, hand))
		{
			return;
		}
	}

	void UniverseSimulation::ToggleEquip(UUID entity_id, bool toggled)
	{
		if (DeferCommand(k_commands->toggle_equip, entity_id, toggled))
		{
			return;
		}
	}

	void UniverseSimulation::RideEntity(UUID entity_id, uint64_t attachment_point)
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

	void UniverseSimulation::ExitEntity(UUID entity_id)
	{
		if (DeferCommand(k_commands->exit_entity, entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::InteractWithEntity(UUID entity_id, const godot::Dictionary& interaction_info)
	{
		if (DeferCommand(k_commands->interact_with_entity, entity_id, interaction_info))
		{
			return;
		}
	}
}