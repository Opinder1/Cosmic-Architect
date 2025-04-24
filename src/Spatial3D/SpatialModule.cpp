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

	void WorldUpdate(Simulation& simulation, WorldPtr world)
	{
		WorldDoNodeCreateCommands(world, simulation.frame_start_time);

		WorldDoNodeDestroyCommands(world);
	}

	void ScaleUpdate(Simulation& simulation, ScalePtr scale)
	{
		ScaleLoadNodesAroundLoaders(scale, simulation.frame_start_time);

		ScaleUnloadUnutilizedNodes(scale, simulation.frame_start_time);

		ScaleUpdateEntityNodes(scale);
	}
}