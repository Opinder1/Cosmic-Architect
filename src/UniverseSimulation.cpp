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

#include <godot_cpp/classes/worker_thread_pool.hpp>

namespace voxel_game
{
	struct TaskData
	{
		Simulation& simulation;
		void(*callback)(Simulation&, size_t);
	};

	void TaskCallback(void* userdata, uint32_t element)
	{
		TaskData* taskdata = reinterpret_cast<TaskData*>(userdata);

		taskdata->callback(taskdata->simulation, element);
	}

	void DoTasks(Simulation& simulation, void(*callback)(Simulation&, size_t), size_t count)
	{
		if (simulation.processor_count == 0)
		{
			for (size_t index = 0; index < count; index++)
			{
				callback(simulation, index);
			}
		}
		else
		{
			simulation.thread_mode = true;

			godot::WorkerThreadPool* thread_pool = godot::WorkerThreadPool::get_singleton();

			TaskData data{ simulation, callback };

			uint64_t id = thread_pool->add_native_group_task(
				&TaskCallback,
				&data,
				count,
				simulation.processor_count,
				simulation.high_priority);

			thread_pool->wait_for_group_task_completion(id);

			simulation.thread_mode = false;
		}
	}

	void Initialize(Simulation& simulation)
	{
		simulation::Initialize(simulation);
		spatial3d::Initialize(simulation);
		universe::Initialize(simulation);
		galaxy::Initialize(simulation);
	}

	void Uninitialize(Simulation& simulation)
	{
		simulation::Uninitialize(simulation);
		spatial3d::Uninitialize(simulation);
		universe::Uninitialize(simulation);
		galaxy::Uninitialize(simulation);
	}
	
	void WorkerUpdate(Simulation& simulation, size_t worker_index)
	{
		simulation::WorkerUpdate(simulation, worker_index);
		spatial3d::WorkerUpdate(simulation, worker_index);
		universe::WorkerUpdate(simulation, worker_index);
		galaxy::WorkerUpdate(simulation, worker_index);
	}

	void WorkerUpdateTask(void* userdata, uint32_t worker_index)
	{
		WorkerUpdate(*(Simulation*)userdata, worker_index);
	}

	void Update(Simulation& simulation)
	{
		simulation::Update(simulation);
		spatial3d::Update(simulation);
		universe::Update(simulation);
		galaxy::Update(simulation);

		DoTasks(simulation, &WorkerUpdate, simulation.worker_count);
	}
}