#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	void UniverseServer::StoreEntity(const ID& entity_id, const ID& inventory_id)
	{
		if (DeferCommand<&UniverseServer::StoreEntity>(entity_id, inventory_id))
		{
			return;
		}
	}

	void UniverseServer::HoldBlock(const ID& volume_id, const godot::Vector4i& position)
	{
		if (DeferCommand<&UniverseServer::HoldBlock>(volume_id, position))
		{
			return;
		}
	}

	void UniverseServer::HoldEntity(const ID& entity_id)
	{
		if (DeferCommand<&UniverseServer::HoldEntity>(entity_id))
		{
			return;
		}
	}

	void UniverseServer::DropHeldEntity()
	{
		if (DeferCommand<&UniverseServer::DropHeldEntity>())
		{
			return;
		}
	}

	void UniverseServer::EquipItemFromWorld(const ID& entity_id)
	{
		if (DeferCommand<&UniverseServer::EquipItemFromWorld>(entity_id))
		{
			return;
		}
	}

	void UniverseServer::EquipItemFromInventory(const ID& entity_id, const ID& inventory_id, uint64_t item_index)
	{
		if (DeferCommand<&UniverseServer::EquipItemFromInventory>(entity_id, inventory_id, item_index))
		{
			return;
		}
	}

	void UniverseServer::DropEquipToWorld(const ID& entity_id)
	{
		if (DeferCommand<&UniverseServer::DropEquipToWorld>(entity_id))
		{
			return;
		}
	}

	void UniverseServer::UnequipItemToInventory(const ID& entity_id, const ID& inventory_id, uint64_t item_index)
	{
		if (DeferCommand<&UniverseServer::UnequipItemToInventory>(entity_id, inventory_id, item_index))
		{
			return;
		}
	}

	void UniverseServer::SetLeftHandEquip(const ID& entity_id)
	{
		if (DeferCommand<&UniverseServer::SetLeftHandEquip>(entity_id))
		{
			return;
		}
	}

	void UniverseServer::SetRightHandEquip(const ID& entity_id)
	{
		if (DeferCommand<&UniverseServer::SetRightHandEquip>(entity_id))
		{
			return;
		}
	}

	void UniverseServer::UseEquip(const ID& entity_id, uint64_t hand)
	{
		if (DeferCommand<&UniverseServer::UseEquip>(entity_id, hand))
		{
			return;
		}
	}

	void UniverseServer::ToggleEquip(const ID& entity_id, bool toggled)
	{
		if (DeferCommand<&UniverseServer::ToggleEquip>(entity_id, toggled))
		{
			return;
		}
	}

	void UniverseServer::RideEntity(const ID& entity_id, uint64_t attachment_point)
	{
		if (DeferCommand<&UniverseServer::RideEntity>(entity_id, attachment_point))
		{
			return;
		}
	}

	void UniverseServer::ChangeAttachmentPoint(uint64_t new_attachment_point)
	{
		if (DeferCommand<&UniverseServer::ChangeAttachmentPoint>(new_attachment_point))
		{
			return;
		}
	}

	void UniverseServer::ExitEntity(const ID& entity_id)
	{
		if (DeferCommand<&UniverseServer::ExitEntity>(entity_id))
		{
			return;
		}
	}

	void UniverseServer::InteractWithEntity(const ID& entity_id, const godot::Dictionary& interaction_info)
	{
		if (DeferCommand<&UniverseServer::InteractWithEntity>(entity_id, interaction_info))
		{
			return;
		}
	}
}