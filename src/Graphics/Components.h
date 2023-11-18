#pragma once

#include <godot_cpp/variant/rid.hpp>

#include <entt/fwd.hpp>

struct RenderingInstanceComponent
{
	godot::RID instance_id;
};

struct InstanceChangedComponent {};

struct MeshComponent
{
	godot::RID mesh_id;
};