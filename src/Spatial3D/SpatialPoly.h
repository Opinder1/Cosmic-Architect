#pragma once

#include "Util/Poly.h"
#include "Util/Util.h"
#include "Util/Debug.h"

namespace voxel_game::spatial3d
{
	struct WorldType : PolyType<WorldType, 6>
	{
		using PolyType::PolyType;
	};

	struct ScaleType : PolyType<ScaleType, 6>
	{
		using PolyType::PolyType;
	};

	struct NodeType : PolyType<NodeType, 6>
	{
		using PolyType::PolyType;
	};

	using WorldPtr = WorldType::Ptr;
	using ScalePtr = ScaleType::Ptr;
	using NodePtr = NodeType::Ptr;
}