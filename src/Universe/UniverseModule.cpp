#include "UniverseModule.h"
#include "UniverseComponents.h"
#include "UniverseSimulation.h"

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

namespace voxel_game::universe
{
	simulation::ConfigDefaults config_defaults;

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

	// Spawns a bunch of random cubes around the camera
	struct UniverseNodeLoaderTest
	{
		Simulation& simulation;
		spatial3d::WorldRef world;

		void CreateGalaxy(spatial3d::NodeRef node, godot::Vector3 position, godot::Vector3 scale)
		{
			entity::Ptr galaxy = simulation.entity_factory.CreatePoly(GenerateUUID());

			simulation.entity_factory.AddTypes<galaxy::CWorld, physics3d::CPosition, physics3d::CScale, physics3d::CPosition, physics3d::CScale, spatial3d::CEntity>(galaxy.GetID());

			galaxy->*&physics3d::CPosition::position = position;
			galaxy->*&physics3d::CScale::scale = scale;

			(node->*&spatial3d::Node::entities).insert(galaxy);
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

			for (entity::Ptr galaxy : node->*&Node::galaxies)
			{
				// galaxy.unref();
			}
		}
	};

	void Initialize(Simulation& simulation)
	{
		InitializeConfigDefaults();

		simulation.universe_types.node_type.AddType<spatial3d::Node>();
		simulation.universe_types.node_type.AddType<Node>();

		simulation.universe_types.scale_type.AddType<spatial3d::Scale>();
		simulation.universe_types.scale_type.AddType<Scale>();

		simulation.universe_types.world_type.AddType<spatial3d::World>();
		simulation.universe_types.world_type.AddType<World>();
	}

	void Uninitialize(Simulation& simulation)
	{

	}

	void Update(Simulation& simulation)
	{
		for (spatial3d::WorldRef world : simulation.universe_worlds)
		{
			UniverseNodeLoaderTest loader{ simulation, world };

			for (spatial3d::ScaleRef scale : world->*&spatial3d::World::scales)
			{
				for (spatial3d::NodeRef node : scale->*&spatial3d::Scale::load_commands)
				{
					loader.LoadNodePlane(node);
				}

				for (spatial3d::NodeRef node : scale->*&spatial3d::Scale::unload_commands)
				{
					loader.UnloadNode(node);
				}
			}
		}
	}

	void WorkerUpdate(Simulation& simulation, size_t index)
	{

	}

	entity::Ptr CreateNewUniverse(Simulation& simulation, const godot::StringName& path)
	{
		// Create the universe
		entity::Ptr universe_entity = simulation.entity_factory.CreatePoly(GenerateUUID());

		simulation.entity_factory.AddTypes<
			simulation::CPath,
			universe::CWorld,
			spatial3d::CWorld,
			simulation::CConfig
		>(universe_entity.GetID());

		universe_entity->*&simulation::CPath::path = path;

		if (rendering::IsEnabled())
		{
			simulation.entity_factory.AddTypes<rendering::CTransform>(universe_entity.GetID());
		}

		simulation::InitializeConfig(universe_entity.Get<simulation::CConfig>(), path.path_join("config.json"), config_defaults);

		universe_entity->*&spatial3d::CWorld::world = spatial3d::CreateWorld(simulation.universe_types, spatial3d::k_max_world_scale);
		(universe_entity->*&spatial3d::CWorld::world)->*&spatial3d::World::node_size = 16;
		(universe_entity->*&spatial3d::CWorld::world)->*&spatial3d::World::node_keepalive = 1s;

		// flecs::entity entity_loader = world.entity();

		// entity_loader.emplace<loading::CEntityDatabase>(path.path_join("entities.db"));
		// entity_loader.child_of(universe_entity);

		return universe_entity;
	}
}