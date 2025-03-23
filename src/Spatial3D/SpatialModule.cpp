#include "SpatialModule.h"
#include "SpatialComponents.h"
#include "SpatialTraverse.h"

#include "Simulation/SimulationComponents.h"

#include "Physics3D/PhysicsComponents.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

#include <easy/profiler.h>

namespace voxel_game::spatial3d
{
	flecs::entity CreateSyncPhase(flecs::world& world, flecs::entity_t parent)
	{
		flecs::entity sync_phase = world.entity(DEBUG_ONLY("ThreadSyncPhase"))
			.add(flecs::Phase)
			.child_of(parent)
			.depends_on(parent);

		world.system(DEBUG_ONLY("ThreadSyncSystem"))
			.immediate()
			.kind(sync_phase)
			.run([](flecs::iter& it) {});

		return sync_phase;
	}

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
		world.import<sim::Components>();
		world.import<physics3d::Components>();

		world.observer<CLoader, CWorld>()
			.event(flecs::OnAdd)
			.term_at(1).up(flecs::ChildOf)
			.each([](CLoader& loader, CWorld& world)
		{
			(world.world->*&World::loaders).insert(loader.loader);
		});

		world.observer<CLoader, CWorld>()
			.event(flecs::OnRemove)
			.term_at(1).up(flecs::ChildOf)
			.each([](CLoader& loader, CWorld& world)
		{
			(world.world->*&World::loaders).erase(loader.loader);
		});

		// Systems

		// System to initialize spatial nodes that have been added
		world.system<CWorld, const sim::CFrame>(DEBUG_ONLY("WorldCreateNodes"))
			//.multi_threaded()
			.term_at(1).singleton()
			.each([](CWorld& spatial_world, const sim::CFrame& frame)
		{
			EASY_BLOCK("WorldCreateNodes");

			WorldCreateNodes(spatial_world.world, frame);
		});

		// System to delete spatial nodes that have been marked to unload
		world.system<CWorld>(DEBUG_ONLY("WorldDestroyNodes"))
			//.multi_threaded()
			.each([](CWorld& spatial_world)
		{
			EASY_BLOCK("WorldDestroyNodes");

			WorldDestroyNodes(spatial_world.world);
		});

		// Systen to create or update all nodes in the range of loaders
		world.system<CScale, const CWorld, const sim::CFrame>(DEBUG_ONLY("LoaderTouchNodes"))
			.multi_threaded()
			.term_at(1).up(flecs::ChildOf)
			.term_at(2).singleton()
			.each([](CScale& spatial_scale, const CWorld& spatial_world, const sim::CFrame& frame)
		{
			EASY_BLOCK("LoaderTouchNodes");

			ScaleLoadNodes(spatial_world.world, spatial_scale.scale, frame);
		});

		// System to mark any nodes that are no longer being observed to be unloaded
		world.system<CScale, const CWorld, const sim::CFrame>(DEBUG_ONLY("ScaleUnloadUnusedNodes"))
			.multi_threaded()
			.term_at(1).up(flecs::ChildOf)
			.term_at(2).singleton()
			.each([](CScale& spatial_scale, const CWorld& spatial_world, const sim::CFrame& frame)
		{
			EASY_BLOCK("ScaleUnloadUnusedNodes");

			ScaleUnloadNodes(spatial_world.world, spatial_scale.scale, frame);
		});

		// System to delete spatial nodes that have been marked to unload
		world.system<const physics3d::CPosition, CLoader>(DEBUG_ONLY("LoaderUpdatePosition"))
			.multi_threaded()
			.each([](const physics3d::CPosition& position, CLoader& spatial_loader)
		{
			spatial_loader.loader->position = position.position;
		});

		// System to delete spatial nodes that have been marked to unload
		world.system<const physics3d::CPosition, CEntity>(DEBUG_ONLY("EntityUpdatePosition"))
			.multi_threaded()
			.each([](const physics3d::CPosition& position, CEntity& spatial_entity)
		{
			//spatial_entity.entity->position = position.position;
		});
	}

	void InitializeWorldScaleEntities(flecs::entity world_entity, WorldRef world)
	{
		flecs::scoped_world scope = world_entity.scope();

		for (uint8_t scale_index = 0; scale_index < world->*&World::max_scale; scale_index++)
		{
			flecs::entity marker_entity = scope.entity();

#if defined(DEBUG_ENABLED)
			marker_entity.set_name(godot::vformat("Scale%d", scale_index).utf8());
#endif

			marker_entity.emplace<CScale>((world->*&World::scales)[scale_index]);
		}
	}
}