#pragma once

#include "SpatialAABB.h"

#include "Util/Poly.h"

#include <flecs/flecs.h>

#include <godot_cpp/variant/vector3i.hpp>

#include <robin_hood/robin_hood.h>

#include <vector>
#include <deque>

namespace voxel_game
{
	struct EntityCommandProcessorBase
	{
		size_t state_size;
		void(*state_initialize)(void*, flecs::world_t*, flecs::entity_t);
		void(*state_destroy)(void*);
	};

	template<class ProcessorT, class CommandT, class Callable>
	void RunEntityCommandsWithProcessors(flecs::entity entity, const std::vector<ProcessorT>& processors, const std::vector<CommandT>& commands, Callable&& command_callback)
	{
		static_assert(std::is_base_of_v<EntityCommandProcessorBase, ProcessorT>);

		if (commands.empty()) // Don't continue if there aren't any commands
		{
			return;
		}

		VariableLengthArray<void*> states = MakeVariableLengthArray(void*, processors.size());

		for (size_t i = 0; i < states.size(); i++)
		{
			states[i] = alloca(processors[i].state_size);

			processors[i].state_initialize(states[i], entity.world(), entity);
		}

		auto run_processors_delegate = [&processors, &states](auto&&... args)
		{
			for (size_t i = 0; i < states.size(); i++)
			{
				processors[i].process(states[i], args...);
			}
		};

		for (const CommandT& command : commands)
		{
			command_callback(command, run_processors_delegate);
		}

		for (size_t i = 0; i < states.size(); i++)
		{
			processors[i].state_destroy(states[i]);
		}
	}
}

namespace voxel_game::spatial3d
{
	struct World;
	struct Scale;

	struct SpatialNodeCommandProcessorBase : EntityCommandProcessorBase
	{
		void(*process)(void*, World&, godot::Vector3i, Poly);

		bool operator==(const SpatialNodeCommandProcessorBase& other) { return process == other.process; }
	};

	struct SpatialRegionCommandProcessorBase : EntityCommandProcessorBase
	{
		void(*process)(void*, World&, const AABB&);

		bool operator==(const SpatialRegionCommandProcessorBase& other) { return process == other.process; }
	};

	struct SpatialScaleCommandProcessorBase : EntityCommandProcessorBase
	{
		void(*process)(void*, World&, size_t);

		bool operator==(const SpatialScaleCommandProcessorBase& other) { return process == other.process; }
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
		static void StateInitializeProc(void* state_ptr, flecs::world_t* world, flecs::entity_t spatial_world_entity)
		{
			new (state_ptr) StateT(world, spatial_world_entity);
		}

		static void StateDestroyProc(void* state_ptr)
		{
			std::destroy_at(static_cast<StateT*>(state_ptr));
		}

		static void ProcessProc(void* state_ptr, World& world, godot::Vector3i pos, Poly poly)
		{
			static_cast<StateT*>(state_ptr)->Process(world, pos, poly);
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
		static void StateInitializeProc(void* state_ptr, flecs::world_t* world, flecs::entity_t spatial_world_entity)
		{
			new (state_ptr) StateT(world, spatial_world_entity);
		}

		static void StateDestroyProc(void* state_ptr)
		{
			std::destroy_at(static_cast<StateT*>(state_ptr));
		}

		static void ProcessProc(void* state_ptr, World& world, const AABB& region)
		{
			static_cast<StateT*>(state_ptr)->Process(world, region);
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
		static void StateInitializeProc(void* state_ptr, flecs::world_t* world, flecs::entity_t spatial_world_entity)
		{
			new (state_ptr) StateT(world, spatial_world_entity);
		}

		static void StateDestroyProc(void* state_ptr)
		{
			std::destroy_at(static_cast<StateT*>(state_ptr));
		}

		static void ProcessProc(void* state_ptr, World& world, size_t scale_index)
		{
			static_cast<StateT*>(state_ptr)->Process(world, scale_index);
		}
	};

	// System to tick spatial nodes that have been marked to tick
	world.system<const SpatialScale3DWorkerComponent, SpatialWorld3DComponent>(DEBUG_ONLY("SpatialWorldProcessTickCommands"))
		.multi_threaded()
		.kind<WorldScaleWorkerPhase>()
		.term_at(2).parent()
		.each([&world](flecs::entity worker_entity, const SpatialScale3DWorkerComponent& scale_worker, SpatialWorld3DComponent& spatial_world)
	{
		DEBUG_THREAD_CHECK_READ(&world, &spatial_world);

		size_t scale_index = scale_worker.scale;
		SpatialScale3D& scale = *spatial_world.scales[scale_index];

		DEBUG_THREAD_CHECK_WRITE(&world, &scale);

		if (spatial_world.tick_command_processors.empty()) // Don't continue if there aren't any processors but make sure to clear the commands
		{
			scale.tick_commands.clear();
			return;
		}

		RunEntityCommandsWithProcessors(worker_entity.parent(), spatial_world.tick_command_processors, scale.tick_commands,
			[&world, &spatial_world, &scale](godot::Vector3i pos, auto& run_processors_delegate)
		{
			SpatialNodeMap::iterator it = scale.nodes.find(pos);
			DEBUG_ASSERT(it != scale.nodes.end(), "We should have only sent unload commands for existing nodes");

			SpatialNode3D& node = *it->second;

			DEBUG_THREAD_CHECK_WRITE(&world, &node);

			run_processors_delegate(spatial_world, scale, node);
		});

		scale.tick_commands.clear();
	});
}