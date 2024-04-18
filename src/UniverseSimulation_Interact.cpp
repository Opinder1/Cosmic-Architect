#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	void UniverseSimulation::StoreEntity(UUID entity_id, UUID inventory_id)
	{
		SIM_DEFER_COMMAND(k_commands->store_entity, entity_id, inventory_id);
	}

	void UniverseSimulation::HoldBlock(UUID volume_id, const godot::Vector4i& position)
	{
		SIM_DEFER_COMMAND(k_commands->hold_block, volume_id, position);
	}

	void UniverseSimulation::HoldEntity(UUID entity_id)
	{
		SIM_DEFER_COMMAND(k_commands->hold_entity, entity_id);
	}

	void UniverseSimulation::DropHeldEntity()
	{
		SIM_DEFER_COMMAND(k_commands->drop_held_entity);
	}

	void UniverseSimulation::EquipItemFromWorld(UUID entity_id)
	{
		SIM_DEFER_COMMAND(k_commands->equip_item_from_world, entity_id);
	}

	void UniverseSimulation::EquipItemFromInventory(UUID entity_id, UUID inventory_id, uint64_t item_index)
	{
		SIM_DEFER_COMMAND(k_commands->equip_item_from_inventory, entity_id, inventory_id, item_index);
	}

	void UniverseSimulation::DropEquipToWorld(UUID entity_id)
	{
		SIM_DEFER_COMMAND(k_commands->drop_equip_to_world, entity_id);
	}

	void UniverseSimulation::UnequipItemToInventory(UUID entity_id, UUID inventory_id, uint64_t item_index)
	{
		SIM_DEFER_COMMAND(k_commands->unequip_item_to_inventory, entity_id, inventory_id, item_index);
	}

	void UniverseSimulation::SetLeftHandEquip(UUID entity_id)
	{
		SIM_DEFER_COMMAND(k_commands->set_left_hand_equip, entity_id);
	}

	void UniverseSimulation::SetRightHandEquip(UUID entity_id)
	{
		SIM_DEFER_COMMAND(k_commands->set_right_hand_equip, entity_id);
	}

	void UniverseSimulation::UseEquip(UUID entity_id, uint64_t hand)
	{
		SIM_DEFER_COMMAND(k_commands->use_equip, entity_id, hand);
	}

	void UniverseSimulation::ToggleEquip(UUID entity_id, bool toggled)
	{
		SIM_DEFER_COMMAND(k_commands->toggle_equip, entity_id, toggled);
	}

	void UniverseSimulation::RideEntity(UUID entity_id, uint64_t attachment_point)
	{
		SIM_DEFER_COMMAND(k_commands->ride_entity, entity_id, attachment_point);
	}

	void UniverseSimulation::ChangeAttachmentPoint(uint64_t new_attachment_point)
	{
		SIM_DEFER_COMMAND(k_commands->change_attachment_point, new_attachment_point);
	}

	void UniverseSimulation::ExitEntity(UUID entity_id)
	{
		SIM_DEFER_COMMAND(k_commands->exit_entity, entity_id);
	}

	void UniverseSimulation::InteractWithEntity(UUID entity_id, const godot::Dictionary& interaction_info)
	{
		SIM_DEFER_COMMAND(k_commands->interact_with_entity, entity_id, interaction_info);
	}
}