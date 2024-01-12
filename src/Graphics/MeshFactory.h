#pragma once

#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/box_mesh.hpp>

namespace godot
{
	class Mesh;
}

class CubeMeshFactory
{
public:
	CubeMeshFactory();

	godot::Ref<godot::Mesh> GenerateMesh(const std::vector<godot::Transform3D>& transforms);

private:
	godot::Ref<godot::BoxMesh> m_box_mesh;
	godot::Ref<godot::SurfaceTool> m_surface_tool;
};