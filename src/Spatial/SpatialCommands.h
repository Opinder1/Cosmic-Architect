#pragma once

#include "SpatialAABB.h"

#include "Util/VariableLengthArray.h"

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
		void(*state_initialize)(void*, flecs::entity);
		void(*state_destroy)(void*);
	};

	struct SpatialNodeCommandProcessorBase : SpatialCommandProcessorBase
	{
		void(*process)(void*, SpatialWorld3DComponent&, SpatialScale3D&, SpatialNode3D&);

		bool operator==(const SpatialNodeCommandProcessorBase& other) { return process == other.process; }
	};

	struct SpatialRegionCommandProcessorBase : SpatialCommandProcessorBase
	{
		void(*process)(void*, SpatialWorld3DComponent&, SpatialAABB);

		bool operator==(const SpatialRegionCommandProcessorBase& other) { return process == other.process; }
	};

	struct SpatialScaleCommandProcessorBase : SpatialCommandProcessorBase
	{
		void(*process)(void*, SpatialWorld3DComponent&, size_t, SpatialScale3D&);

		bool operator==(const SpatialScaleCommandProcessorBase& other) { return process == other.process; }
	};

	template<class ProcessorT, class CommandT, class Callable>
	void ProcessCommands(flecs::entity entity, const std::vector<ProcessorT>& processors, const std::vector<CommandT>& commands, Callable&& command_processor)
	{
		static_assert(std::is_base_of_v<SpatialCommandProcessorBase, ProcessorT>);

		if (commands.empty()) // Don't continue if there aren't any commands
		{
			return;
		}

		VariableLengthArray<void*> states = MakeVariableLengthArray(void*, processors.size());

		for (size_t i = 0; i < states.size(); i++)
		{
			states[i] = alloca(processors[i].state_size);

			processors[i].state_initialize(states[i], entity);
		}

		for (const CommandT& command : commands)
		{
			command_processor(command, [&processors, &states](auto&&... args)
			{
				for (size_t i = 0; i < states.size(); i++)
				{
					processors[i].process(states[i], args...);
				}
			});
		}

		for (size_t i = 0; i < states.size(); i++)
		{
			processors[i].state_destroy(states[i]);
		}
	}

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

	template<class StateT, class ScaleT, class NodeT>
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