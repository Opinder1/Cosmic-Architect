#pragma once

#include "Util/Nocopy.h"
#include "Util/UUID.h"
#include "Util/SmallVector.h"
#include "Util/DirtyTracker.h"

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/aabb.hpp>

#include <robin_hood/robin_hood.h>

#include <vector>

namespace voxel_game::rendering
{
	enum class BaseType
	{
		Mesh,
		MultiMesh,
		DirectionalLight,
		OmniLight,
		SpotLight,
		ReflectionProbe,
		Decal,
		VoxelGI,
		Particles,
		FogVolume,
		Occluder,
	};

	enum class InstanceDataFormat
	{
		Position12Bit, // 12 bit position (4 bit int)
		Position24Bit, // 24 bit position (8 bit int)
		Position48Bit, // 48 bit position (16 bit float)
		Position96Bit, // 96 bit position (32 bit float)
		PositionRotation112bit, // 48 bit position (16 bit float) 64 bit rotation (16 bit float)
		PositionRotation224bit, // 96 bit position (32 bit float) 128 bit rotation (32 bit float)
		PositionRotationVelocity160bit, // 48 bit position (16 bit float) 64 bit rotation (16 bit float) 48 bit velocity (16 bit float)
		PositionRotationVelocity320bit, // 96 bit position (32 bit float) 128 bit rotation (32 bit float) 96 bit velocity (32 bit float)
		Transform2D,
		Transform2DColor1,
		Transform2DColor2,
		Transform2DColor3,
		Transform3D,
		Transform3DColor1,
		Transform3DColor2,
		Transform3DColor3,
		Invalid,
	};

	struct InstanceDataType
	{
		// Format for how data is stored for each instance
		InstanceDataFormat format = InstanceDataFormat::Invalid;

		// The region size for each block. If 0 then use just one region
		size_t block_region_size = 0;
	};

	// A block of instances of a mesh type
	struct MeshTypeInstancerBlock : Nocopy
	{
		godot::RID instance_id;
		godot::RID multimesh_id;
		godot::AABB aabb;

		DirtyRangeTracker change_tracker;

		std::vector<size_t> instances;
		std::vector<unsigned char> instance_data;
		std::vector<unsigned char> last_instance_data;
	};

	// A type of mesh instance that is rendered.
	class MeshTypeInstancer : Nocopy
	{
	public:
		MeshTypeInstancer(BaseType type, godot::RID base_id);

		size_t EntityCount();

		void AddEntity(UUID entity);

		void RemoveEntity(UUID entity);

		void SetTransform(UUID entity, godot::Transform3D transform);

		void RebalanceBlocks(godot::Transform3D camera_transform);

	private:
		godot::RID m_base_id;

		BaseType m_type;

		// Info about how the type is stored in the instance data
		InstanceDataType m_data_type;

		size_t m_block_dimensions = 3;

		// Can have one or many render blocks
		GrowingSmallVector<MeshTypeInstancerBlock, 1> m_blocks;
	};

	// Mesh type instancers for meshes that can be made into multimeshes
	class Instancer : Nocopy
	{
	public:
		Instancer();

		void SetScenario(godot::RID scenario);

		void AddType(UUID type_id, BaseType type, godot::RID base_id);

		void RemoveType(UUID type_id);

		void AddEntity(UUID type_id, UUID entity);

		void RemoveEntity(UUID type_id, UUID entity);

		void SetTransform(UUID type_id, UUID entity, godot::Transform3D transform);

		void RebalanceBlocks(godot::Transform3D camera_transform);

		void CleanupEmptyTypes();

	private:
		godot::RID m_scenario;

		godot::Transform3D m_transform;

		robin_hood::unordered_map<UUID, MeshTypeInstancer> m_instancers;
	};
}