#pragma once

#include "SpatialAABB.h"

#include "Util/EntityCommandExecutor.h"

#include <godot_cpp/variant/vector3i.hpp>

#include <flecs/flecs.h>

#include <memory>
#include <vector>

namespace voxel_game::spatial3d
{
	struct World;
	struct Scale;
	struct Node;

	struct BuilderBase
	{
		std::unique_ptr<Scale>(*scale_create)() = nullptr;
		void(*scale_destroy)(std::unique_ptr<Scale>&) = nullptr;
		std::unique_ptr<Node>(*node_create)() = nullptr;
		void(*node_destroy)(std::unique_ptr<Node>&) = nullptr;
	};

	struct NodeCommandProcessorBase : EntityCommandProcessorBase
	{
		void(*process)(void*, World&, Scale&, Node&) = nullptr;

		bool operator==(const NodeCommandProcessorBase& other) { return process == other.process; }
	};

	struct RegionCommandProcessorBase : EntityCommandProcessorBase
	{
		void(*process)(void*, World&, AABB) = nullptr;

		bool operator==(const RegionCommandProcessorBase& other) { return process == other.process; }
	};

	struct ScaleCommandProcessorBase : EntityCommandProcessorBase
	{
		void(*process)(void*, World&, size_t, Scale&) = nullptr;

		bool operator==(const ScaleCommandProcessorBase& other) { return process == other.process; }
	};

	template<class ScaleT, class NodeT>
	struct Builder : BuilderBase
	{
		static_assert(std::is_base_of_v<Scale, ScaleT>);
		static_assert(std::is_base_of_v<Node, NodeT>);

		Builder()
		{
			scale_create = &ScaleCreate;
			scale_destroy = &ScaleDestroy;
			node_create = &NodeCreate;
			node_destroy = &NodeDestroy;
		}

		static std::unique_ptr<Scale> ScaleCreate()
		{
			return std::make_unique<ScaleT>();
		}

		static void ScaleDestroy(std::unique_ptr<Scale>& scale)
		{
			delete static_cast<ScaleT*>(scale.release());
		}

		static std::unique_ptr<Node> NodeCreate()
		{
			return std::make_unique<NodeT>();
		}

		static void NodeDestroy(std::unique_ptr<Node>& node)
		{
			delete static_cast<NodeT*>(node.release());
		}
	};

	template<class StateT, class ScaleT, class NodeT>
	struct NodeCommandProcessor : NodeCommandProcessorBase
	{
		static_assert(std::is_base_of_v<Scale, ScaleT>);
		static_assert(std::is_base_of_v<Node, NodeT>);

		NodeCommandProcessor()
		{
			state_size = sizeof(StateT);
			state_align = alignof(StateT);
			state_initialize = &StateInitializeProc;
			state_destroy = &StateDestroyProc;
			process = &ProcessProc;
		}

	private:
		static void StateInitializeProc(void* state_ptr, flecs::world_t* world, flecs::entity_t spatial_world_entity)
		{
			new (state_ptr) StateT(world, spatial_world_entity);
		}

		static void StateDestroyProc(void* state_ptr)
		{
			std::destroy_at(static_cast<StateT*>(state_ptr));
		}

		static void ProcessProc(void* state_ptr, World& spatial_world, Scale& spatial_scale, Node& spatial_node)
		{
			static_cast<StateT*>(state_ptr)->Process(spatial_world, static_cast<ScaleT&>(spatial_scale), static_cast<NodeT&>(spatial_node));
		}
	};

	template<class StateT>
	struct RegionCommandProcessor : RegionCommandProcessorBase
	{
		RegionCommandProcessor()
		{
			state_size = sizeof(StateT);
			state_align = alignof(StateT);
			state_initialize = &StateInitializeProc;
			state_destroy = &StateDestroyProc;
			process = &ProcessProc;
		}

	private:
		static void StateInitializeProc(void* state_ptr, flecs::world_t* world, flecs::entity_t spatial_world_entity)
		{
			new (state_ptr) StateT(world, spatial_world_entity);
		}

		static void StateDestroyProc(void* state_ptr)
		{
			std::destroy_at(static_cast<StateT*>(state_ptr));
		}

		static void ProcessProc(void* state_ptr, World& spatial_world, AABB region)
		{
			static_cast<StateT*>(state_ptr)->Process(spatial_world, region);
		}
	};

	template<class StateT, class ScaleT>
	struct ScaleCommandProcessor : ScaleCommandProcessorBase
	{
		static_assert(std::is_base_of_v<Scale, ScaleT>);

		ScaleCommandProcessor()
		{
			state_size = sizeof(StateT);
			state_align = alignof(StateT);
			state_initialize = &StateInitializeProc;
			state_destroy = &StateDestroyProc;
			process = &ProcessProc;
		}

	private:
		static void StateInitializeProc(void* state_ptr, flecs::world_t* world, flecs::entity_t spatial_world_entity)
		{
			new (state_ptr) StateT(world, spatial_world_entity);
		}

		static void StateDestroyProc(void* state_ptr)
		{
			std::destroy_at(static_cast<StateT*>(state_ptr));
		}

		static void ProcessProc(void* state_ptr, World& spatial_world, size_t scale_index, Scale& scale)
		{
			static_cast<StateT*>(state_ptr)->Process(spatial_world, scale_index, static_cast<ScaleT&>(scale));
		}
	};
}