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
	void OnDestroySpatialEntity(Simulation& simulation, entity::Ptr entity)
	{

	}

	void Initialize(Simulation& simulation)
	{
		simulation.entity_factory.AddCallback<CWorld>(entity::Event::Destroy, cb::Bind<OnDestroySpatialEntity>());
	}

	void Uninitialize(Simulation& simulation)
	{
		DEBUG_ASSERT(simulation.spatial_worlds.empty(), "All worlds should have been destroyed");
		DEBUG_ASSERT(simulation.spatial_scales.empty(), "All scales should have been destroyed");
	}

	void Update(Simulation& simulation)
	{

	}

	void WorkerUpdate(Simulation& simulation, size_t index)
	{

	}

	void WorldUpdate(Simulation& simulation, WorldPtr world)
	{
		if (simulation.uninitializing)
		{
			UnloadWorld(world);
		}
		else
		{
			WorldDoNodeLoadCommands(world, simulation.frame_start_time);

			WorldDoNodeUnloadCommands(world);
		}
	}

	void ScaleUpdate(Simulation& simulation, ScalePtr scale)
	{
		if (!simulation.uninitializing)
		{
			ScaleLoadNodesAroundLoaders(scale, simulation.frame_start_time);

			ScaleUpdateEntityNodes(scale);
		}

		ScaleUnloadUnutilizedNodes(scale, simulation.frame_start_time);
	}
}