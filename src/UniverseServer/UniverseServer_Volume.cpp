#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetVolumeInfo(const UUID& volume_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		
		auto it = m_info_cache.volume_info_map.find(volume_id);

		if (it != m_info_cache.volume_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	godot::Dictionary UniverseServer::GetBlockInfo(const UUID& volume_id, const godot::Vector4i& position)
	{
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

	void UniverseServer::PlaceBlock(const UUID& volume_id, const godot::Vector4i& position, const UUID& block_id, const godot::Dictionary& block_data)
	{
		if (DeferCommand<&UniverseServer::PlaceBlock>(volume_id, position, block_id, block_data))
		{
			return;
		}
	}

	void UniverseServer::FillBlocks(const UUID& volume_id, const godot::Vector4i& position_first, const godot::Vector4i& position_second, const UUID& block_id, uint32_t block_data)
	{
		if (DeferCommand<&UniverseServer::FillBlocks>(volume_id, position_first, position_second, block_id, block_data))
		{
			return;
		}
	}

	void UniverseServer::PlaceBlockInNewVolume(const godot::Vector4& fragment_position, const UUID& block_id, const godot::Dictionary& block_data)
	{
		if (DeferCommand<&UniverseServer::PlaceBlockInNewVolume>(fragment_position, block_id, block_data))
		{
			return;
		}
	}

	void UniverseServer::InteractBlock(const UUID& volume_id, const godot::Vector4i& position, const godot::Dictionary& interaction)
	{
		if (DeferCommand<&UniverseServer::InteractBlock>(volume_id, position, interaction))
		{
			return;
		}
	}

	godot::Vector4i UniverseServer::GetEntityPositionInVolume(const UUID& volume_id, const UUID& entity_id)
	{
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

	godot::Vector4i UniverseServer::FragmentPositionToVolumePosition(const UUID& volume_id, const godot::Vector4& fragment_position)
	{
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

	godot::Vector4 UniverseServer::VolumePositionToFragmentPosition(const UUID& volume_id, const godot::Vector4i& volume_position)
	{
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