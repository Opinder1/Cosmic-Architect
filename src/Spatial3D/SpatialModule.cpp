#include "SpatialModule.h"
#include "SpatialComponents.h"
#include "SpatialTraverse.h"

#include "UniverseSimulation.h"

#include "Physics3D/PhysicsComponents.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

#include <easy/profiler.h>

namespace voxel_game::spatial3d
{
	void Initialize(Simulation& simulation)
	{

	}

	void Uninitialize(Simulation& simulation)
	{

	}

	void Update(Simulation& simulation)
	{

	}

	void WorkerUpdate(Simulation& simulation, size_t index)
	{

	}

	void WorldUpdate(Simulation& simulation, WorldRef world)
	{
		WorldCreateNodes(world, simulation.frame_start_time);

		WorldDestroyNodes(world);
	}

	void ScaleUpdate(Simulation& simulation, ScaleRef scale)
	{
		ScaleLoadNodes(scale->*&Scale::world, scale, simulation.frame_start_time);

		ScaleUnloadNodes(scale->*&Scale::world, scale, simulation.frame_start_time);
	}
}