#pragma once

#include "Util/Util.h"
#include "Util/GodotMemory.h"
#include "Util/Span.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/rid.hpp>

#include <TKRZW/tkrzw_thread_util.h>

#include <robin_hood/robin_hood.h>

#include <optional>

namespace voxel_game::rendering
{
	enum class RIDType : uint32_t
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

	constexpr const size_t k_num_alloc_types = to_underlying(RIDType::Count);

	// A server that keeps some pools of render objects preallocated for any allocators to take
	// Any taken render objects will automatically be refilled
	class AllocatorServer : public godot::Object
	{
		GDCLASS(AllocatorServer, godot::Object); // Is an object to be used with call_on_render_thread

	private:
		struct TypeData
		{
			std::vector<godot::RID> rids;
		};

	public:
		static AllocatorServer* get_singleton();

		AllocatorServer();
		~AllocatorServer();

		void RequestRIDs(bool sync);

		void GetRIDs(Span<uint16_t, k_num_alloc_types> requested, Span<std::vector<godot::RID>, k_num_alloc_types> rids_out);

		void FreeRIDs(Span<std::vector<godot::RID>, k_num_alloc_types> rids_in);

	public:
		static void _bind_methods();
		static void _cleanup_methods();

	private:
		bool AnyRequired();
		void AllocateRIDsInternal();
		void DeallocateRIDsInternal();

	private:
		static godot::OptObj<AllocatorServer> k_singleton;

		std::atomic_size_t m_requests = 0;

		tkrzw::SpinMutex m_mutex; // Mutex to protect the write lists

		TypeData m_types[k_num_alloc_types];
	};

	// An allocator that will get instances from the server so instances can be created without
	// any locking of mutexes or even atomics
	class Allocator
	{
	public:
		Allocator();
		~Allocator();

		void Process();

		godot::RID GetRID(RIDType type);

		godot::RID EnsureRID(RIDType type);

	private:
		std::vector<godot::RID> m_rids[k_num_alloc_types];
	};
}