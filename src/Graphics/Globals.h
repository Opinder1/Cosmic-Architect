#pragma once

#include "MeshFactory.h"

namespace godot
{
	class RenderingServer;
}

struct RenderingGlobal
{
	godot::RenderingServer* server = nullptr;

	CubeMeshFactory cube_mesh_factory;
};