#pragma once

#include <entt/fwd.hpp>

#include <vector>

struct GroupChildrenComponent
{
	std::vector<entt::entity> child_groups;
	std::vector<uint8_t> child_group_influence; // Influence the child group has on this group
	std::vector<float> child_group_percentage; // Percentage of population that is in the child group
};

struct GroupMembersComponent
{
	uint64_t number_of_members; // Estimated number of entities this group should contain
	uint32_t number_of_spawned_members; // Number of entities of this group that are loaded
};

struct GroupSpeciesComponent
{
	std::vector<entt::entity> group_species; // Species that beings of this group may spawn as
};

struct GroupCultureComponent
{
	entt::entity culture; // Culture that beings of this group spawn with
};

struct GroupRulerComponent
{
	entt::entity group_leader; // Leader of this group being a single entity or child group
	bool leader_is_group;
};

// Information about what the entity owns like stars, planets and land
struct GroupOwnedComponent
{
	std::vector<entt::entity> owned_entities;
};