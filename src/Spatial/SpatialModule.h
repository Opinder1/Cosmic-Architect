#pragma once

#include "Util/Time.h"

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct SpatialCoord3D;
	struct SpatialNode3D;
	struct SpatialWorld3D;

	struct WorldTime
	{
		uint64_t frame_index = 0;
		Clock::time_point frame_start;
	};

	struct WorldLoaderProgressPhase {};
	struct WorldRegionProgressPhase {};
	struct WorldNodeProgressPhase {};
	struct WorldScaleProgressPhase {};
	struct WorldProgressPhase {};

	struct SpatialModule
	{
		SpatialModule(flecs::world& world);

		static void AddSingletons(flecs::world& world);

		static void AddTags(flecs::world& world);

		static void AddComponents(flecs::world& world);

		static void AddObservers(flecs::world& world);

		static void AddPhases(flecs::world& world);

		static void AddSystems(flecs::world& world);

		static void AddSyncs(flecs::world& world);

		static void AddExamples(flecs::world& world);

		static SpatialNode3D* GetNode(const SpatialWorld3D& world, SpatialCoord3D coord);
	};
}