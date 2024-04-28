#include "SpatialModule.h"
#include "SpatialComponents.h"

#include "Physics/PhysicsComponents.h"
#include "Simulation/SimulationComponents.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	// Run a callback for a node and all of its children
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

	// Run a callback for all the nodes in a cuboid region
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

	// Run a callback for all the nodes in a sphere
	template<class Callable>
	void ForEachCoordInSphere(godot::Vector3 pos, double radius, Callable&& callback)
	{
		godot::Vector3i start = pos - godot::Vector3i(radius, radius, radius);
		godot::Vector3i end = pos + godot::Vector3i(radius, radius, radius);

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

	// Wrapper to run world loaders in different threads
	template<class... Args>
	struct SpatialWorldLoaderSystem
	{
		using Callback = cb::Callback<void(const SpatialLoader3DComponent&, SpatialWorld3DComponent&, SpatialNode3D*, Args...)>;

		SpatialWorldLoaderSystem(const Callback& callback) :
			callback(callback)
		{}

		void operator()(const SpatialLoader3DComponent& spatial_loader, SpatialWorld3DComponent& spatial_world, Args&&... args) const
		{
			for (size_t scale_index = spatial_loader.min_lod; scale_index < spatial_loader.max_lod; scale_index++)
			{
				const SpatialScale3D& spatial_scale = spatial_world.world.scales[scale_index];

				ForEachCoordInSphere(spatial_loader.coord.pos, spatial_loader.dist_per_lod, [&, scale_index](godot::Vector3i pos)
				{
					auto it = spatial_scale.nodes.find(pos);

					if (it != spatial_scale.nodes.end())
					{
						callback(spatial_loader, spatial_world, it->second, args...);
					}
					else
					{
						callback(spatial_loader, spatial_world, nullptr, args...);
					}
				});
			}
		}

		Callback callback;
	};

	template<class... Args>
	struct SpatialWorldRegionSystem
	{
		using Callback = cb::Callback<void(const SpatialRegion3DComponent&, SpatialWorld3DComponent&, SpatialNode3D*, Args...)>;

		SpatialWorldRegionSystem(const Callback& callback) :
			callback(callback)
		{}

		void operator()(const SpatialRegion3DComponent& spatial_world_region, SpatialWorld3DComponent& spatial_world, Args&&... args) const
		{
			uint32_t scale_index = spatial_world_region.region.scale;
			godot::Vector3i start = spatial_world_region.region.pos;
			godot::Vector3i end = spatial_world_region.region.pos + spatial_world_region.region.size;

			SpatialScale3D& scale = spatial_world.world.scales[scale_index];

			ForEachCoordInRegion(start, end, [&](godot::Vector3i pos)
			{
				SpatialNode3D* root_node = scale.nodes[pos];

				ForEachChildNodeRecursive(root_node, [&](SpatialNode3D* node)
				{
					callback(spatial_world_region, spatial_world, node, args...);
				});
			});
		}

		Callback callback;
	};

	template<class... Args>
	struct SpatialWorldNodeSystem
	{
		using Callback = cb::Callback<void(const SpatialNode3DComponent&, SpatialWorld3DComponent&, SpatialNode3D*, Args...)>;

		SpatialWorldNodeSystem(const Callback& callback) :
			callback(callback)
		{}

		void operator()(const SpatialNode3DComponent& spatial_world_node, SpatialWorld3DComponent& spatial_world, Args&&... args) const
		{
			SpatialScale3D& scale = spatial_world.world.scales[spatial_world_node.node.scale];

			auto it = scale.nodes.find(spatial_world_node.node.pos);

			if (it == scale.nodes.end())
			{
				DEBUG_PRINT_WARN("This spatial node components node is not loaded");
				return;
			}

			SpatialNode3D* node = it->second;

			ForEachChildNodeRecursive(node, [&](SpatialNode3D* node)
			{
				callback(spatial_world_node, spatial_world, node, args...);
			});
		}

		Callback callback;
	};

	template<class... Args>
	struct SpatialWorldScaleSystem
	{
		using Callback = cb::Callback<void(const SpatialScale3DComponent&, SpatialWorld3DComponent&, SpatialNode3D*, Args...)>;

		SpatialWorldScaleSystem(const Callback& callback) :
			callback(callback)
		{}

		void operator()(const SpatialScale3DComponent& spatial_world_scale, SpatialWorld3DComponent& spatial_world, Args&&... args) const
		{
			SpatialScale3D& scale = spatial_world.world.scales[spatial_world_scale.scale];

			for (auto&& [pos, node] : scale.nodes)
			{
				callback(spatial_world_scale, spatial_world, node, args...);
			}
		}

		Callback callback;
	};

	template<class... Args>
	struct SpatialWorldSystem
	{
		using Callback = cb::Callback<void(SpatialWorld3DComponent&, SpatialNode3D*, Args...)>;

		SpatialWorldSystem(const Callback& callback) :
			callback(callback)
		{}

		void operator()(SpatialWorld3DComponent& spatial_world, Args&&... args) const
		{
			for (SpatialScale3D& scale : spatial_world.world.scales)
			{
				for (auto&& [pos, node] : scale.nodes)
				{
					callback(spatial_world, node, args...);
				}
			}
		}

		Callback callback;
	};

	// System to keep alive all nodes around a loader and request the loading of any missing
	void SpatialWorldLoaderUpdateNodes(const SpatialLoader3DComponent& spatial_loader, SpatialWorld3DComponent& spatial_world, SpatialCommands3DComponent& spatial_commands, const SimulationGlobal& world_time)
	{
		for (size_t scale_index = spatial_loader.min_lod; scale_index < spatial_loader.max_lod; scale_index++)
		{
			const SpatialScale3D& spatial_scale = spatial_world.world.scales[scale_index];

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
	}

	// System to erase any nodes that are no longer being observed by any loader
	void SpatialWorldUnloadUnusedNodes(const SpatialScale3DComponent& spatial_world_scale, SpatialWorld3DComponent& spatial_world, const SimulationGlobal& world_time)
	{
		SpatialScale3D& scale = spatial_world.world.scales[spatial_world_scale.scale];

		for (auto&& [coord, node] : scale.nodes)
		{
			if (world_time.frame_start - node->last_update_time > 20s)
			{
				scale.nodes.erase(coord);
			}
		}
	}

	// System to process all commands added for each node
	void SpatialWorldApplyCommands(flecs::entity entity, SpatialWorld3DComponent& spatial_world)
	{
		auto stage_command_query = flecs::query<SpatialCommands3DComponent>(entity.world(), spatial_world.commands_query);

		// For each command list that is a child of the world
		stage_command_query.each([&spatial_world](SpatialCommands3DComponent& spatial_commands)
		{
			// Commands for each scale
			for (size_t scale_index = 0; scale_index < k_max_world_scale; scale_index++)
			{
				SpatialScaleCommands& scale_commands = spatial_commands.scales[scale_index];
				SpatialScale3D& scale = spatial_world.world.scales[scale_index];

				for (godot::Vector3i& pos : scale_commands.nodes_load)
				{
					scale.nodes.try_emplace(pos, spatial_world.world.create_node());
				}

				for (godot::Vector3i& pos : scale_commands.nodes_unload)
				{
					auto it = scale.nodes.find(pos);

					spatial_world.world.destroy_node(it->second);

					scale.nodes.erase(it);
				}

				scale_commands.nodes_load.clear();
				scale_commands.nodes_unload.clear();
			}
		});
	}

	SpatialModule::SpatialModule(flecs::world& world)
	{
		world.module<SpatialModule>("SpatialModule");

		world.import<SpatialComponents>();
		world.import<PhysicsComponents>();
		world.import<SimulationComponents>();

		// Spatial world loader systems
		world.system("WorldLoaderProgressSync")
			.no_readonly()
			.kind<WorldLoaderProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system<const SpatialLoader3DComponent, SpatialWorld3DComponent>("SpatialWorldLoaderExample")
			.multi_threaded()
			.kind<WorldLoaderProgressPhase>()
			.term<SpatialThread3DComponent>()
			.term_at(2).parent()
			.each(SpatialWorldLoaderSystem({}));

		world.system<const SpatialLoader3DComponent, SpatialWorld3DComponent, SpatialCommands3DComponent, const SimulationGlobal>("SpatialWorldLoaderUpdateNodes")
			.multi_threaded()
			.kind<WorldLoaderProgressPhase>()
			.term_at(2).parent()
			.term_at(4).src<SimulationGlobal>()
			.each(SpatialWorldLoaderUpdateNodes);

		// Spatial world node systems
		world.system("WorldNodeProgressSync")
			.no_readonly()
			.kind<WorldNodeProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system<const SpatialNode3DComponent, SpatialWorld3DComponent>("SpatialWorldNodeExample")
			.multi_threaded()
			.kind<WorldNodeProgressPhase>()
			.term<SpatialThread3DComponent>()
			.term_at(2).parent()
			.each(SpatialWorldNodeSystem({}));

		// Spatial world region systems
		world.system("WorldRegionProgressSync")
			.no_readonly()
			.kind<WorldRegionProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system<const SpatialRegion3DComponent, SpatialWorld3DComponent>("SpatialWorldRegionExample")
			.multi_threaded()
			.kind<WorldRegionProgressPhase>()
			.term<SpatialThread3DComponent>()
			.term_at(2).parent()
			.each(SpatialWorldRegionSystem({}));

		// Spatial world scale systems
		world.system("WorldScaleProgressSync")
			.no_readonly()
			.kind<WorldScaleProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system<const SpatialScale3DComponent, SpatialWorld3DComponent>("SpatialWorldScaleExample")
			.multi_threaded()
			.kind<WorldScaleProgressPhase>()
			.term<SpatialThread3DComponent>()
			.term_at(2).parent()
			.each(SpatialWorldScaleSystem({}));

		world.system<const SpatialScale3DComponent, SpatialWorld3DComponent, const SimulationGlobal>("SpatialWorldUnloadUnusedNodes")
			.multi_threaded()
			.kind<WorldScaleProgressPhase>()
			.term<SpatialThread3DComponent>()
			.term_at(2).parent()
			.term_at(3).src<SimulationGlobal>()
			.each(SpatialWorldUnloadUnusedNodes);

		// Spatial world scale
		world.system("WorldProgressSync")
			.no_readonly()
			.kind<WorldProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system<SpatialWorld3DComponent>("SpatialWorldExample")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.each(SpatialWorldSystem({}));

		world.system<SpatialWorld3DComponent>("SpatialWorldApplyCommands")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.each(SpatialWorldApplyCommands);
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