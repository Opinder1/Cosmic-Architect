#pragma once

#include "Util/GodotHash.h"

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <robin_hood/robin_hood.h>

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::simulation
{
	struct CConfig;

	using ConfigDefaults = robin_hood::unordered_map<godot::String, godot::Variant>;

	void Initialize(Simulation& simulation);

	void Uninitialize(Simulation& simulation);

	void Update(Simulation& simulation);

	void WorkerUpdate(Simulation& simulation, size_t index);
	
	void LoadJsonConfig(CConfig& config);

	void SaveJsonConfig(CConfig& config);

	void InitializeConfig(CConfig& config, const godot::String& path, const ConfigDefaults& defaults);
}