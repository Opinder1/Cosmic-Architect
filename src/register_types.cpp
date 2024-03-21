#include "CommandQueue.h"
#include "FlecsWorld.h"
#include "FlecsWorldNode.h"
#include "UniverseSimulation.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>

void godot_log_flecs(
	int32_t level,     /* Logging level */
	const char* file,  /* File where message was logged */
	int32_t line,      /* Line it was logged */
	const char* msg)
{
	/* >0: Debug tracing. Only enabled in debug builds. */
	/*  0: Tracing. Enabled in debug/release builds. */
	/* -2: Warning. An issue occurred, but operation was successful. */
	/* -3: Error. An issue occurred, and operation was unsuccessful. */
	/* -4: Fatal. An issue occurred, and application must quit. */

	godot::String log_msg = godot::vformat("%s:%d: %s", file, line, msg);

	switch (level)
	{
	case 0:
		godot::UtilityFunctions::print(log_msg);
		break;

	case 1:
		DEBUG_PRINT_INFO(log_msg);
		break;

	case 2:
		DEBUG_PRINT_WARN(log_msg);
		break;

	case 3:
		DEBUG_PRINT_ERROR(log_msg);
		break;

	case 4:
		DEBUG_PRINT_ERROR(log_msg);
		DEBUG_CRASH();
		break;
	}
}

void initialize_voxelgame_module(godot::ModuleInitializationLevel p_level)
{
	if (p_level == godot::MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		godot::UtilityFunctions::print("Loading voxel world extension");

		ecs_os_api.log_ = godot_log_flecs;

		godot::ClassDB::register_class<voxel_game::CommandQueue>();
		godot::ClassDB::register_class<voxel_game::FlecsWorld>();
		godot::ClassDB::register_class<voxel_game::FlecsWorldNode>();
		godot::ClassDB::register_class<voxel_game::UniverseSimulation>();

		godot::UtilityFunctions::print("Loaded voxel world extension");
	}
}

void uninitialize_voxelgame_module(godot::ModuleInitializationLevel p_level)
{
	if (p_level == godot::MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		godot::UtilityFunctions::print("Unloading voxel world extension");

		voxel_game::UniverseSimulation::_cleanup_methods();
		voxel_game::CommandQueue::_cleanup_methods();

		godot::UtilityFunctions::print("Unloaded voxel world extension");
	}
}

extern "C"
{
	GDExtensionBool GDE_EXPORT voxelgame_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization)
	{
		godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

		init_obj.register_initializer(initialize_voxelgame_module);
		init_obj.register_terminator(uninitialize_voxelgame_module);
		init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}