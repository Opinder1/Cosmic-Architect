#pragma once

#include "Util/Time.h"

#include <entt/fwd.hpp>

#include <vector>
#include <bitset>

struct SpeciesComponent
{
    entt::entity species;
    uint32_t race_id;
    uint32_t gender_id;
};

struct BodyComponent
{
    uint32_t fitness;
    uint32_t strength;
    uint32_t speed;
    uint32_t stamina;
    uint32_t sight_distance;
    uint32_t body_size;
};

struct VirtualBodyComponent
{
    uint32_t virtual_presence;
};

struct AquaticComponent
{
    uint32_t swim_strength;
    uint32_t swim_speed;
};

struct BreathComponent
{
    uint32_t max_breath;
    uint32_t breath;
};

struct FlightComponent
{
    uint32_t fly_speed;
};

struct LifespanComponent
{
    Clock::time_point birth_time;
    Clock::duration lifespan;
};

struct SpecialEyesComponent
{
    entt::entity eye_power;
};

struct CyborgComponent
{
    std::vector<entt::entity> implant_types;
};

struct HealthComponent
{
    uint32_t max_health;
    uint32_t health;
};

struct HungerComponent
{
    uint32_t max_hunger;
    uint32_t hunger;
};

struct ThirstComponent
{
    uint32_t max_thirst;
    uint32_t thirst;
};

struct StaminaComponent
{
    uint32_t max_stamina;
    uint32_t stamina;
};

struct CultureComponent
{
    entt::entity culture;
};

struct TalentComponent
{
    float talent_multiplier;
};

struct MagicComponent
{
    uint32_t magic_strength;
    uint32_t mana;

    std::vector<entt::entity> learned_spells;
    std::vector<uint8_t> learned_spell_proficiencies;
};

struct CultivationComponent
{
    entt::entity qi_type;
    uint32_t internal_energy;
    uint32_t external_energy;
};

struct MartialSkillsComponent 
{
    std::vector<entt::entity> martial_skills;
    std::vector<uint8_t> martial_skill_proficiencies;
};

struct SuperpowerComponent
{
    entt::entity superpower;
    uint32_t power_level;
};

struct SoulComponent
{
    uint32_t soul_strength;
    uint32_t soul_energy;
};

// Aura is the power eminating from the soul. It can be controlled and distilled into a weapon.
// Untrained aura of strong beings tends to just eminate out in all directions like bloodlust
struct AuraComponent
{
    entt::entity aura_type;
    uint32_t aura_strength;
};

struct DivinityComponent
{
    std::vector<entt::entity> divinity_types;
    std::vector<uint8_t> divinity_powers;
};

struct GodhoodComponent
{
    entt::entity godhood;
};

struct ChaoticComponent
{
    uint32_t chaos_amount;
};

struct LanguagesComponent
{
    std::vector<entt::entity> spoken_languages;
};

struct GroupMemberComponent
{
    std::vector<entt::entity> groups;
};

struct InventoryComponent
{
    std::vector<entt::entity> inventory_items; // Item type or item instance
    std::vector<uint8_t> inventory_item_counts; // (if count == 0 then its item instance)
};

struct EquipmentComponent
{
    std::vector<entt::entity> equipment;
};

struct AbilitiesComponent
{
    std::vector<entt::entity> learned_abilities;
    std::vector<uint8_t> learned_ability_levels;
};

struct BuffsComponent
{
    std::vector<entt::entity> effective_buffs;
};

struct DebuffsComponent
{
    std::vector<entt::entity> effective_debuffs;
};

struct StatusEffects
{
    std::vector<uint8_t> effects; // (Could just turn into components)
    std::vector<uint8_t> effect_amounts;
};

struct InternetAccount
{
    entt::entity internet_account;
};