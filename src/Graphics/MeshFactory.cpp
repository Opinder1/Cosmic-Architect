#include "MeshFactory.h"

#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/box_mesh.hpp>

CubeMeshFactory::CubeMeshFactory()
{
	m_box_mesh.instantiate();
	m_surface_tool.instantiate();
}

godot::Ref<godot::Mesh> CubeMeshFactory::GenerateMesh(const std::vector<godot::Transform3D>& transforms)
{
	for (const godot::Transform3D& transform : transforms)
	{
		m_surface_tool->append_from(m_box_mesh, 0, transform);
	}

	godot::Ref<godot::Mesh> mesh = m_surface_tool->commit();

	m_surface_tool->clear();

	return mesh;
}