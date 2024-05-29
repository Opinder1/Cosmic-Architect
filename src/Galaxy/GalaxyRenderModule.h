#pragma once

#include "Util/Hash.h"
#include "Util/Nocopy.h"

#include <godot_cpp/templates/rid_owner.hpp>

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/quaternion.hpp>

#include <robin_hood/robin_hood.h>

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct GalaxyRenderComponent
	{
		godot::RID id;
		godot::Vector3i mesh_pos;
	};

	struct GalaxyRenderData
	{
		godot::Vector3 position;
		godot::Quaternion rotation;
		uint32_t texture_index = 0;
	};

	struct GalaxyRenderMesh : Nocopy, Nomove
	{
		godot::RID instance;
		godot::RID multimesh;

		uint32_t allocated = 0;
		uint32_t dirty = false;

		godot::RID_Owner<GalaxyRenderData> render_data;
	};

	struct GalaxyRenderContext : Nocopy
	{
		godot::RID scenario;

		godot::RID mesh;
		
		robin_hood::unordered_map<godot::Vector3i, std::unique_ptr<GalaxyRenderMesh>> node_meshes;
	};

	struct GalaxyRenderModule
	{
		GalaxyRenderModule(flecs::world& world);
	};
}