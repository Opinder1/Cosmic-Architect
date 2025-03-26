#pragma once

#include "Util/Poly.h"
#include "Util/Util.h"
#include "Util/Debug.h"

namespace voxel_game::spatial3d
{
	struct NodeType : PolyType<NodeType, 6>
	{
		using PolyType::PolyType;
	};

	struct ScaleType : PolyType<ScaleType, 6>
	{
		using PolyType::PolyType;
	};

	struct WorldType : PolyType<WorldType, 6>
	{
		using PolyType::PolyType;
	};

	struct NodeRef : PolyRef<NodeType>
	{
		using PolyRef::PolyRef;
	};

	struct ScaleRef : PolyRef<ScaleType>
	{
		using PolyRef::PolyRef;
	};

	struct WorldRef : PolyRef<WorldType>
	{
		using PolyRef::PolyRef;
	};

	struct Types
	{
		NodeType node_type;
		ScaleType scale_type;
		WorldType world_type;
	};
}