#pragma once

#include <cstdint>

namespace flecs
{
	class world;
}

namespace voxel_game::loading
{
	enum class LoadingMode
	{
		Loading,
		Loaded,
		Unloading,
		Unloaded
	};

	// Entities should manage their own progress and loading/unloading their children
	struct Loadable
	{
		LoadingMode mode = LoadingMode::Unloaded;
		size_t progress = 0;
		size_t child_count = 0;
	};

	struct Components
	{
		Components(flecs::world& world);
	};
}