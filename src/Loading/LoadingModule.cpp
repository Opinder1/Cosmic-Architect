#include "LoadingModule.h"
#include "LoadingWorld.h"

#include "Util/Debug.h"

namespace voxel_game::loading
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

	void WorldUpdate(Simulation& simulation, spatial3d::WorldPtr world)
	{
		WorldDoCreateCommands(simulation, world);
	}

	void ScaleUpdate(Simulation& simulation, spatial3d::ScalePtr scale)
	{
		WorldDoLoadCommands(simulation, scale);
	}
}