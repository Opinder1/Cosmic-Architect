#include "register_types.h"

#include "Simulation/SimulationServer.h"

#include "Simulation/Network/NetworkMessager.h"

#include "VoxelWorld.h"

#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>

void initialize_voxelgame_module(godot::ModuleInitializationLevel p_level)
{
	if (p_level == godot::MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		godot::UtilityFunctions::print("Loading voxel world extension");

		sim::SimulationServer::Initialize();

		sim::NetworkMessager::Initialize();

		// 

		//ClassDB::register_class<voxel_world::VoxelWorldNode>();

		godot::UtilityFunctions::print("Loaded voxel world extension");
	}
}

void uninitialize_voxelgame_module(godot::ModuleInitializationLevel p_level)
{
	if (p_level == godot::MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		godot::UtilityFunctions::print("Unloading voxel world extension");

		sim::NetworkMessager::Uninitialize();

		sim::SimulationServer::Uninitialize();

		godot::UtilityFunctions::print("Unloaded voxel world extension");
	}
}

extern "C"
{
	GDExtensionBool GDE_EXPORT voxelworld_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization)
	{
		godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

		init_obj.register_initializer(initialize_voxelgame_module);
		init_obj.register_terminator(uninitialize_voxelgame_module);
		init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}