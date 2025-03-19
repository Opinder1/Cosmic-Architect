#pragma once

#include <flecs/flecs.h>

#include <vector>
#include <cstdint>

namespace voxel_game
{
    struct CSoul
    {
        uint8_t soul_level; // A base value to calculate the stats of the entity
        uint8_t soul_rank; // A base value to calculate the potency (influence) of the entities abilities
        uint16_t soul_strength; // A base value to calculate the effectiveness of the entities abilities
        uint16_t soul_energy; // A base value to calculate all resource pools of the entity
    };

    // Relationship to a aura type entity. Can have multiple
    struct RAura
    {
        uint8_t aura_strength;
    };

    // Relationship to an ability type entity. Can have multiple
    struct RAbility
    {
        uint8_t ability_proficiency;
    };

    struct CMagic
    {
        uint16_t magic_strength;
        uint16_t mana;
    };

    // Relationship to a spell type entity. Can have multiple
    struct RSpell
    {
        uint8_t spell_proficiency;
    };

    // Relationship to a qi type entity. Can have multiple
    struct RCultivation
    {
        uint16_t internal_energy;
        uint16_t external_energy;
    };

    // Relationship to a divinity type entity. Can have multiple
    struct RDivinity // Also known as power over universal laws
    {
        uint16_t divinity_power;
    };

    struct CChaotic // Also known as disregard of universal laws
    {
        uint16_t chaos_amount;
    };
}