#pragma once

#include <entt/fwd.hpp>

#include <vector>
#include <bitset>

// This component describes the mind of a being
struct MentalComponent
{
    uint32_t max_saneness;
    uint32_t sanity;
    uint32_t intelligence_modifier;
};

struct SentienceComponent
{
    std::bitset<64> personality_traits;
    std::bitset<64> emotions;
};

// This is this entities views on other entities. The relationships can be between beings, species and groups
struct RelationshipComponent
{
    std::vector<entt::entity> relationship_entities;
    std::vector<std::bitset<64>> relationship_thoughts;
};

// This is this entites views on different religions. This is for beings and groups
struct BeliefComponent
{
    std::vector<entt::entity> interested_religions;
    std::vector<int8_t> believed_amounts;
};

// This component describes how much the being wants to reproduce
struct ReproductiveUrgeComponent
{
    uint32_t reproductive_urge;
};