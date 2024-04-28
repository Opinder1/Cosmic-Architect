#pragma once

#include "Spatial.h"
#include "SpatialComponents.h"
#include "SpatialTraverse.h"

#include "Util/Callback.h"
#include "Util/Debug.h"

namespace voxel_game
{
	// Wrapper to run world loaders in different threads
	template<class... Args>
	struct SpatialWorldLoaderSystem
	{
		using Callback = cb::Callback<void(SpatialCoord3D, SpatialNode3D*, SpatialWorld3DComponent&, const SpatialLoader3DComponent&, Args...)>;

		SpatialWorldLoaderSystem(const Callback& callback) :
			callback(callback)
		{}

		void operator()(SpatialWorld3DComponent& spatial_world, const SpatialLoader3DComponent& spatial_loader, Args&&... args) const
		{
			for (uint8_t scale_index = spatial_loader.min_lod; scale_index < spatial_loader.max_lod; scale_index++)
			{
				const SpatialScale3D& spatial_scale = spatial_world.world.scales[scale_index];

				ForEachCoordInSphere(spatial_loader.coord.pos, spatial_loader.dist_per_lod, [&, scale_index](godot::Vector3i pos)
				{
					auto it = spatial_scale.nodes.find(pos);

					if (it != spatial_scale.nodes.end())
					{
						callback(SpatialCoord3D{ it->first, scale_index }, it->second, spatial_world, spatial_loader, args...);
					}
					else
					{
						callback(SpatialCoord3D{}, nullptr, spatial_world, spatial_loader, args...);
					}
				});
			}
		}

		Callback callback;
	};

	template<class... Args>
	struct SpatialWorldRegionSystem
	{
		using Callback = cb::Callback<void(SpatialNode3D*, SpatialWorld3DComponent&, const SpatialRegion3DComponent&, Args...)>;

		SpatialWorldRegionSystem(const Callback& callback) :
			callback(callback)
		{}

		void operator()(SpatialWorld3DComponent& spatial_world, const SpatialRegion3DComponent& spatial_world_region, Args&&... args) const
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
					callback(node, spatial_world, spatial_world_region, args...);
				});
			});
		}

		Callback callback;
	};

	template<class... Args>
	struct SpatialWorldNodeSystem
	{
		using Callback = cb::Callback<void(SpatialNode3D*, SpatialWorld3DComponent&, const SpatialNode3DComponent&, Args...)>;

		SpatialWorldNodeSystem(const Callback& callback) :
			callback(callback)
		{}

		void operator()(SpatialWorld3DComponent& spatial_world, const SpatialNode3DComponent& spatial_world_node, Args&&... args) const
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
				callback(node, spatial_world, spatial_world_node, args...);
			});
		}

		Callback callback;
	};

	template<class... Args>
	struct SpatialWorldScaleSystem
	{
		using Callback = cb::Callback<void(SpatialNode3D*, SpatialWorld3DComponent&, const SpatialScale3DComponent&, Args...)>;

		SpatialWorldScaleSystem(const Callback& callback) :
			callback(callback)
		{}

		void operator()(SpatialWorld3DComponent& spatial_world, const SpatialScale3DComponent& spatial_world_scale, Args&&... args) const
		{
			SpatialScale3D& scale = spatial_world.world.scales[spatial_world_scale.scale];

			for (auto&& [pos, node] : scale.nodes)
			{
				callback(node, spatial_world, spatial_world_scale, args...);
			}
		}

		Callback callback;
	};

	template<class... Args>
	struct SpatialWorldSystem
	{
		using Callback = cb::Callback<void(SpatialNode3D*, SpatialWorld3DComponent&, Args...)>;

		SpatialWorldSystem(const Callback& callback) :
			callback(callback)
		{}

		void operator()(SpatialWorld3DComponent& spatial_world, Args&&... args) const
		{
			for (SpatialScale3D& scale : spatial_world.world.scales)
			{
				for (auto&& [pos, node] : scale.nodes)
				{
					callback(node, spatial_world, args...);
				}
			}
		}

		Callback callback;
	};
}