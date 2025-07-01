#pragma once

#include "Util/Poly.h"
#include "Util/Util.h"
#include "Util/Debug.h"

namespace voxel_game::spatial3d
{
	struct NodeType : PolyType<NodeType, 10>
	{
		using PolyType::PolyType;
	};

	struct ScaleType : PolyType<ScaleType, 8>
	{
		using PolyType::PolyType;
	};

	struct WorldType : PolyType<WorldType, 11>
	{
		using PolyType::PolyType;
	};

	using NodePtr = NodeType::Ptr;
	using ScalePtr = ScaleType::Ptr;
	using WorldPtr = WorldType::Ptr;
}