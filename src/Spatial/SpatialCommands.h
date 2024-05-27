#pragma once

#include "SpatialAABB.h"

#include <godot_cpp/variant/vector3i.hpp>

#include <flecs/flecs.h>

#include <memory>
#include <vector>

// Macro to declare what variables are being accessed in parallel
#define PARALLEL_ACCESS(...)

namespace voxel_game
{
	struct SpatialWorld3DComponent;
	struct SpatialScale3D;
	struct SpatialNode3D;

	struct SpatialNodeBuilderBase
	{
		std::unique_ptr<SpatialNode3D>(*node_create)();
		void(*node_destroy)(std::unique_ptr<SpatialNode3D>&);
	};

	struct SpatialCommandProcessorBase
	{
		size_t state_size;
		void(*state_initialize)(void*, flecs::entity, SpatialWorld3DComponent&);
		void(*state_destroy)(void*);
	};

	struct SpatialNodeCommandProcessorBase : SpatialCommandProcessorBase
	{
		void(*process)(void*, SpatialScale3D&, SpatialNode3D&);

		bool operator==(const SpatialNodeCommandProcessorBase& other) { return process == other.process; }
	};

	struct SpatialRegionCommandProcessorBase : SpatialCommandProcessorBase
	{
		void(*process)(void*, SpatialAABB);

		bool operator==(const SpatialRegionCommandProcessorBase& other) { return process == other.process; }
	};

	struct SpatialScaleCommandProcessorBase : SpatialCommandProcessorBase
	{
		void(*process)(void*, size_t, SpatialScale3D&);

		bool operator==(const SpatialScaleCommandProcessorBase& other) { return process == other.process; }
	};

	template<class NodeT>
	struct SpatialNodeBuilder : SpatialNodeBuilderBase
	{
		SpatialNodeBuilder()
		{
			node_create = &NodeCreate;
			node_destroy = &NodeDestroy;
		}

		static std::unique_ptr<SpatialNode3D> NodeCreate()
		{
			return std::make_unique<NodeT>();
		}

		static void NodeDestroy(std::unique_ptr<SpatialNode3D>& node)
		{
			reinterpret_cast<std::unique_ptr<NodeT>&>(node).reset();
		}
	};

	template<class StateT>
	struct SpatialNodeCommandProcessor : SpatialNodeCommandProcessorBase
	{
		SpatialNodeCommandProcessor()
		{
			state_size = sizeof(StateT);
			state_initialize = &StateInitializeProc;
			state_destroy = &StateDestroyProc;
			process = &ProcessProc;
		}

	private:
		static void StateInitializeProc(void* state_ptr, flecs::entity entity, SpatialWorld3DComponent& spatial_world)
		{
			new (state_ptr) StateT(entity, spatial_world);
		}

		static void StateDestroyProc(void* state_ptr)
		{
			std::destroy_at(static_cast<StateT*>(state_ptr));
		}

		static void ProcessProc(void* state_ptr, SpatialScale3D& spatial_scale, SpatialNode3D& spatial_node)
		{
			static_cast<StateT*>(state_ptr)->Process(spatial_scale, spatial_node);
		}
	};

	template<class StateT>
	struct SpatialRegionCommandProcessor : SpatialRegionCommandProcessorBase
	{
		SpatialRegionCommandProcessor()
		{
			state_size = sizeof(StateT);
			state_initialize = &StateInitializeProc;
			state_destroy = &StateDestroyProc;
			process = &ProcessProc;
		}

	private:
		static void StateInitializeProc(void* state_ptr, flecs::entity entity, SpatialWorld3DComponent& spatial_world)
		{
			new (state_ptr) StateT(entity, spatial_world);
		}

		static void StateDestroyProc(void* state_ptr)
		{
			std::destroy_at(static_cast<StateT*>(state_ptr));
		}

		static void ProcessProc(void* state_ptr, SpatialAABB region)
		{
			static_cast<StateT*>(state_ptr)->Process(region);
		}
	};

	template<class StateT>
	struct SpatialScaleCommandProcessor : SpatialScaleCommandProcessorBase
	{
		SpatialScaleCommandProcessor()
		{
			state_size = sizeof(StateT);
			state_initialize = &StateInitializeProc;
			state_destroy = &StateDestroyProc;
			process = &ProcessProc;
		}

	private:
		static void StateInitializeProc(void* state_ptr, flecs::entity entity, SpatialWorld3DComponent& spatial_world)
		{
			new (state_ptr) StateT(entity, spatial_world);
		}

		static void StateDestroyProc(void* state_ptr)
		{
			std::destroy_at(static_cast<StateT*>(state_ptr));
		}

		static void ProcessProc(void* state_ptr, size_t scale_index, SpatialScale3D& scale)
		{
			static_cast<StateT*>(state_ptr)->Process(scale_index, scale);
		}
	};
}