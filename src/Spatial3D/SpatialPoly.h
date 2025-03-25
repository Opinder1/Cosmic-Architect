#pragma once

#include "Util/Poly.h"
#include "Util/Util.h"
#include "Util/Debug.h"

namespace voxel_game::spatial3d
{
	struct NodePolyType {};
	struct ScalePolyType {};
	struct WorldPolyType {};

	using NodeType = PolyType<NodePolyType, 6>;
	using ScaleType = PolyType<ScalePolyType, 6>;
	using WorldType = PolyType<WorldPolyType, 6>;

	using NodeRef = PolyRef<NodeType>;
	using ScaleRef = PolyRef<ScaleType>;
	using WorldRef = PolyRef<WorldType>;

	struct Types
	{
		NodeType node_type;
		ScaleType scale_type;
		WorldType world_type;
	};
}