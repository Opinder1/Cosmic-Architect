#include "CommandQueue.h"
#include "Universe.h"
#include "UniverseSimulation.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <flecs/flecs.h>

void flecs_log_to_godot(int32_t level, const char* file, int32_t line, const char* msg)
{
	/* >0: Debug tracing. Only enabled in debug builds. */
	/*  0: Tracing. Enabled in debug/release builds. */
	/* -2: Warning. An issue occurred, but operation was successful. */
	/* -3: Error. An issue occurred, and operation was unsuccessful. */
	/* -4: Fatal. An issue occurred, and application must quit. */

	godot::String log_msg = godot::vformat("%s:%d: %s", file, line, msg);

	switch (level)
	{
	case -4:
		DEBUG_PRINT_ERROR(log_msg);
		DEBUG_CRASH();
		break;

	case -3:
		DEBUG_PRINT_ERROR(log_msg);
		break;

	case -2:
		DEBUG_PRINT_WARN(log_msg);
		break;

	case -1:
		DEBUG_PRINT_INFO(log_msg);
		break;

	case 0:
		godot::UtilityFunctions::print(log_msg);
		break;

	case 1:
	case 2:
	case 3:
	case 4:
		godot::UtilityFunctions::print(godot::vformat("[Trace %d]", level), log_msg);
		break;
	}
}

void* flecs_malloc_godot(ecs_size_t size)
{
	return memalloc(size);
}

void* flecs_realloc_godot(void* ptr, ecs_size_t size)
{
	if (ptr == nullptr)
	{
		return memalloc(size);
	}
	else
	{
		return memrealloc(ptr, size);
	}
}

void* flecs_calloc_godot(ecs_size_t size)
{
	return memset(memalloc(size), 0, size);
}

void flecs_free_godot(void* ptr)
{
	memfree(ptr);
}

void initialize_flecs()
{
	ecs_os_init();

	ecs_os_api.log_ = flecs_log_to_godot;
	ecs_os_api.log_level_ = 0;

	ecs_os_api.malloc_ = flecs_malloc_godot;
	ecs_os_api.realloc_ = flecs_realloc_godot;
	ecs_os_api.calloc_ = flecs_calloc_godot;
	ecs_os_api.free_ = flecs_free_godot;

	ecs_os_set_api(&ecs_os_api); // Set the initialized flag so we don't override the log_ again
}

void uninitialize_flecs()
{
	ecs_os_fini();
}

void initialize_voxelgame_module(godot::ModuleInitializationLevel p_level)
{
	if (p_level == godot::MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		godot::UtilityFunctions::print("Loading voxel world extension");

		initialize_flecs();

		godot::ClassDB::register_class<voxel_game::CommandQueue>();
		godot::ClassDB::register_class<voxel_game::CommandQueueServer>();
		godot::ClassDB::register_class<voxel_game::Universe>();
		godot::ClassDB::register_class<voxel_game::UniverseSimulation>();

		godot::Engine::get_singleton()->register_singleton("CommandQueueServer", voxel_game::CommandQueueServer::get_singleton());

		godot::UtilityFunctions::print("Loaded voxel world extension");
	}
}

void uninitialize_voxelgame_module(godot::ModuleInitializationLevel p_level)
{
	if (p_level == godot::MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		godot::UtilityFunctions::print("Unloading voxel world extension");

		voxel_game::UniverseSimulation::_cleanup_methods();
		voxel_game::Universe::_cleanup_methods();
		voxel_game::CommandQueueServer::_cleanup_methods();

		uninitialize_flecs();

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