#pragma once

#include "Util/Poly.h"
#include "Util/Util.h"
#include "Util/Debug.h"

namespace voxel_game::spatial3d
{
	struct NodeHeader;
	struct ScaleHeader;
	struct WorldHeader;

#if defined(DEBUG_ENABLED)
	struct Node;
	struct Scale;
	struct World;
#endif

	struct Types
	{
		PolyType<NodeHeader> node_type;
		PolyType<ScaleHeader> scale_type;
		PolyType<WorldHeader> world_type;
	};

	struct NodeHeader
	{
		Types* types;
#if defined(DEBUG_ENABLED) // Pointer to see node info in debugger
		Node* ptr;
#endif
	};

	struct ScaleHeader
	{
		Types* types;
#if defined(DEBUG_ENABLED) // Pointer to see scale info in debugger
		Scale* ptr;
#endif
	};

	struct WorldHeader
	{
		Types* types;
#if defined(DEBUG_ENABLED) // Pointer to see world info in debugger
		World* ptr;
#endif
	};

	struct NodeRef
	{
		NodeRef() : poly(nullptr) {}
		NodeRef(NodeHeader* poly) : poly(poly) {}

		template<auto Member,
			class Ret = get_member_type<decltype(Member)>::type,
			class Class = get_member_class<decltype(Member)>::type>
		Ret& Get()
		{
			return poly->types->node_type.Get<Class>(poly)->*Member;
		}

		template<class T, class Ret>
		Ret& operator->*(Ret T::*Member)
		{
			return poly->types->node_type.Get<T>(poly)->*Member;
		}

		operator bool()
		{
			return poly != nullptr;
		}

		NodeHeader* poly;
	};

	struct ConstNodeRef
	{
		ConstNodeRef() : poly(nullptr) {}
		ConstNodeRef(const NodeHeader* poly) : poly(poly) {}
		ConstNodeRef(NodeRef ref) : poly(ref.poly) {}

		NodeRef NonConst() { return NodeRef(const_cast<NodeHeader*>(poly)); }

		template<auto Member,
			class Ret = get_member_type<decltype(Member)>::type,
			class Class = get_member_class<decltype(Member)>::type>
		const Ret& Get()
		{
			return poly->types->node_type.Get<Class>(poly)->*Member;
		}

		template<class T, class Ret>
		const Ret& operator->*(Ret T::* Member)
		{
			return poly->types->node_type.Get<T>(poly)->*Member;
		}

		operator bool()
		{
			return poly != nullptr;
		}

		const NodeHeader* poly;
	};

	struct ScaleRef
	{
		ScaleRef() : poly(nullptr) {}
		ScaleRef(ScaleHeader* poly) : poly(poly) {}

		template<auto Member,
			class Ret = get_member_type<decltype(Member)>::type,
			class Class = get_member_class<decltype(Member)>::type>
		Ret& Get()
		{
			return poly->types->scale_type.Get<Class>(poly)->*Member;
		}

		template<class T, class Ret>
		Ret& operator->*(Ret T::* Member)
		{
			return poly->types->scale_type.Get<T>(poly)->*Member;
		}

		operator bool()
		{
			return poly != nullptr;
		}

		ScaleHeader* poly;
	};

	struct ConstScaleRef
	{
		ConstScaleRef() : poly(nullptr) {}
		ConstScaleRef(const ScaleHeader* poly) : poly(poly) {}
		ConstScaleRef(ScaleRef ref) : poly(ref.poly) {}

		ScaleRef NonConst() { return ScaleRef(const_cast<ScaleHeader*>(poly)); }

		template<auto Member,
			class Ret = get_member_type<decltype(Member)>::type,
			class Class = get_member_class<decltype(Member)>::type>
		const Ret& Get()
		{
			return poly->types->scale_type.Get<Class>(poly)->*Member;
		}

		template<class T, class Ret>
		const Ret& operator->*(Ret T::* Member)
		{
			return poly->types->scale_type.Get<T>(poly)->*Member;
		}

		operator bool()
		{
			return poly != nullptr;
		}

		const ScaleHeader* poly;
	};

	struct WorldRef
	{
		WorldRef() : poly(nullptr) {}
		WorldRef(WorldHeader* poly) : poly(poly) {}

		template<auto Member,
			class Ret = get_member_type<decltype(Member)>::type,
			class Class = get_member_class<decltype(Member)>::type>
		Ret& Get()
		{
			return poly->types->world_type.Get<Class>(poly)->*Member;
		}

		template<class T, class Ret>
		Ret& operator->*(Ret T::* Member)
		{
			return poly->types->world_type.Get<T>(poly)->*Member;
		}

		operator bool()
		{
			return poly != nullptr;
		}

		WorldHeader* poly;
	};

	struct ConstWorldRef
	{
		ConstWorldRef() : poly(nullptr) {}
		ConstWorldRef(const WorldHeader* poly) : poly(poly) {}
		ConstWorldRef(WorldRef ref) : poly(ref.poly) {}

		WorldRef NonConst() { return WorldRef(const_cast<WorldHeader* > (poly)); }

		template<auto Member,
			class Ret = get_member_type<decltype(Member)>::type,
			class Class = get_member_class<decltype(Member)>::type>
		const Ret& Get()
		{
			return poly->types->world_type.Get<Class>(poly)->*Member;
		}

		template<class T, class Ret>
		const Ret& operator->*(Ret T::* Member)
		{
			return poly->types->world_type.Get<T>(poly)->*Member;
		}

		operator bool()
		{
			return poly != nullptr;
		}

		const WorldHeader* poly;
	};
}