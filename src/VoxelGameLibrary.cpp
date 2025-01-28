#include "UniverseSimulation/UniverseSimulation.h"

#include "Simulation/SimulationServer.h"

#include "Render/RenderAllocator.h"

#include "Commands/CommandWriter.h"
#include "Commands/CommandServer.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <flecs/flecs.h>

#include <easy/profiler.h>

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
		godot::_err_print_error(FUNCTION_STR, file, line, log_msg, true, false);
		godot::_err_flush_stdout();
		GENERATE_TRAP();
		break;

	case -3:
		godot::_err_print_error(FUNCTION_STR, file, line, log_msg, true, false);
		break;

	case -2:
		godot::_err_print_error(FUNCTION_STR, file, line, log_msg, true, true);
		break;

	case -1:
		godot::UtilityFunctions::print(log_msg);
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
	if (ptr != nullptr)
	{
		memfree(ptr);
	}
}

void flecs_perf_trace_push(const char* filename, size_t line, const char* name)
{
	EASY_NONSCOPED_BLOCK(name);
}

void flecs_perf_trace_pop(const char* filename, size_t line, const char* name)
{
	EASY_END_BLOCK;
}

void initialize_flecs()
{
	ecs_os_set_api_defaults();

	ecs_os_api_t api = ecs_os_get_api();

	api.log_ = flecs_log_to_godot;
	api.log_level_ = 0;
	api.flags_ = EcsOsApiHighResolutionTimer | EcsOsApiLogWithTimeStamp | EcsOsApiLogWithTimeDelta;

	api.malloc_ = flecs_malloc_godot;
	api.realloc_ = flecs_realloc_godot;
	api.calloc_ = flecs_calloc_godot;
	api.free_ = flecs_free_godot;

	api.perf_trace_push_ = flecs_perf_trace_push;
	api.perf_trace_pop_ = flecs_perf_trace_pop;

	ecs_os_set_api(&api); // Set the initialized flag so we don't override the log_ again

	ecs_os_init();
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

		godot::ClassDB::register_class<voxel_game::CommandWriter>();
		godot::ClassDB::register_class<voxel_game::CommandServer>();
		godot::ClassDB::register_class<voxel_game::rendering::AllocatorServer>();
		godot::ClassDB::register_class<voxel_game::SimulationServer>();
		godot::ClassDB::register_class<voxel_game::UniverseSimulation>();

		godot::Engine::get_singleton()->register_singleton("CommandServer", voxel_game::CommandServer::get_singleton());
		godot::Engine::get_singleton()->register_singleton("AllocatorServer", voxel_game::rendering::AllocatorServer::get_singleton());

		profiler::startListen();

		godot::UtilityFunctions::print("Loaded voxel world extension");
	}
}

void uninitialize_voxelgame_module(godot::ModuleInitializationLevel p_level)
{
	if (p_level == godot::MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		godot::UtilityFunctions::print("Unloading voxel world extension");

		profiler::stopListen();

		godot::Engine::get_singleton()->unregister_singleton("AllocatorServer");
		godot::Engine::get_singleton()->unregister_singleton("CommandServer");

		voxel_game::UniverseSimulation::_cleanup_methods();
		voxel_game::rendering::AllocatorServer::_cleanup_methods();
		voxel_game::CommandServer::_cleanup_methods();

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