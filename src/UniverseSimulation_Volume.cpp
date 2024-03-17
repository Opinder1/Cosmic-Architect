#include "UniverseSimulation.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetVolumeInfo(UUID volume_id)
	{
		return {};
	}

	godot::Dictionary UniverseSimulation::GetBlockInfo(UUID volume_id, const godot::Vector4i& position)
	{
		return {};
	}

	void UniverseSimulation::PlaceBlock(UUID volume_id, const godot::Vector4i& position, UUID block_id, const godot::Dictionary& block_data)
	{

	}

	void UniverseSimulation::FillBlocks(UUID volume_id, const godot::Vector4i& position_first, const godot::Vector4i& position_second, UUID block_id, uint32_t block_data)
	{

	}

	void UniverseSimulation::PlaceBlockInNewVolume(const godot::Vector4& fragment_position, UUID block_id, const godot::Dictionary& block_data)
	{

	}

	void UniverseSimulation::InteractBlock(UUID volume_id, const godot::Vector4i& position, const godot::Dictionary& interaction)
	{

	}

	godot::Vector4i UniverseSimulation::GetEntityPositionInVolume(UUID volume_id, UUID entity_id)
	{
		return {};
	}

	godot::Vector4i UniverseSimulation::FragmentPositionToVolumePosition(UUID volume_id, const godot::Vector4& fragment_position)
	{
		return {};
	}

	godot::Vector4 UniverseSimulation::VolumePositionToFragmentPosition(UUID volume_id, const godot::Vector4i& volume_position)
	{
		return {};
	}
}