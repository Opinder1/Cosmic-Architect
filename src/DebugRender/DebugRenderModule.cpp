#include "DebugRenderModule.h"

#include "UniverseSimulation.h"

#include "Render/RenderComponents.h"
#include "Spatial3D/SpatialComponents.h"

#include "Spatial3D/SpatialWorld.h"

#include "Render/RenderModule.h"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/world3d.hpp>

namespace voxel_game::debugrender
{
	struct DebugWorld
	{
		godot::RID mesh_rid;
		godot::RID multimesh_rid;
		godot::RID instance_rid;

		entity::Ref entity;

		size_t buffer_size = 0;
	};
	
	static std::unique_ptr<DebugWorld> debug_world;

	godot::Array CreateLineBoxMesh()
	{
		godot::PackedVector3Array vertexes;

		vertexes.push_back({ 0, 0, 0 });
		vertexes.push_back({ 0, 0, 1 });

		vertexes.push_back({ 0, 0, 1 });
		vertexes.push_back({ 0, 1, 1 });

		vertexes.push_back({ 0, 1, 1 });
		vertexes.push_back({ 0, 1, 0 });

		vertexes.push_back({ 0, 1, 0 });
		vertexes.push_back({ 0, 0, 0 });

		// Square 2
		vertexes.push_back({ 1, 0, 0 });
		vertexes.push_back({ 1, 0, 1 });

		vertexes.push_back({ 1, 0, 1 });
		vertexes.push_back({ 1, 1, 1 });

		vertexes.push_back({ 1, 1, 1 });
		vertexes.push_back({ 1, 1, 0 });

		vertexes.push_back({ 1, 1, 0 });
		vertexes.push_back({ 1, 0, 0 });

		// Connectors
		vertexes.push_back({ 0, 0, 0 });
		vertexes.push_back({ 1, 0, 0 });

		vertexes.push_back({ 0, 0, 1 });
		vertexes.push_back({ 1, 0, 1 });

		vertexes.push_back({ 0, 1, 1 });
		vertexes.push_back({ 1, 1, 1 });

		vertexes.push_back({ 0, 1, 0 });
		vertexes.push_back({ 1, 1, 0 });

		godot::Array arrays;
		arrays.resize(RS::ARRAY_MAX);

		arrays[RS::ARRAY_VERTEX] = vertexes;

		return arrays;
	}

	void Initialize(Simulation& simulation)
	{
		debug_world = std::make_unique<DebugWorld>();

		debug_world->mesh_rid = rendering::AllocRID(rendering::RIDType::Mesh);
		debug_world->multimesh_rid = rendering::AllocRID(rendering::RIDType::MultiMesh);
		debug_world->instance_rid = rendering::AllocRID(rendering::RIDType::Instance);

		rendering::AddCommand<&RS::mesh_add_surface_from_arrays>(debug_world->mesh_rid, RS::PRIMITIVE_LINES, CreateLineBoxMesh(), godot::Array{}, godot::Dictionary{}, 0);

		rendering::AddCommand<&RS::multimesh_set_mesh>(debug_world->multimesh_rid, debug_world->mesh_rid);

		rendering::AddCommand<&RS::instance_set_base>(debug_world->instance_rid, debug_world->multimesh_rid);
	}

	void Uninitialize(Simulation& simulation)
	{
		rendering::AddCommand<&RS::free_rid>(debug_world->mesh_rid);
		rendering::AddCommand<&RS::free_rid>(debug_world->multimesh_rid);
		rendering::AddCommand<&RS::free_rid>(debug_world->instance_rid);
		debug_world->entity = entity::Ref();
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

		// Allocate new data if we run out of buffer space
		if (debug_world->buffer_size < node_count)
		{
			if (debug_world->buffer_size == 0)
			{
				debug_world->buffer_size = 1;
			}

			do
			{
				debug_world->buffer_size *= 2;
			} while (debug_world->buffer_size < node_count);

			rendering::AddCommand<&RS::multimesh_allocate_data>(debug_world->multimesh_rid, debug_world->buffer_size, RS::MULTIMESH_TRANSFORM_3D, false, false, false);
		}

		if (node_count > 0)
		{
			instance_data.resize(debug_world->buffer_size * 12);

			rendering::AddCommand<&RS::multimesh_set_visible_instances>(debug_world->multimesh_rid, node_count);

			rendering::AddCommand<&RS::multimesh_set_buffer>(debug_world->multimesh_rid, instance_data);
		}
	}

	void Update(Simulation& simulation)
	{
		if (simulation.frame_index % 100 == 0)
		{
			if (!debug_world->entity && simulation.universes.size() > 0)
			{
				debug_world->entity = simulation.universes[0].Reference();
			}

			if (debug_world->entity)
			{
				godot::Transform3D transform;

				if (debug_world->entity.Has<physics3d::CPosition>())
				{
					const godot::Vector3& position = debug_world->entity->*&physics3d::CPosition::position;

					transform.set_origin(position);
				}

				if (debug_world->entity.Has<physics3d::CRotation>())
				{
					const godot::Quaternion& rotation = debug_world->entity->*&physics3d::CRotation::rotation;

					transform.rotate(rotation.get_axis(), rotation.get_angle());
				}

				if (debug_world->entity.Has<physics3d::CScale>())
				{
					const godot::Vector3& scale = debug_world->entity->*&physics3d::CScale::scale;

					transform.scale(scale);
				}

				rendering::AddCommand<&RS::instance_set_transform>(debug_world->instance_rid, transform);

				rendering::AddCommand<&RS::instance_set_scenario>(debug_world->instance_rid, simulation.universes[0]->*&rendering::CScenario::id);

				BuildWorldDebugVisualization(debug_world->entity->*&spatial3d::CWorld::world);
			}
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