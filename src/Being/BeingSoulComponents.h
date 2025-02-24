#pragma once

#include <cstdint>

namespace voxel_game
{
    struct CSoul
    {
        uint8_t soul_level;
        uint8_t soul_rank;
        uint16_t soul_strength;
        uint16_t soul_energy;
    };

    struct RAura
    {
        uint8_t aura_strength;
    };

    struct RDivinity
    {
        uint16_t divinity_power;
        bool has_godhood;
    };

    struct CChaotic
    {
        uint16_t chaos_amount;
    };

    struct CMagic
    {
        uint16_t magic_strength;
        uint16_t mana;
    };

    struct RCultivation
    {
        uint16_t internal_energy;
        uint16_t external_energy;
    };

    struct RAbility
    {
        uint8_t ability_proficiency;
    };
}