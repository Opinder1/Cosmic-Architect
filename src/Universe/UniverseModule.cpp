#include "UniverseModule.h"
#include "UniverseComponents.h"

#include "Galaxy/GalaxyComponents.h"
#include "Galaxy/GalaxyPrefabs.h"

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
		universe_entity.emplace<loading::CEntityDatabase>(path.path_join("entities.db"));

		if (rendering::IsEnabled())
		{
			universe_entity.add<rendering::CTransform>();
		}

		sim::InitializeConfig(universe_entity, path.path_join("config.json"), config_defaults);

		spatial3d::CWorld* spatial_world = universe_entity.get_mut<spatial3d::CWorld>();

		spatial_world->world = spatial3d::CreateWorld(universe_world_types, spatial3d::k_max_world_scale);
		spatial_world->world->*&spatial3d::World::node_size = 16;
		spatial_world->world->*&spatial3d::World::node_keepalive = 1s;

		spatial3d::InitializeWorldScaleEntities(universe_entity, spatial_world->world);

		return universe_entity;
	}

	// Spawns a bunch of random cubes around the camera
	struct UniverseNodeLoaderTest1
	{
		flecs::entity entity;
		spatial3d::WorldRef world;

		void LoadNode(spatial3d::NodeRef node)
		{
			const uint32_t entities_per_node = 4;
			const uint32_t scale_step = 1 << node->*&spatial3d::Node::scale_index;
			const uint32_t scale_node_step = scale_step * world->*&spatial3d::World::node_size;
			const double box_size = double(scale_step) / 2.0;

			for (size_t i = 0; i < entities_per_node; i++)
			{
				double position_x = (node->*&spatial3d::Node::position).x * scale_node_step;
				double position_y = (node->*&spatial3d::Node::position).y * scale_node_step;
				double position_z = (node->*&spatial3d::Node::position).z * scale_node_step;

				position_x += godot::UtilityFunctions::randf_range(0, scale_node_step);
				position_y += godot::UtilityFunctions::randf_range(0, scale_node_step);
				position_z += godot::UtilityFunctions::randf_range(0, scale_node_step);

				flecs::entity galaxy = sim::GetPool().CreateEntity();

				galaxy.child_of(entity);
				galaxy.is_a<galaxy::GalaxyPrefab>();
				galaxy.set(physics3d::CPosition{ godot::Vector3(position_x, position_y, position_z) });
				galaxy.set(physics3d::CScale{ godot::Vector3(box_size, box_size, box_size) });
				galaxy.add<rendering::CTransform>();
				spatial3d::CEntity& entity = galaxy.ensure<spatial3d::CEntity>();

				(node->*&spatial3d::Node::entities).insert(entity.entity);
				(node->*&Node::galaxies).push_back(galaxy);
			}
		}
	};

	// Spawns a bunch of flat squares around the camera on the xz plane
	struct UniverseNodeLoaderTest2
	{
		flecs::entity entity;
		spatial3d::WorldRef world;

		void LoadNode(spatial3d::NodeRef node)
		{
			if ((node->*&spatial3d::Node::position).y != 0)
			{
				return;
			}

			const uint32_t scale_step = 1 << node->*&spatial3d::Node::scale_index;
			const int32_t scale_node_step = scale_step * world->*&spatial3d::World::node_size;
			const uint8_t box_shrink = 2;

			int32_t position_x = (node->*&spatial3d::Node::position).x * scale_node_step;
			int32_t position_y = (node->*&spatial3d::Node::position).y * scale_node_step;
			int32_t position_z = (node->*&spatial3d::Node::position).z * scale_node_step;

			flecs::entity galaxy = sim::GetPool().CreateEntity();

			galaxy.child_of(entity);
			galaxy.is_a<galaxy::GalaxyPrefab>();
			galaxy.emplace<physics3d::CPosition>(godot::Vector3i{ position_x, position_y - node->*&spatial3d::Node::scale_index - 1, position_z });
			galaxy.emplace<physics3d::CScale>(godot::Vector3i{scale_node_step / 4, 1, scale_node_step / 4});
			galaxy.add<rendering::CTransform>();
			spatial3d::CEntity& entity = galaxy.ensure<spatial3d::CEntity>();

			(node->*&spatial3d::Node::entities).insert(entity.entity);
			(node->*&Node::galaxies).push_back(galaxy);
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

		world.import<sim::Components>();
		world.import<loading::Components>();
		world.import<physics3d::Components>();
		world.import<spatial3d::Components>();
		world.import<universe::Components>();
		world.import<galaxy::Prefabs>();

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
			UniverseNodeLoaderTest2 loader{ entity, spatial_world.world };

			for (spatial3d::NodeRef node : spatial_scale.scale->*&spatial3d::Scale::load_commands)
			{
				loader.LoadNode(node);
			}

			for (spatial3d::NodeRef node : spatial_scale.scale->*&spatial3d::Scale::unload_commands)
			{
				loader.UnloadNode(node);
			}
		});
	}
}