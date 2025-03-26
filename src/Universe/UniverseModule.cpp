#include "UniverseModule.h"
#include "UniverseComponents.h"

#include "Galaxy/GalaxyComponents.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialModule.h"

#include "Physics3D/PhysicsComponents.h"

#include "Loading/LoadingComponents.h"

#include "Simulation/SimulationComponents.h"
#include "Simulation/SimulationModule.h"

#include "Render/RenderComponents.h"
#include "Render/RenderModule.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game::universe
{
	spatial3d::Types universe_world_types;

	sim::ConfigDefaults config_defaults;

	void InitializeConfigDefaults()
	{
		if (!config_defaults.empty())
		{
			return;
		}

		config_defaults =
		{
			// General
			{ "campaign_script", "" },

			// Game mechanics
			{ "universe_size", 0 },
			{ "max_player_level", 1000 },
			{ "max_npc_level", 999 },
			{ "player_experience_gain_multiplier", 1.0 },
			{ "global_loot_amount_multiplier", 1.0 },
			{ "invisibility_ability_opacity", 20.0 },
			{ "weather_events_frequency_multiplier", 1.0 },
			{ "disaster_events_frequency_multiplier", 1.0 },
			{ "faction_events_frequency_multiplier", 1.0 },
			{ "plant_grow_speed_multiplier", 1.0 },
			{ "npc_damage_multiplier", 1.0 },
			{ "npc_health_multiplier", 1.0 },
			{ "npc_energy_multiplier", 1.0 },
			{ "npc_intelligence_multiplier", 1.0 },
			{ "level0_civilisation_generation_factor", 1.0 },
			{ "level1_civilization_generation_factor", 1.0 },
			{ "level2_civilization_generation_factor", 1.0 },
			{ "level3_civilization_generation_factor", 1.0 },
			{ "forget_uninteracted_npcs", true },
			{ "max_dropped_items", 1000 },
			{ "dropped_item_removal_frequency", 10.0 },
			{ "merge_similar_close_items", true },
			{ "merge_all_close_items", false },

			// Player mechanics
			{ "allow_debug_config", false },
			{ "player_experience_gain_multiplier", 1.0 },
			{ "pvp_allowed", false },
			{ "pvp_power_normalise_factor", 0.0 },
			{ "keep_inventory_on_death", false },
			{ "keep_equipment_on_death", false },
			{ "player_level_difference_see_divine", 1000 },
			{ "respawn_time", 10.0 },
			{ "respawn_minimum_distance", 100.0 },
			{ "can_respawn_in_faction", true },
			{ "npc_forget_aggro_on_respawn", false },
			{ "new_player_distance_from_activity", 1000.0 },
			{ "new_player_starting_level", 10 },
			{ "new_player_starting_economy", 0.0 },
			{ "new_player_area_maximum_danger", 2 },

			// Multiplayer
			{ "max_players", 1 },
			{ "use_player_roles", false },
			{ "allow_chat", true },
			{ "enable_profanity_filter", true },
			{ "allow_external_character_profile", false },
			{ "allow_external_schematics", false },
			{ "use_accounts", false },
			{ "use_blacklist", false },
			{ "use_whitelist", false },
			{ "enable_area_claiming", true },
			{ "allow_area_overclaim", false },
			{ "boss_difficulty_increase_per_player", 50.0 },
		};
	}

	flecs::entity CreateNewUniverse(flecs::world& world, const godot::StringName& path)
	{
		// Create the universe
		flecs::entity universe_entity = world.entity();

#if defined(DEBUG_ENABLED)
		universe_entity.set_name("Universe");
#endif

		universe_entity.emplace<sim::CPath>(path);
		universe_entity.add<universe::CWorld>();
		universe_entity.add<spatial3d::CWorld>();
		universe_entity.add<sim::CConfig>();

		if (rendering::IsEnabled())
		{
			world.add<rendering::CTransform>();
		}

		sim::InitializeConfig(universe_entity, path.path_join("config.json"), config_defaults);

		spatial3d::CWorld* spatial_world = universe_entity.get_mut<spatial3d::CWorld>();

		spatial_world->world = spatial3d::CreateWorld(universe_world_types, spatial3d::k_max_world_scale);
		spatial_world->world->*&spatial3d::World::node_size = 16;
		spatial_world->world->*&spatial3d::World::node_keepalive = 1s;

		spatial3d::InitializeWorldScaleEntities(universe_entity, spatial_world->world);

		flecs::entity entity_loader = world.entity();

		entity_loader.emplace<loading::CEntityDatabase>(path.path_join("entities.db"));
		entity_loader.child_of(universe_entity);

		return universe_entity;
	}

	// Spawns a bunch of random cubes around the camera
	struct UniverseNodeLoaderTest
	{
		flecs::entity entity;
		spatial3d::WorldRef world;

		void CreateGalaxy(spatial3d::NodeRef node, godot::Vector3 position, godot::Vector3 scale)
		{
			flecs::entity galaxy = sim::GetPool().CreateEntity();

			galaxy.child_of(entity);
			galaxy.add<galaxy::CWorld>();
			galaxy.add<physics3d::CPosition>();
			galaxy.add<physics3d::CScale>();
			galaxy.set(physics3d::CPosition{ position });
			galaxy.set(physics3d::CScale{ scale });

			if (rendering::IsEnabled())
			{
				flecs::entity galaxy_schematic = sim::GetPool().CreateEntity();

				galaxy_schematic.add<rendering::CPlaceholderCube>();

				galaxy.add<rendering::CTransform>();
				galaxy.add<rendering::CInstance>(galaxy_schematic);
			}

			spatial3d::CEntity& entity = galaxy.ensure<spatial3d::CEntity>();

			(node->*&spatial3d::Node::entities).insert(entity.entity);
			(node->*&Node::galaxies).push_back(galaxy);
		}

		void LoadNodeRandomly(spatial3d::NodeRef node)
		{
			const uint32_t entities_per_node = 4;
			const uint32_t scale_step = 1 << node->*&spatial3d::Node::scale_index;
			const uint32_t scale_node_step = scale_step * world->*&spatial3d::World::node_size;
			const double box_size = double(scale_step) / 2.0;

			for (size_t i = 0; i < entities_per_node; i++)
			{
				godot::Vector3 position = node->*&spatial3d::Node::position * scale_node_step;

				position.x += godot::UtilityFunctions::randf_range(0, scale_node_step);
				position.y += godot::UtilityFunctions::randf_range(0, scale_node_step);
				position.z += godot::UtilityFunctions::randf_range(0, scale_node_step);

				CreateGalaxy(node, position, godot::Vector3(box_size, box_size, box_size));
			}
		}

		void LoadNodePlane(spatial3d::NodeRef node)
		{
			if ((node->*&spatial3d::Node::position).y != 0)
			{
				return;
			}

			const uint32_t scale_step = 1 << node->*&spatial3d::Node::scale_index;
			const int32_t scale_node_step = scale_step * world->*&spatial3d::World::node_size;
			const uint8_t box_shrink = 2;

			godot::Vector3i position = node->*&spatial3d::Node::position * scale_node_step;

			position.y -= node->*&spatial3d::Node::scale_index - 1;

			CreateGalaxy(node, position, godot::Vector3i{ scale_node_step / 4, 1, scale_node_step / 4 });
		}

		void UnloadNode(spatial3d::NodeRef node)
		{
			const uint32_t entities_per_node = 4;
			const uint32_t scale_step = 1 << node->*&spatial3d::Node::scale_index;
			const uint32_t scale_node_step = scale_step * world->*&spatial3d::World::node_size;

			for (flecs::entity_t galaxy : node->*&Node::galaxies)
			{
				flecs::entity(entity.world(), galaxy).destruct();
			}
		}
	};

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
		world.import<sim::Components>();
		world.import<loading::Components>();
		world.import<physics3d::Components>();
		world.import<spatial3d::Components>();
		world.import<universe::Components>();

		InitializeConfigDefaults();

		universe_world_types.node_type.AddType<spatial3d::Node>();
		universe_world_types.scale_type.AddType<spatial3d::Scale>();
		universe_world_types.world_type.AddType<spatial3d::World>();
		universe_world_types.node_type.AddType<Node>();
		universe_world_types.scale_type.AddType<Scale>();
		universe_world_types.world_type.AddType<World>();

		world.system<spatial3d::CScale, const spatial3d::CWorld>(DEBUG_ONLY("UniverseLoadSpatialNode"))
			.multi_threaded()
			.term_at(1).up(flecs::ChildOf)
			.with<const CWorld>().up(flecs::ChildOf)
			.each([](flecs::entity entity, spatial3d::CScale& spatial_scale, const spatial3d::CWorld& spatial_world)
		{
			UniverseNodeLoaderTest loader{ entity.parent(), spatial_world.world};

			for (spatial3d::NodeRef node : spatial_scale.scale->*&spatial3d::Scale::load_commands)
			{
				loader.LoadNodePlane(node);
			}

			for (spatial3d::NodeRef node : spatial_scale.scale->*&spatial3d::Scale::unload_commands)
			{
				loader.UnloadNode(node);
			}
		});
	}
}