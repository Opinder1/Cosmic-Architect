#pragma once

#include <godot_cpp/variant/vector3.hpp>

#include <godot_cpp/variant/transform3d.hpp>

#include <entt/entity/entity.hpp>

#include <bitset>

struct PositionComponent
{
	godot::Vector3 position;
};

struct Transform3DComponent
{
	godot::Transform3D transform;
};

struct TreeComponent
{
    std::size_t children = 0;
    entt::entity first = entt::null;
    entt::entity prev = entt::null;
    entt::entity next = entt::null;
    entt::entity parent =  entt::null;
};

// This is the lod that this component exists at
struct LODComponent
{
    uint32_t lod = 0;
};

// This is how persistent this component is in keeping itself loaded or being saved to file
struct PersistentComponent
{
    uint32_t persistence = 0;
};