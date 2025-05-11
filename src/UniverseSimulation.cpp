#include "UniverseSimulation.h"

#include "Universe/UniverseModule.h"
#include "Galaxy/GalaxyModule.h"
#include "GalaxyRender/GalaxyRenderModule.h"
#include "Player/PlayerModule.h"
#include "Spatial3D/SpatialModule.h"
#include "Voxel/VoxelModule.h"
#include "VoxelRender/VoxelRenderModule.h"
#include "Render/RenderModule.h"
#include "Simulation/SimulationModule.h"
#include "Physics3D/PhysicsModule.h"
#include "Loading/LoadingModule.h"

#include "Util/SmallVector.h"

#include <godot_cpp/classes/worker_thread_pool.hpp>

namespace voxel_game
{
	// The simulation task callback executor
	void TaskCallback(void* userdata, uint32_t element)
	{
		TaskData* taskdata = reinterpret_cast<TaskData*>(userdata);

		taskdata->callback(taskdata->simulation, element);
	}

	// Run a single task group and wait for it after
	void SimulationDoTasks(Simulation& simulation, TaskData& task_data)
	{
		if (simulation.processor_count == 0)
		{
			for (size_t index = 0; index < task_data.count; index++)
			{
				task_data.callback(simulation, index);
			}
		}
		else
		{
			simulation.thread_mode = true;

			godot::WorkerThreadPool* thread_pool = godot::WorkerThreadPool::get_singleton();

			uint64_t id = thread_pool->add_native_group_task(
				&TaskCallback,
				&task_data,
				task_data.count,
				simulation.processor_count,
				simulation.high_priority);

			thread_pool->wait_for_group_task_completion(id);

			simulation.thread_mode = false;
		}
	}

	// Run multiple task groups at the same time and wait for them all after all have been started
	void SimulationDoMultitasks(Simulation& simulation, Span<TaskData> task_data)
	{
		if (task_data.Size() == 1)
		{
			SimulationDoTasks(simulation, *task_data.Data());
		}
		else if (simulation.processor_count == 0)
		{
			for (TaskData& data : task_data)
			{
				SimulationDoTasks(simulation, data);
			}
		}
		else
		{
			simulation.thread_mode = true;

			godot::WorkerThreadPool* thread_pool = godot::WorkerThreadPool::get_singleton();

			GrowingSmallVector<uint64_t, 16> ids;

			for (TaskData& data : task_data)
			{
				uint64_t id = thread_pool->add_native_group_task(
					&TaskCallback,
					&data,
					data.count,
					simulation.processor_count,
					simulation.high_priority);

				ids.push_back(id);
			}

			for (size_t index = 0; index < task_data.Size(); index++)
			{
				thread_pool->wait_for_group_task_completion(ids[index]);
			}

			simulation.thread_mode = false;
		}
	}

	void SimulationUniverseWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.universe_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		loading::WorldUpdate(simulation, world);
		universe::WorldUpdate(simulation, world);
	}

	void SimulationGalaxyWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.galaxy_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		loading::WorldUpdate(simulation, world);
		galaxy::WorldUpdate(simulation, world);
	}

	void SimulationStarSystemWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.star_system_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		loading::WorldUpdate(simulation, world);
	}

	void SimulationPlanetWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.planet_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		loading::WorldUpdate(simulation, world);
	}

	void SimulationSpaceStationWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.space_station_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		loading::WorldUpdate(simulation, world);
	}

	void SimulationSpaceShipWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.space_ship_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		loading::WorldUpdate(simulation, world);
	}

	void SimulationVehicleWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.vehicle_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		loading::WorldUpdate(simulation, world);
	}

	// Do parallel spatial world updates
	void SimulationWorldUpdate(Simulation& simulation)
	{
		TaskData world_tasks[7] = {
			{ simulation, &SimulationUniverseWorldUpdateTask, simulation.universe_type.worlds.size() },
			{ simulation, &SimulationGalaxyWorldUpdateTask, simulation.galaxy_type.worlds.size() },
			{ simulation, &SimulationStarSystemWorldUpdateTask, simulation.star_system_type.worlds.size() },
			{ simulation, &SimulationPlanetWorldUpdateTask, simulation.planet_type.worlds.size() },
			{ simulation, &SimulationSpaceStationWorldUpdateTask, simulation.space_station_type.worlds.size() },
			{ simulation, &SimulationSpaceShipWorldUpdateTask, simulation.space_ship_type.worlds.size() },
			{ simulation, &SimulationVehicleWorldUpdateTask, simulation.vehicle_type.worlds.size() },
		};

		SimulationDoMultitasks(simulation, world_tasks);
	}

	void SimulationUniverseScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.universe_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		loading::ScaleUpdate(simulation, scale);
		universe::ScaleUpdate(simulation, scale);
	}

	void SimulationGalaxyScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.galaxy_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		loading::ScaleUpdate(simulation, scale);
		galaxy::ScaleUpdate(simulation, scale);
	}

	void SimulationStarSystemScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.star_system_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		loading::ScaleUpdate(simulation, scale);
	}

	void SimulationPlanetScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.planet_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		loading::ScaleUpdate(simulation, scale);
	}

	void SimulationSpaceStationScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.space_station_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		loading::ScaleUpdate(simulation, scale);
	}

	void SimulationSpaceShipScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.space_ship_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		loading::ScaleUpdate(simulation, scale);
	}

	void SimulationVehicleScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.vehicle_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		loading::ScaleUpdate(simulation, scale);
	}

	// Do parallel spatial scale updates
	void SimulationScaleUpdate(Simulation& simulation)
	{
		TaskData scale_tasks[7] = {
			{ simulation, &SimulationUniverseScaleUpdateTask, simulation.universe_type.scales.size() },
			{ simulation, &SimulationGalaxyScaleUpdateTask, simulation.galaxy_type.scales.size() },
			{ simulation, &SimulationStarSystemScaleUpdateTask, simulation.star_system_type.scales.size() },
			{ simulation, &SimulationPlanetScaleUpdateTask, simulation.planet_type.scales.size() },
			{ simulation, &SimulationSpaceStationScaleUpdateTask, simulation.space_station_type.scales.size() },
			{ simulation, &SimulationSpaceShipScaleUpdateTask, simulation.space_ship_type.scales.size() },
			{ simulation, &SimulationVehicleScaleUpdateTask, simulation.vehicle_type.scales.size() },
		};

		SimulationDoMultitasks(simulation, scale_tasks);
	}

	void SimulationEntityUpdateTask(Simulation& simulation, size_t index)
	{
		entity::WRef entity = simulation.entities[index];

		DEBUG_THREAD_CHECK_WRITE(entity.Data());

		simulation.entity_factory.DoEvent(simulation, entity, entity::Event::TaskUpdate);
	}

	// Run all entities in parallel doing entity specific code
	void SimulationEntityUpdate(Simulation& simulation)
	{
		TaskData entity_task{ simulation, &SimulationEntityUpdateTask, simulation.entities.size() };

		SimulationDoTasks(simulation, entity_task);
	}
	
	void SimulationWorkerUpdateTask(Simulation& simulation, size_t index)
	{
		simulation::WorkerUpdate(simulation, index);
		loading::WorkerUpdate(simulation, index);
		spatial3d::WorkerUpdate(simulation, index);
		universe::WorkerUpdate(simulation, index);
		galaxy::WorkerUpdate(simulation, index);
	}

	// Run worker tasks in parallel for systems that need them. There is usually one per CPU core
	void SimulationWorkerUpdate(Simulation& simulation)
	{
		TaskData worker_task{ simulation, &SimulationWorkerUpdateTask, simulation.worker_count };

		SimulationDoTasks(simulation, worker_task);
	}

	bool IsSimulationUnloadDone(Simulation& simulation)
	{
		return simulation.entities.size() == 0;
		/*
		return	simulation::IsUnloadDone(simulation) &&
				spatial3d::IsUnloadDone(simulation) &&
				loading::IsUnloadDone(simulation) &&
				universe::IsUnloadDone(simulation) &&
				galaxy::IsUnloadDone(simulation);
				*/
	}

	// Do singlethreaded update
	void SimulationSingleUpdate(Simulation& simulation)
	{
		simulation::Update(simulation);
		spatial3d::Update(simulation);
		loading::Update(simulation);
		universe::Update(simulation);
		galaxy::Update(simulation);

		for (entity::WRef entity : simulation.entities)
		{
			simulation.entity_factory.DoEvent(simulation, entity, entity::Event::Update);
		}
	}

	void SimulationInitialize(Simulation& simulation)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation); // Should be called singlethreaded

		simulation::Initialize(simulation);
		spatial3d::Initialize(simulation);
		loading::Initialize(simulation);
		universe::Initialize(simulation);
		galaxy::Initialize(simulation);
	}

	void SimulationUninitialize(Simulation& simulation)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation); // Should be called singlethreaded

		simulation.uninitializing = true;

		// Unload all entities in the world. The entities memory still exists while there are references
		for (entity::WRef entity : simulation.entities)
		{
			SimulationUnloadEntity(simulation, entity);
		}

		// Loop update while unloading
		while (!IsSimulationUnloadDone(simulation))
		{
			SimulationWorldUpdate(simulation);
			SimulationScaleUpdate(simulation);
			SimulationEntityUpdate(simulation);
			SimulationWorkerUpdate(simulation);
			SimulationSingleUpdate(simulation);
		}

		// Unload modules
		galaxy::Uninitialize(simulation);
		universe::Uninitialize(simulation);
		loading::Uninitialize(simulation);
		spatial3d::Uninitialize(simulation);
		simulation::Uninitialize(simulation);

		// Finally cleanup the entity factory after all references should have been removed
		simulation.entity_factory.Cleanup();
		DEBUG_ASSERT(simulation.entity_factory.GetCount() == 0, "We should have destroyed all entities");

		simulation.uninitializing = false;
	}

	void SimulationUpdate(Simulation& simulation)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation); // Should be called singlethreaded

		SimulationWorldUpdate(simulation);
		SimulationScaleUpdate(simulation);
		SimulationEntityUpdate(simulation);
		SimulationWorkerUpdate(simulation);
		SimulationSingleUpdate(simulation);

		simulation.entity_factory.Cleanup();
	}

	void SimulationLoadEntity(Simulation& simulation, entity::WRef entity)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation); // Should be called singlethreaded

		simulation.entity_factory.DoEvent(simulation, entity, entity::Event::Load);
	}

	void SimulationUnloadEntity(Simulation& simulation, entity::WRef entity)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation); // Should be called singlethreaded

		simulation.entity_factory.DoEvent(simulation, entity, entity::Event::Unload);
	}
}