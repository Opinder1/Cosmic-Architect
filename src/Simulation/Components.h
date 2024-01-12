#pragma once

#include "UUID.h"

#include "Message/Message.h"

#include <robin_hood/robin_hood.h>

namespace sim
{
	// Assign this to any entity created in the current tick
	struct NewComponent {};

	// Assign this to any entity deleted in the current tick
	struct DeletedComponent {};

	struct ParentDeletedComponent {};

	// A component to give the entity a global unique identifier
	struct IDComponent
	{
		UUID id;
	};

	// This entity has a messager that we can send messages to
	struct LinkedMessagerComponent
	{
		UUID messager_id;
		MessageQueue queued_messages;
	};

	// This component notes the linked messager is a simulation in another thread
	struct LinkedThreadSimulationComponent {};

	// This component notes the linked messager is a simulation in another process
	struct LinkedNetworkSimulationComponent {};

	// This entity has an owner that is the authority over it
	struct SimulationOwnerComponent
	{
		UUID owner_simulation_id;
	};

	template<class Tag>
	struct GroupComponent
	{
		size_t num_members
	};

	template<class Tag>
	struct GroupMemberComponent : Tag::GroupMember
	{
		UUID group_id;
		entt::entity group;
		Tag::Data data;
	};

	template<class Tag>
	struct MultiGroupMemberComponent : Tag::MultiGroupMember
	{
		struct Data
		{
			entt::entity entity;
			Tag::Data data;
		};

		robin_hood::unordered_flat_map<UUID, Data> groups;
	};

	template<class Tag>
	struct VectorChildComponent
	{
		entt::entity parent;
	};

	template<class Tag>
	struct VectorComponent
	{
		std::vector<entt::entity> children;
	};

	template<class Tag>
	struct HierarchyComponent : Tag::Hierarchy
	{
		size_t num_children;
		entt::entity first_child;
		entt::entity last_child;

		entt::entity parent;
		entt::entity prev_sibling;
		entt::entity next_sibling;
	};
}