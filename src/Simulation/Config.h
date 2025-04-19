#pragma once

#include "Util/GodotHash.h"

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace voxel_game::simulation
{
	struct Config
	{
		godot::String path;
		godot::Dictionary values;
	};

	using ConfigDefaults = robin_hood::unordered_map<godot::String, godot::Variant>;

	void LoadJsonConfig(Config& config);

	void SaveJsonConfig(Config& config);

	void InitializeConfig(Config& config, const godot::String& path, const ConfigDefaults& defaults);
}