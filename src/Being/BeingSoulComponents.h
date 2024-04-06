#pragma once

#include <cstdint>

namespace voxel_game
{
    struct SoulComponent
    {
        uint8_t soul_level;
        uint8_t soul_rank;
        uint16_t soul_strength;
        uint16_t soul_energy;
    };

    struct AuraRelationship
    {
        uint8_t aura_strength;
    };

    struct DivinityRelationship
    {
        uint16_t divinity_power;
        bool has_godhood;
    };

    struct ChaoticComponent
    {
        uint16_t chaos_amount;
    };

    struct MagicComponent
    {
        uint16_t magic_strength;
        uint16_t mana;
    };

    struct CultivationRelationship
    {
        uint16_t internal_energy;
        uint16_t external_energy;
    };

    struct AbilityRelationship
    {
        uint8_t ability_proficiency;
    };
}