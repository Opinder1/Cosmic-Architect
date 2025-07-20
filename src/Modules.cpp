#include "Modules.h"

#include "Simulation/SimulationModule.h"
#include "Render/RenderModule.h"
#include "DebugRender/DebugRenderModule.h"
#include "Spatial3D/SpatialModule.h"
#include "Universe/UniverseModule.h"
#include "Galaxy/GalaxyModule.h"

namespace voxel_game
{
	const Module simulation_module_schematic =
	{
		&simulation::Initialize,
		&simulation::Uninitialize,
		&simulation::IsUnloadDone,
		&simulation::Update,
		&simulation::WorkerUpdate
	};

	const Module rendering_module_schematic =
	{
		&rendering::Initialize,
		&rendering::Uninitialize,
		&rendering::IsUnloadDone,
		&rendering::Update,
		&rendering::WorkerUpdate
	};

	const Module debugrender_module_schematic =
	{
		&debugrender::Initialize,
		&debugrender::Uninitialize,
		&debugrender::IsUnloadDone,
		&debugrender::Update,
		&debugrender::WorkerUpdate
	};

	const Module spatial3d_module_schematic =
	{
		&spatial3d::Initialize,
		&spatial3d::Uninitialize,
		&spatial3d::IsUnloadDone,
		&spatial3d::Update,
		&spatial3d::WorkerUpdate
	};

	const Module universe_module_schematic =
	{
		&universe::Initialize,
		&universe::Uninitialize,
		&universe::IsUnloadDone,
		&universe::Update,
		&universe::WorkerUpdate
	};

	const Module galaxy_module_schematic =
	{
		&galaxy::Initialize,
		&galaxy::Uninitialize,
		&galaxy::IsUnloadDone,
		&galaxy::Update,
		&galaxy::WorkerUpdate
	};
}