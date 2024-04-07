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

	struct SpatialModule
	{
		SpatialModule(flecs::world& world);

		static void AddComponents(flecs::world& world);

		static void AddRelationships(flecs::world& world);

		static void AddObservers(flecs::world& world);

		static void AddPhases(flecs::world& world);

		static void AddSystems(flecs::world& world);

		static void AddSyncs(flecs::world& world);

		static void AddExamples(flecs::world& world);

		static SpatialNode3D* GetNode(const SpatialWorld3D& world, SpatialCoord3D coord);
	};
}