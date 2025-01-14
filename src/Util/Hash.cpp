#include "Hash.h"

namespace
{
	void HashCombine(size_t& seed, size_t hash)
	{
		hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= hash;
	}
}

size_t UUIDHash::operator()(const UUID& uuid) const
{
    static_assert(sizeof(UUID) == sizeof(uint64_t) * 2);

    uint64_t* arr = (uint64_t*)&uuid;

    return arr[0] ^ arr[1];
}

namespace robin_hood
{
    size_t hash<godot::Color>::operator()(const godot::Color& color) const noexcept
    {
        size_t seed = 0;
        hash<float> hasher;

        HashCombine(seed, hasher(color.r));
        HashCombine(seed, hasher(color.g));
        HashCombine(seed, hasher(color.b));
        HashCombine(seed, hasher(color.a));

        return seed;
    }

    size_t hash<godot::AABB>::operator()(const godot::AABB& aabb) const noexcept
    {
        size_t seed = 0;
        hash<godot::Vector3> hasher;

        HashCombine(seed, hasher(aabb.position));
        HashCombine(seed, hasher(aabb.size));

        return seed;
    }

    size_t hash<godot::Basis>::operator()(const godot::Basis& basis) const noexcept
    {
        size_t seed = 0;
        hash<godot::Vector3> hasher;

        HashCombine(seed, hasher(basis.rows[0]));
        HashCombine(seed, hasher(basis.rows[1]));
        HashCombine(seed, hasher(basis.rows[2]));

        return seed;
    };

    size_t hash<godot::Vector2>::operator()(const godot::Vector2& vector) const noexcept
    {
        size_t seed = 0;
        hash<real_t> hasher;

        HashCombine(seed, hasher(vector.x));
        HashCombine(seed, hasher(vector.y));

        return seed;
    }

    size_t hash<godot::Vector2i>::operator()(const godot::Vector2i& vector) const noexcept
    {
        size_t seed = 0;
        hash<int32_t> hasher;

        HashCombine(seed, hasher(vector.x));
        HashCombine(seed, hasher(vector.y));

        return seed;
    }

    size_t hash<godot::Vector3>::operator()(const godot::Vector3& vector) const noexcept
    {
        size_t seed = 0;
        hash<real_t> hasher;

        HashCombine(seed, hasher(vector.x));
        HashCombine(seed, hasher(vector.y));
        HashCombine(seed, hasher(vector.z));

        return seed;
    }

    size_t hash<godot::Vector3i>::operator()(const godot::Vector3i& vector) const noexcept
    {
        size_t seed = 0;
        hash<int32_t> hasher;

        HashCombine(seed, hasher(vector.x));
        HashCombine(seed, hasher(vector.y));
        HashCombine(seed, hasher(vector.z));

        return seed;
    }

    size_t hash<godot::Vector4>::operator()(const godot::Vector4& vector) const noexcept
    {
        size_t seed = 0;
        hash<real_t> hasher;

        HashCombine(seed, hasher(vector.x));
        HashCombine(seed, hasher(vector.y));
        HashCombine(seed, hasher(vector.z));
        HashCombine(seed, hasher(vector.w));

        return seed;
    }

    size_t hash<godot::Vector4i>::operator()(const godot::Vector4i& vector) const noexcept
    {
        size_t seed = 0;
        hash<int32_t> hasher;

        HashCombine(seed, hasher(vector.x));
        HashCombine(seed, hasher(vector.y));
        HashCombine(seed, hasher(vector.z));
        HashCombine(seed, hasher(vector.w));

        return seed;
    }

    size_t hash<godot::Rect2>::operator()(const godot::Rect2& rect) const noexcept
    {
        size_t seed = 0;
        hash<godot::Vector2> hasher;

        HashCombine(seed, hasher(rect.position));
        HashCombine(seed, hasher(rect.size));

        return seed;
    }

    size_t hash<godot::Rect2i>::operator()(const godot::Rect2i& rect) const noexcept
    {
        size_t seed = 0;
        hash<godot::Vector2i> hasher;

        HashCombine(seed, hasher(rect.position));
        HashCombine(seed, hasher(rect.size));

        return seed;
    }

    size_t hash<godot::Transform2D>::operator()(const godot::Transform2D& transform) const noexcept
    {
        size_t seed = 0;
        hash<godot::Vector2> hasher;

        HashCombine(seed, hasher(transform.columns[0]));
        HashCombine(seed, hasher(transform.columns[1]));
        HashCombine(seed, hasher(transform.columns[2]));

        return seed;
    }

    size_t hash<godot::Transform3D>::operator()(const godot::Transform3D& transform) const noexcept
    {
        size_t seed = 0;

        HashCombine(seed, hash<godot::Basis>{}(transform.basis));
        HashCombine(seed, hash<godot::Vector3>{}(transform.origin));

        return seed;
    }

    size_t hash<godot::Plane>::operator()(const godot::Plane& plane) const noexcept
    {
        size_t seed = 0;

        HashCombine(seed, hash<godot::Vector3>{}(plane.normal));
        HashCombine(seed, hash<real_t>{}(plane.d));

        return seed;
    }

    size_t hash<godot::Projection>::operator()(const godot::Projection& projection) const noexcept
    {
        size_t seed = 0;
        hash<godot::Vector4> hasher;

        HashCombine(seed, hasher(projection.columns[0]));
        HashCombine(seed, hasher(projection.columns[1]));
        HashCombine(seed, hasher(projection.columns[2]));
        HashCombine(seed, hasher(projection.columns[3]));

        return seed;
    }

    size_t hash<godot::Quaternion>::operator()(const godot::Quaternion& quaternion) const noexcept
    {
        size_t seed = 0;
        hash<real_t> hasher;

        HashCombine(seed, hasher(quaternion.x));
        HashCombine(seed, hasher(quaternion.y));
        HashCombine(seed, hasher(quaternion.z));
        HashCombine(seed, hasher(quaternion.w));

        return seed;
    }

    size_t hash<godot::RID>::operator()(const godot::RID& rid) const noexcept
    {
        return hash<int64_t>{}(rid.get_id());
    }

    size_t hash<godot::String>::operator()(const godot::String& string) const noexcept
    {
        return string.hash();
    }

    size_t hash<godot::StringName>::operator()(const godot::StringName& string) const noexcept
    {
        return string.hash();
    }
}