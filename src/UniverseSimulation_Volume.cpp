#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetVolumeInfo(UUID volume_id)
	{
		std::shared_lock lock(m_cache_mutex);
		
		auto it = m_read_cache.volume_info_map.find(volume_id);

		if (it != m_read_cache.volume_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	godot::Dictionary UniverseSimulation::GetBlockInfo(UUID volume_id, const godot::Vector4i& position)
	{
		std::shared_lock lock(m_cache_mutex);
		
		godot::Dictionary volume = GetVolumeInfo(volume_id);

		if (volume.is_empty())
		{
			return godot::Dictionary{};
		}

		godot::Dictionary blocks = volume.find_key("cached_blocks");

		if (blocks.is_empty())
		{
			return godot::Dictionary{};
		}

		return blocks.find_key(position);
	}

	void UniverseSimulation::PlaceBlock(UUID volume_id, const godot::Vector4i& position, UUID block_id, const godot::Dictionary& block_data)
	{
		SIM_DEFER_COMMAND(k_commands->place_block, volume_id, position, block_id, block_data);
	}

	void UniverseSimulation::FillBlocks(UUID volume_id, const godot::Vector4i& position_first, const godot::Vector4i& position_second, UUID block_id, uint32_t block_data)
	{
		SIM_DEFER_COMMAND(k_commands->fill_blocks, volume_id, position_first, position_second, block_id, block_data);
	}

	void UniverseSimulation::PlaceBlockInNewVolume(const godot::Vector4& fragment_position, UUID block_id, const godot::Dictionary& block_data)
	{
		SIM_DEFER_COMMAND(k_commands->place_block_in_new_volume, fragment_position, block_id, block_data);
	}

	void UniverseSimulation::InteractBlock(UUID volume_id, const godot::Vector4i& position, const godot::Dictionary& interaction)
	{
		SIM_DEFER_COMMAND(k_commands->interact_block, volume_id, position, interaction);
	}

	godot::Vector4i UniverseSimulation::GetEntityPositionInVolume(UUID volume_id, UUID entity_id)
	{
		std::shared_lock lock(m_cache_mutex);

		godot::Dictionary volume = GetVolumeInfo(volume_id);

		if (volume.is_empty())
		{
			return godot::Vector4i{};
		}

		godot::Dictionary entity_positions = volume.find_key("cached_entity_positions");

		if (entity_positions.is_empty())
		{
			return godot::Vector4i{};
		}

		return entity_positions.find_key(entity_id);
	}

	godot::Vector4i UniverseSimulation::FragmentPositionToVolumePosition(UUID volume_id, const godot::Vector4& fragment_position)
	{
		std::shared_lock lock(m_cache_mutex);

		godot::Dictionary volume = GetVolumeInfo(volume_id);

		if (volume.is_empty())
		{
			return godot::Vector4i{};
		}

		godot::Transform3D volume_transform = volume.find_key("transform");

		godot::Vector3 spatial_position(fragment_position.x, fragment_position.y, fragment_position.z);

		godot::Vector3 volume_position = volume_transform.xform_inv(spatial_position);

		return godot::Vector4i(volume_position.x, volume_position.y, volume_position.z, fragment_position.w);
	}

	godot::Vector4 UniverseSimulation::VolumePositionToFragmentPosition(UUID volume_id, const godot::Vector4i& volume_position)
	{
		std::shared_lock lock(m_cache_mutex);

		godot::Dictionary volume = GetVolumeInfo(volume_id);

		if (volume.is_empty())
		{
			return godot::Vector4i{};
		}

		godot::Transform3D volume_transform = volume.find_key("transform");

		godot::Vector3 spatial_position(volume_position.x, volume_position.y, volume_position.z);

		godot::Vector3 fragment_position = volume_transform.xform(spatial_position);

		return godot::Vector4i(fragment_position.x, fragment_position.y, fragment_position.z, volume_position.w);
	}
}