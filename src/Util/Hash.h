#pragma once

#include "UUID.h"

#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/vector4i.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/rect2i.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/variant/projection.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include <robin_hood/robin_hood.h>

template<class T>
struct ByteHash
{
    size_t operator()(const T& vector) const noexcept
    {
        return robin_hood::hash_bytes(&vec, sizeof(T));
    }
};

struct UUIDHash
{
    size_t operator()(const UUID&) const;
};

namespace robin_hood
{
    template<>
    struct hash<godot::Color>
    {
        size_t operator()(const godot::Color& color) const noexcept;
    };

    template<>
    struct hash<godot::AABB>
    {
        size_t operator()(const godot::AABB& aabb) const noexcept;
    };

    template<>
    struct hash<godot::Basis>
    {
        size_t operator()(const godot::Basis& basis) const noexcept;
    };

    template<>
    struct hash<godot::Vector2>
    {
        size_t operator()(const godot::Vector2& vector) const noexcept;
    };

    template<>
    struct hash<godot::Vector2i>
    {
        size_t operator()(const godot::Vector2i& vector) const noexcept;
    };

    template<>
    struct hash<godot::Vector3>
    {
        size_t operator()(const godot::Vector3& vector) const noexcept;
    };

    template<>
    struct hash<godot::Vector3i>
    {
        size_t operator()(const godot::Vector3i& vector) const noexcept;
    };

    template<>
    struct hash<godot::Vector4>
    {
        size_t operator()(const godot::Vector4& vector) const noexcept;
    };

    template<>
    struct hash<godot::Vector4i>
    {
        size_t operator()(const godot::Vector4i& vector) const noexcept;
    };

    template<>
    struct hash<godot::Rect2>
    {
        size_t operator()(const godot::Rect2& rect) const noexcept;
    };

    template<>
    struct hash<godot::Rect2i>
    {
        size_t operator()(const godot::Rect2i& rect) const noexcept;
    };

    template<>
    struct hash<godot::Transform2D>
    {
        size_t operator()(const godot::Transform2D& transform) const noexcept;
    };

    template<>
    struct hash<godot::Transform3D>
    {
        size_t operator()(const godot::Transform3D& transform) const noexcept;
    };

    template<>
    struct hash<godot::Plane>
    {
        size_t operator()(const godot::Plane& plane) const noexcept;
    };

    template<>
    struct hash<godot::Projection>
    {
        size_t operator()(const godot::Projection& projection) const noexcept;
    };

    template<>
    struct hash<godot::Quaternion>
    {
        size_t operator()(const godot::Quaternion& quaternion) const noexcept;
    };

    template<>
    struct hash<godot::RID>
    {
        size_t operator()(const godot::RID& rid) const noexcept;
    };

    template<>
    struct hash<godot::String>
    {
        size_t operator()(const godot::String& string) const noexcept;
    };

    template<>
    struct hash<godot::StringName>
    {
        size_t operator()(const godot::StringName& string) const noexcept;
    };
}