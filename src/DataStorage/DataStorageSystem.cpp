#include "DataStorageSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/UUID.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/os.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

void DataStorageSystem::OnInitialize(sim::Simulation& simulation)
{
	godot::UtilityFunctions::print(godot::OS::get_singleton()->get_executable_path());
}

void DataStorageSystem::OnShutdown(sim::Simulation& simulation)
{

}