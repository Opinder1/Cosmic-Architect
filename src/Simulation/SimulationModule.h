#pragma once

#include <godot_cpp/variant/string.hpp>

#include <flecs/flecs.h>

namespace voxel_game::sim
{
	struct CConfig;
	class ThreadEntityPool;

	struct Module
	{
		Module(flecs::world& world);
	};

	ThreadEntityPool& GetPool();
	
	void LoadJsonConfig(godot::String path, CConfig& config);

	void SaveJsonConfig(godot::String path, CConfig& config);
}