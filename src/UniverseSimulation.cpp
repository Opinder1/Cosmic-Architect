#include "UniverseSimulation.h"

#include "Universe/UniverseModule.h"
#include "Galaxy/GalaxyModule.h"
#include "GalaxyRender/GalaxyRenderModule.h"
#include "Player/PlayerModule.h"
#include "Spatial3D/SpatialModule.h"
#include "Voxel/VoxelModule.h"
#include "VoxelRender/VoxelRenderModule.h"
#include "Render/RenderModule.h"
#include "DebugRender/DebugRenderModule.h"
#include "Simulation/SimulationModule.h"
#include "Physics3D/PhysicsModule.h"
#include "Entity/EntityModule.h"

#include "Entity/EntityComponents.h"

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
		debugrender::WorldUpdate(simulation, world);
		universe::WorldUpdate(simulation, world);
	}

	void SimulationGalaxyWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.galaxy_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		debugrender::WorldUpdate(simulation, world);
		galaxy::WorldUpdate(simulation, world);
	}

	void SimulationStarSystemWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.star_system_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		debugrender::WorldUpdate(simulation, world);
	}

	void SimulationPlanetWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.planet_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		debugrender::WorldUpdate(simulation, world);
	}

	void SimulationSpaceStationWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.space_station_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		debugrender::WorldUpdate(simulation, world);
	}

	void SimulationSpaceShipWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.space_ship_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		debugrender::WorldUpdate(simulation, world);
	}

	void SimulationVehicleWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.vehicle_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		debugrender::WorldUpdate(simulation, world);
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
		debugrender::ScaleUpdate(simulation, scale);
		universe::ScaleUpdate(simulation, scale);
	}

	void SimulationGalaxyScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.galaxy_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		debugrender::ScaleUpdate(simulation, scale);
		galaxy::ScaleUpdate(simulation, scale);
	}

	void SimulationStarSystemScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.star_system_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		debugrender::ScaleUpdate(simulation, scale);
	}

	void SimulationPlanetScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.planet_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		debugrender::ScaleUpdate(simulation, scale);
	}

	void SimulationSpaceStationScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.space_station_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		debugrender::ScaleUpdate(simulation, scale);
	}

	void SimulationSpaceShipScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.space_ship_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		debugrender::ScaleUpdate(simulation, scale);
	}

	void SimulationVehicleScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.vehicle_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		debugrender::ScaleUpdate(simulation, scale);
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
		entity::WRef entity = simulation.updating_entities[index];

		DEBUG_THREAD_CHECK_WRITE(entity.Data());

		simulation.entity_factory.DoEvent(simulation, entity::Event::TaskUpdate, entity::EventData{ entity });
	}

	// Run all entities in parallel doing entity specific code
	void SimulationEntityUpdate(Simulation& simulation)
	{
		TaskData entity_task = { simulation, &SimulationEntityUpdateTask, simulation.updating_entities.size() };

		SimulationDoTasks(simulation, entity_task);
	}
	
	void SimulationWorkerUpdateTask(Simulation& simulation, size_t index)
	{
		simulation::WorkerUpdate(simulation, index);
		rendering::WorkerUpdate(simulation, index);
		debugrender::WorkerUpdate(simulation, index);
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
		return	simulation::IsUnloadDone(simulation) &&
				debugrender::IsUnloadDone(simulation) &&
				spatial3d::IsUnloadDone(simulation) &&
				universe::IsUnloadDone(simulation) &&
				galaxy::IsUnloadDone(simulation);
	}

	// Do singlethreaded update
	void SimulationSingleUpdate(Simulation& simulation)
	{
		simulation::Update(simulation);
		rendering::Update(simulation);
		debugrender::Update(simulation);
		spatial3d::Update(simulation);
		universe::Update(simulation);
		galaxy::Update(simulation);

		for (entity::WRef entity : simulation.updating_entities)
		{
			simulation.entity_factory.DoEvent(simulation, entity::Event::MainUpdate, entity::EventData{ entity });
		}
	}

	void SimulationInitialize(Simulation& simulation)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation); // Should be called singlethreaded
		
		godot::print_line("Initializing simulation");

		simulation::Initialize(simulation);
		rendering::Initialize(simulation);
		debugrender::Initialize(simulation);
		spatial3d::Initialize(simulation);
		universe::Initialize(simulation);
		galaxy::Initialize(simulation);
	}

	void SimulationUnload(Simulation& simulation)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation); // Should be called singlethreaded

		godot::print_line("Unloading simulation");

		simulation.unloading = true;

		// Unload all entities in the world. The entities memory still exists while there are references
		std::vector<entity::Ref> updating_entities = std::move(simulation.updating_entities);
		for (entity::WRef entity : updating_entities)
		{
			entity::OnUnloadEntity(simulation, entity);
		}
		updating_entities.clear();

		// Loop update while unloading
		while (!IsSimulationUnloadDone(simulation))
		{
			SimulationUpdate(simulation);
		}

		simulation.unloading = false;
	}

	void SimulationUninitialize(Simulation& simulation)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation); // Should be called singlethreaded

		// Unload modules
		galaxy::Uninitialize(simulation);
		universe::Uninitialize(simulation);
		spatial3d::Uninitialize(simulation);
		debugrender::Uninitialize(simulation);
		rendering::Uninitialize(simulation);
		simulation::Uninitialize(simulation);

		// Finally cleanup the entity factory after all references should have been removed
		simulation.entity_factory.Cleanup();
		DEBUG_ASSERT(simulation.entity_factory.GetCount() == 0, "We should have destroyed all entities");
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
}