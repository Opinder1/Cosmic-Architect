#pragma once

#include "Util/Hash.h"

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <flecs/flecs.h>

#include <robin_hood/robin_hood.h>

namespace voxel_game::sim
{
	struct CConfig;
	class ThreadEntityPool;

	using ConfigDefaults = robin_hood::unordered_map<godot::String, godot::Variant>;

	struct Module
	{
		Module(flecs::world& world);
	};

	ThreadEntityPool& GetPool();
	
	void LoadJsonConfig(CConfig& config);

	void SaveJsonConfig(CConfig& config);

	void InitializeConfig(flecs::entity entity, const godot::String& path, const ConfigDefaults& defaults);
}