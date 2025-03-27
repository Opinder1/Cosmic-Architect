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

	struct NodeRef : NodeType::Ref
	{
		using NodeType::Ref::Ref;
	};

	struct ScaleRef : ScaleType::Ref
	{
		using ScaleType::Ref::Ref;
	};

	struct WorldRef : WorldType::Ref
	{
		using WorldType::Ref::Ref;
	};

	struct Types
	{
		NodeType node_type;
		ScaleType scale_type;
		WorldType world_type;
	};
}