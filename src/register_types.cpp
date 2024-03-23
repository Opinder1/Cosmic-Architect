#include "CommandQueue.h"
#include "Universe.h"
#include "UniverseSimulation.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/engine.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>

void initialize_voxelgame_module(godot::ModuleInitializationLevel p_level)
{
	if (p_level == godot::MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		godot::UtilityFunctions::print("Loading voxel world extension");

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