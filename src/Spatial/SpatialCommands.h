#pragma once

#include "SpatialAABB.h"

#include "EntityCommandProcessor.h"

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

	struct SpatialBuilderBase
	{
		std::unique_ptr<SpatialScale3D>(*scale_create)();
		void(*scale_destroy)(std::unique_ptr<SpatialScale3D>&);
		std::unique_ptr<SpatialNode3D>(*node_create)();
		void(*node_destroy)(std::unique_ptr<SpatialNode3D>&);
	};

	struct SpatialNodeCommandProcessorBase : EntityCommandProcessorBase
	{
		void(*process)(void*, SpatialWorld3DComponent&, SpatialScale3D&, SpatialNode3D&);

		bool operator==(const SpatialNodeCommandProcessorBase& other) { return process == other.process; }
	};

	struct SpatialRegionCommandProcessorBase : EntityCommandProcessorBase
	{
		void(*process)(void*, SpatialWorld3DComponent&, SpatialAABB);

		bool operator==(const SpatialRegionCommandProcessorBase& other) { return process == other.process; }
	};

	struct SpatialScaleCommandProcessorBase : EntityCommandProcessorBase
	{
		void(*process)(void*, SpatialWorld3DComponent&, size_t, SpatialScale3D&);

		bool operator==(const SpatialScaleCommandProcessorBase& other) { return process == other.process; }
	};

	template<class ScaleT, class NodeT>
	struct SpatialBuilder : SpatialBuilderBase
	{
		static_assert(std::is_base_of_v<SpatialScale3D, ScaleT>);
		static_assert(std::is_base_of_v<SpatialNode3D, NodeT>);

		SpatialBuilder()
		{
			scale_create = &ScaleCreate;
			scale_destroy = &ScaleDestroy;
			node_create = &NodeCreate;
			node_destroy = &NodeDestroy;
		}

		static std::unique_ptr<SpatialScale3D> ScaleCreate()
		{
			return std::make_unique<ScaleT>();
		}

		static void ScaleDestroy(std::unique_ptr<SpatialScale3D>& scale)
		{
			reinterpret_cast<std::unique_ptr<ScaleT>&>(scale).reset();
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

	template<class StateT, class ScaleT, class NodeT>
	struct SpatialNodeCommandProcessor : SpatialNodeCommandProcessorBase
	{
		static_assert(std::is_base_of_v<SpatialScale3D, ScaleT>);
		static_assert(std::is_base_of_v<SpatialNode3D, NodeT>);

		SpatialNodeCommandProcessor()
		{
			state_size = sizeof(StateT);
			state_initialize = &StateInitializeProc;
			state_destroy = &StateDestroyProc;
			process = &ProcessProc;
		}

	private:
		static void StateInitializeProc(void* state_ptr, flecs::entity entity)
		{
			new (state_ptr) StateT(entity);
		}

		static void StateDestroyProc(void* state_ptr)
		{
			std::destroy_at(static_cast<StateT*>(state_ptr));
		}

		static void ProcessProc(void* state_ptr, SpatialWorld3DComponent& spatial_world, SpatialScale3D& spatial_scale, SpatialNode3D& spatial_node)
		{
			static_cast<StateT*>(state_ptr)->Process(spatial_world, static_cast<ScaleT&>(spatial_scale), static_cast<NodeT&>(spatial_node));
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
		static void StateInitializeProc(void* state_ptr, flecs::entity entity)
		{
			new (state_ptr) StateT(entity);
		}

		static void StateDestroyProc(void* state_ptr)
		{
			std::destroy_at(static_cast<StateT*>(state_ptr));
		}

		static void ProcessProc(void* state_ptr, SpatialWorld3DComponent& spatial_world, SpatialAABB region)
		{
			static_cast<StateT*>(state_ptr)->Process(spatial_world, region);
		}
	};

	template<class StateT, class ScaleT>
	struct SpatialScaleCommandProcessor : SpatialScaleCommandProcessorBase
	{
		static_assert(std::is_base_of_v<SpatialScale3D, ScaleT>);

		SpatialScaleCommandProcessor()
		{
			state_size = sizeof(StateT);
			state_initialize = &StateInitializeProc;
			state_destroy = &StateDestroyProc;
			process = &ProcessProc;
		}

	private:
		static void StateInitializeProc(void* state_ptr, flecs::entity entity)
		{
			new (state_ptr) StateT(entity);
		}

		static void StateDestroyProc(void* state_ptr)
		{
			std::destroy_at(static_cast<StateT*>(state_ptr));
		}

		static void ProcessProc(void* state_ptr, SpatialWorld3DComponent& spatial_world, size_t scale_index, SpatialScale3D& scale)
		{
			static_cast<StateT*>(state_ptr)->Process(spatial_world, scale_index, static_cast<ScaleT&>(spatial_scale));
		}
	};
}