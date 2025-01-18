#pragma once

#include "Util/Util.h"
#include "Util/GodotMemory.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/rid.hpp>

#include <TKRZW/tkrzw_thread_util.h>

#include <optional>

namespace voxel_game::rendering
{
	enum class AllocateType : uint32_t
	{
		Texture2D,
		Texture3D,
		Shader,
		Material,
		Mesh,
		MultiMesh,
		Skeleton,
		DirectionalLight,
		OmniLight,
		SpotLight,
		ReflectionProbe,
		Decal,
		VoxelGI,
		Lightmap,
		Particles,
		ParticlesCollision,
		FogVolume,
		VisibilityNotifier,
		Occluder,
		Camera,
		Viewport,
		Sky,
		CompositorEffect,
		Compositor,
		Environment,
		CameraAttributes,
		Scenario,
		Instance,
		Canvas,
		CanvasTexture,
		CanvasItem,
		CanvasLight,
		CanvasLightOccluder,
		CanvasOccluderPolygon,
		Count,
	};

	constexpr const size_t k_num_alloc_types = to_underlying(AllocateType::Count);

	// A server that keeps some pools of render objects preallocated for any allocators to take
	// Any taken render objects will automatically be refilled
	class AllocatorServer : public godot::Object
	{
		GDCLASS(AllocatorServer, godot::Object);

	private:
		struct TypeData
		{
			std::vector<godot::RID> rids;
		};

	public:
		static AllocatorServer* get_singleton();

		AllocatorServer();
		~AllocatorServer();

		void RequestRIDs(AllocateType type, std::vector<godot::RID>& rids_out);

		void AllocateRIDs();

	public:
		static void _bind_methods();
		static void _cleanup_methods();

	private:
		void AllocateRIDsInternal();

	private:
		static godot::OptObj<AllocatorServer> k_singleton;

		tkrzw::SpinMutex m_mutex; // Mutex to protect the write lists

		TypeData m_types[k_num_alloc_types];
	};

	// An allocator that will get instances from the server so instances can be created without
	// any locking of mutexes or even atomics
	class Allocator
	{
	public:
		Allocator(AllocateType type);
		~Allocator();

		void Process();

		godot::RID RequestRID();

	private:
		AllocateType m_type;
		std::vector<godot::RID> m_rids;
	};
}