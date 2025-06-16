#include "UniverseServer/UniverseServer.h"

#include "Simulation/SimulationServer.h"

#include "Render/RenderAllocator.h"

#include "Commands/CommandWriter.h"
#include "Commands/CommandServer.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <easy/profiler.h>

#include <fmt/format.h>

void initialize_voxelgame_module(godot::ModuleInitializationLevel p_level)
{
	if (p_level == godot::MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		godot::print_line("Loading voxel world extension");

		godot::ClassDB::register_class<voxel_game::CommandWriter>();
		godot::ClassDB::register_class<voxel_game::CommandServer>();
		godot::ClassDB::register_class<voxel_game::rendering::AllocatorServer>();
		godot::ClassDB::register_abstract_class<voxel_game::SimulationServer>();
		godot::ClassDB::register_class<voxel_game::UniverseServer>();

		godot::Engine::get_singleton()->register_singleton("CommandServer", voxel_game::CommandServer::get_singleton());
		godot::Engine::get_singleton()->register_singleton("AllocatorServer", voxel_game::rendering::AllocatorServer::get_singleton());
		godot::Engine::get_singleton()->register_singleton("UniverseServer", voxel_game::UniverseServer::get_singleton());

		profiler::startListen();

		godot::print_line("Loaded voxel world extension");
	}
}

void uninitialize_voxelgame_module(godot::ModuleInitializationLevel p_level)
{
	if (p_level == godot::MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		godot::print_line("Unloading voxel world extension");

		profiler::stopListen();

		godot::Engine::get_singleton()->unregister_singleton("UniverseServer");
		godot::Engine::get_singleton()->unregister_singleton("AllocatorServer");
		godot::Engine::get_singleton()->unregister_singleton("CommandServer");

		voxel_game::UniverseServer::_cleanup_methods();
		voxel_game::rendering::AllocatorServer::_cleanup_methods();
		voxel_game::CommandServer::_cleanup_methods();

		godot::print_line("Unloaded voxel world extension");
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