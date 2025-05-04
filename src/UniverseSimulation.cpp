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
	void SimulationDoMultitasks(Simulation& simulation, TaskData* data, size_t count)
	{
		if (count == 1)
		{
			SimulationDoTasks(simulation, *data);
		}
		else if (simulation.processor_count == 0)
		{
			for (size_t index = 0; index < count; index++)
			{
				SimulationDoTasks(simulation, data[index]);
			}
		}
		else
		{
			simulation.thread_mode = true;

			godot::WorkerThreadPool* thread_pool = godot::WorkerThreadPool::get_singleton();

			GrowingSmallVector<uint64_t, 16> ids;

			for (size_t index = 0; index < count; index++)
			{
				uint64_t id = thread_pool->add_native_group_task(
					&TaskCallback,
					data + index,
					data[index].count,
					simulation.processor_count,
					simulation.high_priority);

				ids.push_back(id);
			}

			for (size_t index = 0; index < count; index++)
			{
				thread_pool->wait_for_group_task_completion(ids[index]);
			}

			simulation.thread_mode = false;
		}
	}

	void SimulationUniverseWorldUpdate(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.universe_type.worlds[index];

		spatial3d::WorldUpdate(simulation, world);
		loading::WorldUpdate(simulation, world);
		universe::WorldUpdate(simulation, world);
	}

	void SimulationUniverseScaleUpdate(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.universe_type.scales[index];

		spatial3d::ScaleUpdate(simulation, scale);
		loading::ScaleUpdate(simulation, scale);
		universe::ScaleUpdate(simulation, scale);
	}

	void SimulationGalaxyWorldUpdate(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.universe_type.worlds[index];

		spatial3d::WorldUpdate(simulation, world);
		loading::WorldUpdate(simulation, world);
		galaxy::WorldUpdate(simulation, world);
	}

	void SimulationGalaxyScaleUpdate(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.universe_type.scales[index];

		spatial3d::ScaleUpdate(simulation, scale);
		loading::ScaleUpdate(simulation, scale);
		galaxy::ScaleUpdate(simulation, scale);
	}

	void SimulationStarSystemWorldUpdate(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.universe_type.worlds[index];

		spatial3d::WorldUpdate(simulation, world);
		loading::WorldUpdate(simulation, world);
	}

	void SimulationStarSystemScaleUpdate(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.universe_type.scales[index];

		spatial3d::ScaleUpdate(simulation, scale);
		loading::ScaleUpdate(simulation, scale);
	}

	void SimulationPlanetWorldUpdate(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.universe_type.worlds[index];

		spatial3d::WorldUpdate(simulation, world);
		loading::WorldUpdate(simulation, world);
	}

	void SimulationPlanetScaleUpdate(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.universe_type.scales[index];

		spatial3d::ScaleUpdate(simulation, scale);
		loading::ScaleUpdate(simulation, scale);
	}

	void SimulationSpaceStationWorldUpdate(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.universe_type.worlds[index];

		spatial3d::WorldUpdate(simulation, world);
		loading::WorldUpdate(simulation, world);
	}

	void SimulationSpaceStationScaleUpdate(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.universe_type.scales[index];

		spatial3d::ScaleUpdate(simulation, scale);
		loading::ScaleUpdate(simulation, scale);
	}

	void SimulationSpaceShipWorldUpdate(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.universe_type.worlds[index];

		spatial3d::WorldUpdate(simulation, world);
		loading::WorldUpdate(simulation, world);
	}

	void SimulationSpaceShipScaleUpdate(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.universe_type.scales[index];

		spatial3d::ScaleUpdate(simulation, scale);
		loading::ScaleUpdate(simulation, scale);
	}

	void SimulationVehicleWorldUpdate(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.universe_type.worlds[index];

		spatial3d::WorldUpdate(simulation, world);
		loading::WorldUpdate(simulation, world);
	}

	void SimulationVehicleScaleUpdate(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.universe_type.scales[index];

		spatial3d::ScaleUpdate(simulation, scale);
		loading::ScaleUpdate(simulation, scale);
	}

	void SimulationInitialize(Simulation& simulation)
	{
		simulation::Initialize(simulation);
		loading::Initialize(simulation);
		spatial3d::Initialize(simulation);
		universe::Initialize(simulation);
		galaxy::Initialize(simulation);
	}

	void SimulationUninitialize(Simulation& simulation)
	{
		galaxy::Uninitialize(simulation);
		universe::Uninitialize(simulation);
		spatial3d::Uninitialize(simulation);
		loading::Uninitialize(simulation);
		simulation::Uninitialize(simulation);

		simulation.entities.clear();

		// Call Cleanup() multiple times to unwind dependency chains
		size_t entity_count = SIZE_MAX;
		while (entity_count > 0)
		{
			simulation.entity_factory.Cleanup();

			size_t new_entity_count = simulation.entity_factory.GetCount();

			DEBUG_ASSERT(new_entity_count != entity_count, "We should have removed entities. Cyclic references?");

			entity_count = new_entity_count;
		}
	}

	void SimulationSingleUpdate(Simulation& simulation)
	{
		simulation::Update(simulation);
		loading::Update(simulation);
		spatial3d::Update(simulation);
		universe::Update(simulation);
		galaxy::Update(simulation);
	}
	
	void SimulationWorkerUpdate(Simulation& simulation, size_t index)
	{
		simulation::WorkerUpdate(simulation, index);
		loading::WorkerUpdate(simulation, index);
		spatial3d::WorkerUpdate(simulation, index);
		universe::WorkerUpdate(simulation, index);
		galaxy::WorkerUpdate(simulation, index);
	}

	void SimulationEntityUpdate(Simulation& simulation, size_t index)
	{
		entity::WRef entity = simulation.entities[index];

		simulation.entity_factory.DoEvent(simulation, entity, entity::Event::Update);
	}

	void SimulationUpdate(Simulation& simulation)
	{
		// Do parallel spatial world updates
		TaskData world_tasks[7] = {
			{ simulation, &SimulationUniverseWorldUpdate, simulation.universe_type.worlds.size() },
			{ simulation, &SimulationGalaxyWorldUpdate, simulation.galaxy_type.worlds.size() },
			{ simulation, &SimulationStarSystemWorldUpdate, simulation.star_system_type.worlds.size() },
			{ simulation, &SimulationPlanetWorldUpdate, simulation.planet_type.worlds.size() },
			{ simulation, &SimulationSpaceStationWorldUpdate, simulation.space_station_type.worlds.size() },
			{ simulation, &SimulationSpaceShipWorldUpdate, simulation.space_ship_type.worlds.size() },
			{ simulation, &SimulationVehicleWorldUpdate, simulation.vehicle_type.worlds.size() },
		};

		SimulationDoMultitasks(simulation, world_tasks, 7);
		
		// Do parallel spatial scale updates
		TaskData scale_tasks[7] = {
			{ simulation, &SimulationUniverseScaleUpdate, simulation.universe_type.scales.size() },
			{ simulation, &SimulationGalaxyScaleUpdate, simulation.galaxy_type.scales.size() },
			{ simulation, &SimulationStarSystemScaleUpdate, simulation.star_system_type.scales.size() },
			{ simulation, &SimulationPlanetScaleUpdate, simulation.planet_type.scales.size() },
			{ simulation, &SimulationSpaceStationScaleUpdate, simulation.space_station_type.scales.size() },
			{ simulation, &SimulationSpaceShipScaleUpdate, simulation.space_ship_type.scales.size() },
			{ simulation, &SimulationVehicleScaleUpdate, simulation.vehicle_type.scales.size() },
		};

		SimulationDoMultitasks(simulation, scale_tasks, 7);

		// Run all entities in parallel doing entity specific code
		TaskData entity_task{ simulation, &SimulationEntityUpdate, simulation.entities.size() };

		SimulationDoTasks(simulation, entity_task);

		// Run worker tasks in parallel for systems that need them. There is usually one per CPU core
		TaskData worker_task{ simulation, &SimulationWorkerUpdate, simulation.worker_count };

		SimulationDoTasks(simulation, worker_task);

		// Do singlethreaded update
		SimulationSingleUpdate(simulation);
	}
}