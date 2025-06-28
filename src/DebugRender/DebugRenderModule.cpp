#include "DebugRenderModule.h"

#include "UniverseSimulation.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialWorld.h"

#include "Render/RenderContext.h"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>

namespace voxel_game::debugrender
{
	struct DebugWorld
	{
		godot::RID mesh_rid;
		godot::RID multimesh_rid;
		godot::RID instance_rid;

		size_t buffer_size = 0;
	};
	
	static std::unique_ptr<DebugWorld> debug_world;

	void Initialize(Simulation& simulation)
	{
		debug_world = std::make_unique<DebugWorld>();

		debug_world->mesh_rid = rendering::AllocRID(rendering::RIDType::Mesh);
		debug_world->multimesh_rid = rendering::AllocRID(rendering::RIDType::MultiMesh);
		debug_world->instance_rid = rendering::AllocRID(rendering::RIDType::Instance);

		godot::PackedVector3Array vertexes;

		vertexes.push_back({ 0, 0, 0 });
		vertexes.push_back({ 0, 0, 1 });

		vertexes.push_back({ 0, 0, 1 });
		vertexes.push_back({ 0, 1, 1 });

		vertexes.push_back({ 0, 1, 1 });
		vertexes.push_back({ 0, 1, 0 });

		vertexes.push_back({ 0, 1, 0 });
		vertexes.push_back({ 0, 0, 0 });

		godot::Array arrays;
		arrays.resize(RS::ARRAY_MAX);

		arrays[RS::ARRAY_VERTEX] = vertexes;

		rendering::AddCommand<&RS::mesh_add_surface_from_arrays>(debug_world->mesh_rid, RS::PRIMITIVE_LINES, arrays, godot::Array{}, godot::Dictionary{}, 0);

		rendering::AddCommand<&RS::multimesh_set_mesh>(debug_world->multimesh_rid, debug_world->mesh_rid);

		rendering::AddCommand<&RS::instance_set_base>(debug_world->instance_rid, debug_world->mesh_rid);
		godot::SceneTree* scene_tree = static_cast<godot::SceneTree*>(godot::Engine::get_singleton()->get_main_loop());
		rendering::AddCommand<&RS::instance_set_scenario>(debug_world->instance_rid, scene_tree->get_root()->get_viewport_rid());
	}

	void Uninitialize(Simulation& simulation)
	{

	}

	bool IsUnloadDone(Simulation& simulation)
	{
		return true;
	}

	void BuildWorldDebugVisualization(spatial3d::WorldPtr world)
	{
		size_t node_count = 0;

		godot::PackedFloat32Array instance_data;

		uint64_t node_size = world->*&spatial3d::World::node_size;

		spatial3d::WorldForEachScale(world, [&](spatial3d::ScalePtr scale)
		{
			int64_t scale_mod = (uint64_t(1) << scale->*&spatial3d::Scale::index) * node_size;

			// For each create command
			for (auto&& [pos, node] : scale->*&spatial3d::Scale::nodes)
			{
				if (!node || node->*&spatial3d::Node::children_mask != 0)
				{
					continue;
				}

				// Push transform
				instance_data.push_back(scale_mod);
				instance_data.push_back(0);
				instance_data.push_back(0);
				instance_data.push_back(pos.x * scale_mod);

				instance_data.push_back(0);
				instance_data.push_back(scale_mod);
				instance_data.push_back(0);
				instance_data.push_back(pos.y * scale_mod);

				instance_data.push_back(0);
				instance_data.push_back(0);
				instance_data.push_back(scale_mod);
				instance_data.push_back(pos.z * scale_mod);

				node_count++;
			}
		});

		if (debug_world->buffer_size < node_count)
		{
			do
			{
				debug_world->buffer_size *= 2;
			} while (debug_world->buffer_size < node_count);

			rendering::AddCommand<&RS::multimesh_allocate_data>(debug_world->multimesh_rid, debug_world->buffer_size, RS::MULTIMESH_TRANSFORM_3D, false, false, false);
		}

		rendering::AddCommand<&RS::multimesh_set_visible_instances>(debug_world->multimesh_rid, node_count);
		rendering::AddCommand<&RS::multimesh_set_buffer>(debug_world->multimesh_rid, instance_data);
	}

	void Update(Simulation& simulation)
	{
		if (simulation.universes.size() > 0)
		{
			BuildWorldDebugVisualization(simulation.universes[0]->*&spatial3d::CWorld::world);
		}
	}

	void WorkerUpdate(Simulation& simulation, size_t index)
	{

	}

	void WorldUpdate(Simulation& simulation, spatial3d::WorldPtr world)
	{

	}

	void ScaleUpdate(Simulation& simulation, spatial3d::ScalePtr scale)
	{

	}
}