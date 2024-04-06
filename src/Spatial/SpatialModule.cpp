#include "SpatialModule.h"
#include "SpatialComponents.h"

#include "Physics/PhysicsComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	template<class Callable>
	void ForEachChildNodeRecursive(SpatialNode3D* node, Callable&& callback)
	{
		if (node->children_mask == 0)
		{
			return;
		}

		for (SpatialNode3D* node : node->children_array)
		{
			if (node != nullptr)
			{
				callback(node);

				ForEachChildNodeRecursive(node, callback);
			}
		}
	}

	template<class Callable>
	void ForEachCoordInRegion(godot::Vector3i start, godot::Vector3i end, Callable&& callback)
	{
		if (end.x < start.x) std::swap(start.x, end.x);
		if (end.y < start.y) std::swap(start.y, end.y);
		if (end.z < start.z) std::swap(start.z, end.z);

		for (; start.x < end.x; start.x++)
		{
			for (; start.y < end.x; start.y++)
			{
				for (; start.z < end.x; start.z++)
				{
					callback(start);
				}
			}
		}
	}

	template<class Callable>
	void ForEachCoordInSphere(godot::Vector3 pos, double radius, Callable&& callback)
	{
		godot::Vector3i start = start - godot::Vector3i(radius, radius, radius);
		godot::Vector3i end = end + godot::Vector3i(radius, radius, radius);

		for (; start.x < end.x; start.x++)
		{
			for (; start.y < end.x; start.y++)
			{
				for (; start.z < end.x; start.z++)
				{
					if (pos.distance_squared_to(start) < radius)
					{
						callback(start);
					}
				}
			}
		}
	}

	SpatialModule::SpatialModule(flecs::world& world)
	{
		world.module<SpatialModule>("SpatialModule");

		AddComponents(world);
		AddRelationships(world);
		AddObservers(world);
		AddPhases(world);
		AddSystems(world);
	}

	void SpatialModule::AddComponents(flecs::world& world)
	{
		world.component<WorldTime>();
		world.component<SpatialEntity3DComponent>();
		world.component<SpatialScale3DComponent>();
		world.component<SpatialWorld3DComponent>();
		world.component<SpatialCommands3DComponent>();
		world.component<SpatialScaleThread3DComponent>();
		world.component<SpatialRegionThread3DComponent>();
		world.component<SpatialNodeThread3DComponent>();
		world.component<SpatialLoader3DComponent>();
	}

	void SpatialModule::AddRelationships(flecs::world& world)
	{
		world.component<SpatialEntity3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.component<SpatialScale3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf);
		world.component<SpatialScaleThread3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf).add_second<SpatialCommands3DComponent>(flecs::With);
		world.component<SpatialRegionThread3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf).add_second<SpatialCommands3DComponent>(flecs::With);
		world.component<SpatialNodeThread3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf).add_second<SpatialCommands3DComponent>(flecs::With);
		world.component<SpatialLoader3DComponent>().add_second<SpatialCommands3DComponent>(flecs::With).add_second<Position3DComponent>(flecs::With).add_second<SpatialScale3DComponent>(flecs::With);
	}

	void SpatialModule::AddObservers(flecs::world& world)
	{
		world.observer<SpatialWorld3DComponent>()
			.event(flecs::OnSet)
			.each([](flecs::entity entity, SpatialWorld3DComponent& spatial_world)
		{
			spatial_world.commands_query = entity.world().query_builder<SpatialCommands3DComponent>()
				.term(flecs::ChildOf, entity)
				.build();
		});

		world.observer<SpatialWorld3DComponent>()
			.event(flecs::UnSet)
			.each([](flecs::entity entity, SpatialWorld3DComponent& spatial_world)
		{
			spatial_world.commands_query.destruct();
		});
	}

	void SpatialModule::AddPhases(flecs::world& world)
	{
		world.entity<WorldLoaderProgressPhase>().add(flecs::Phase).depends_on(flecs::OnUpdate);
		world.entity<WorldNodeProgressPhase>().add(flecs::Phase).depends_on<WorldLoaderProgressPhase>();
		world.entity<WorldRegionProgressPhase>().add(flecs::Phase).depends_on<WorldNodeProgressPhase>();
		world.entity<WorldScaleProgressPhase>().add(flecs::Phase).depends_on<WorldRegionProgressPhase>();
		world.entity<WorldProgressPhase>().add(flecs::Phase).depends_on<WorldScaleProgressPhase>();
	}

	void SpatialModule::AddSystems(flecs::world& world)
	{
		AddSyncs(world);

		world.system<WorldTime>("WorldUpdateTime")
			.kind(flecs::OnUpdate)
			.term_at(1).src<WorldTime>()
			.iter([](flecs::iter& it, WorldTime* world_time)
		{
			world_time->frame_index++;
			world_time->frame_start = Clock::now();
		});

		world.system<const SpatialLoader3DComponent, SpatialCommands3DComponent, SpatialWorld3DComponent, const WorldTime>("SpatialWorldLoaderUpdateNodes")
			.multi_threaded()
			.kind<WorldLoaderProgressPhase>()
			.term_at(3).parent()
			.term_at(4).src<WorldTime>()
			.each([](const SpatialLoader3DComponent& spatial_loader, SpatialCommands3DComponent& spatial_commands, SpatialWorld3DComponent& world, const WorldTime& world_time)
		{
			for (size_t scale_index = spatial_loader.min_lod; scale_index < spatial_loader.max_lod; scale_index++)
			{
				const SpatialScale3D& spatial_scale = world.world->scales[scale_index];

				ForEachCoordInSphere(spatial_loader.coord.pos, spatial_loader.dist_per_lod, [scale_index, &spatial_scale, &spatial_commands, &world_time](godot::Vector3i pos)
				{
					auto it = spatial_scale.nodes.find(pos);

					if (it != spatial_scale.nodes.end())
					{
						it->second->last_update_time = world_time.frame_start;
					}
					else
					{
						spatial_commands.scales[scale_index].nodes_load.push_back(pos);
					}
				});
			}
		});

		world.system<const SpatialScaleThread3DComponent, SpatialWorld3DComponent, const WorldTime>("SpatialWorldUnloadUnusedNodes")
			.multi_threaded()
			.kind<WorldScaleProgressPhase>()
			.term_at(2).parent()
			.term_at(3).src<WorldTime>()
			.each([](const SpatialScaleThread3DComponent& spatial_world_scale, SpatialWorld3DComponent& spatial_world, const WorldTime& world_time)
		{
			SpatialScale3D& scale = spatial_world.world->scales[spatial_world_scale.scale];

			for (auto&& [coord, node] : scale.nodes)
			{
				if (world_time.frame_start - node->last_update_time > 20s)
				{
					scale.nodes.erase(coord);
				}
			}
		});

		// Apply commands multithreaded per world
		world.system<SpatialWorld3DComponent>("SpatialWorldApplyCommands")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.each([](flecs::entity entity, SpatialWorld3DComponent& spatial_world)
		{
			auto stage_command_query = flecs::query<SpatialCommands3DComponent>(entity.world(), spatial_world.commands_query);

			// For each command list that is a child of the world
			stage_command_query.each([&spatial_world](SpatialCommands3DComponent& spatial_commands)
			{
				// Commands for each scale
				for (size_t scale_index = 0; scale_index < k_max_world_scale; scale_index++)
				{
					SpatialCommands3D& commands = spatial_commands.scales[scale_index];
					SpatialScale3D& scale = spatial_world.world->scales[scale_index];

					for (godot::Vector3i& pos : commands.nodes_load)
					{
						scale.nodes.try_emplace(pos);
					}

					for (godot::Vector3i& pos : commands.nodes_unload)
					{
						scale.nodes.erase(pos);
					}

					for (godot::Vector3i& coord : commands.nodes_changed)
					{

					}

					commands.nodes_load.clear();
					commands.nodes_unload.clear();
					commands.nodes_changed.clear();
				}
			});
		});
	}

	void SpatialModule::AddSyncs(flecs::world& world)
	{
		world.system("WorldNodeProgressSync")
			.no_readonly()
			.kind<WorldNodeProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system("WorldRegionProgressSync")
			.no_readonly()
			.kind<WorldRegionProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system("WorldScaleProgressSync")
			.no_readonly()
			.kind<WorldScaleProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system("WorldProgressSync")
			.no_readonly()
			.kind<WorldProgressPhase>()
			.iter([](flecs::iter& it) {});
	}

	void SpatialModule::AddExamples(flecs::world& world)
	{
		world.system<const SpatialLoader3DComponent, SpatialWorld3DComponent>("SpatialWorldLoaderExample")
			.multi_threaded()
			.kind<WorldLoaderProgressPhase>()
			.term_at(2).parent()
			.each([](const SpatialLoader3DComponent& spatial_loader, SpatialWorld3DComponent& world)
		{
			for (size_t scale_index = spatial_loader.min_lod; scale_index < spatial_loader.max_lod; scale_index++)
			{
				const SpatialScale3D& spatial_scale = world.world->scales[scale_index];

				ForEachCoordInSphere(spatial_loader.coord.pos, spatial_loader.dist_per_lod, [scale_index, &spatial_scale](godot::Vector3i pos)
				{
					auto it = spatial_scale.nodes.find(pos);

					if (it != spatial_scale.nodes.end())
					{
						it->second;
					}
				});
			}
		});

		world.system<const SpatialNodeThread3DComponent, SpatialWorld3DComponent>("SpatialWorldNodeExample")
			.multi_threaded()
			.kind<WorldNodeProgressPhase>()
			.term_at(2).parent()
			.each([](const SpatialNodeThread3DComponent& spatial_world_node, SpatialWorld3DComponent& spatial_world)
		{
			SpatialScale3D& scale = spatial_world.world->scales[spatial_world_node.node.scale];

			auto it = scale.nodes.find(spatial_world_node.node.pos);

			if (it != scale.nodes.end())
			{
				SpatialNode3D* node = it->second;

				ForEachChildNodeRecursive(node, [](SpatialNode3D* node)
				{
					node;
				});
			}
		});

		world.system<const SpatialRegionThread3DComponent, SpatialWorld3DComponent>("SpatialWorldRegionExample")
			.multi_threaded()
			.kind<WorldRegionProgressPhase>()
			.term_at(2).parent()
			.each([](const SpatialRegionThread3DComponent& spatial_world_region, SpatialWorld3DComponent& spatial_world)
		{
			uint32_t scale_index = spatial_world_region.region.scale;
			godot::Vector3i start = spatial_world_region.region.pos;
			godot::Vector3i end = spatial_world_region.region.pos + spatial_world_region.region.size;

			SpatialScale3D& scale = spatial_world.world->scales[scale_index];

			ForEachCoordInRegion(start, end, [&scale](godot::Vector3i pos)
			{
				SpatialNode3D* node = scale.nodes[pos];

				ForEachChildNodeRecursive(node, [](SpatialNode3D* node)
				{
					node;
				});
			});
		});

		world.system<const SpatialScaleThread3DComponent, SpatialWorld3DComponent>("SpatialWorldScaleExample")
			.multi_threaded()
			.kind<WorldScaleProgressPhase>()
			.term_at(2).parent()
			.each([](const SpatialScaleThread3DComponent& spatial_world_scale, SpatialWorld3DComponent& spatial_world)
		{
			SpatialScale3D& scale = spatial_world.world->scales[spatial_world_scale.scale];

			for (auto&& [pos, node] : scale.nodes)
			{
				node;
			}
		});

		world.system<SpatialWorld3DComponent>("SpatialWorldExample")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.each([](SpatialWorld3DComponent& spatial_world)
		{
			for (SpatialScale3D& scale : spatial_world.world->scales)
			{
				for (auto&& [pos, node] : scale.nodes)
				{
					node;
				}
			}
		});
	}

	SpatialNode3D* SpatialModule::GetNode(const SpatialWorld3D& world, SpatialCoord3D coord)
	{
		DEBUG_ASSERT(coord.scale < k_max_world_scale, "The coordinates scale is out of range");

		auto& nodes = world.scales[coord.scale].nodes;

		auto it = nodes.find(coord.pos);

		if (it == nodes.end())
		{
			return nullptr;
		}

		return it->second;
	}
}