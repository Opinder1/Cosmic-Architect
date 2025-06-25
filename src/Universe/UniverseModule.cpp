#include "UniverseModule.h"
#include "UniverseSimulation.h"

#include "Render/RenderModule.h"
#include "Galaxy/GalaxyModule.h"
#include "Entity/EntityModule.h"
#include "Spatial3D/SpatialModule.h"

#include "UniverseComponents.h"
#include "Entity/EntityComponents.h"
#include "Galaxy/GalaxyComponents.h"
#include "Spatial3D/SpatialComponents.h"
#include "Physics3D/PhysicsComponents.h"
#include "Loading/LoadingComponents.h"
#include "Render/RenderComponents.h"

#include "UniverseWorld.h"
#include "Loading/LoadingWorld.h"

#include "Util/Debug.h"

namespace voxel_game::universe
{
	simulation::ConfigDefaults g_config_defaults;

	void InitializeConfigDefaults()
	{
		if (!g_config_defaults.empty())
		{
			return;
		}

		g_config_defaults =
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
		spatial3d::WorldPtr world;

		void LoadNodeRandomly(spatial3d::NodePtr node)
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

				galaxy::CreateGalaxy(simulation, node, position, godot::Vector3(box_size, box_size, box_size), entity::Ref());
			}
		}

		void LoadNodePlane(spatial3d::NodePtr node)
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

			galaxy::CreateGalaxy(simulation, node, position, godot::Vector3i{ scale_node_step / 4, 1, scale_node_step / 4 }, entity::Ref());
		}

		void UnloadNode(spatial3d::NodePtr node)
		{
			const uint32_t entities_per_node = 4;
			const uint32_t scale_step = 1 << node->*&spatial3d::Node::scale_index;
			const uint32_t scale_node_step = scale_step * world->*&spatial3d::World::node_size;

			for (entity::WRef galaxy : node->*&Node::galaxies)
			{
				// galaxy.unref();
			}
		}
	};

	entity::Ref LoadUniverse(Simulation& simulation, UUID id, const godot::String& path)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation);

		bool created;
		entity::Ref universe_entity = simulation.entity_factory.GetPoly(id, created);

		if (!created)
		{
			return universe_entity;
		}

		// Create the universe
		simulation.entity_factory.AddTypes<
			universe::CUniverse,
			entity::CRelationship,
			spatial3d::CWorld,
			loading::CStreamable,
			loading::CAutosave,
			rendering::CScenario
		>(universe_entity.GetID());

		if (rendering::IsEnabled())
		{
			simulation.entity_factory.AddTypes<rendering::CTransform>(universe_entity.GetID());
		}

		universe_entity->*&CUniverse::path = path;

		simulation::InitializeConfig(universe_entity->*&CUniverse::config, path.path_join("config.json"), g_config_defaults);
		universe_entity->*&CUniverse::last_config_save = simulation.frame_start_time;

		spatial3d::WorldPtr world = spatial3d::CreateWorld(simulation.universe_type);

		world->*&spatial3d::World::node_size = 16;
		world->*&spatial3d::PartialWorld::node_keepalive = 1s;

		loading::WorldOpenDatabase(simulation, world, path.path_join("galaxies.db"));

		spatial3d::EntitySetWorld(simulation, universe_entity, world);

		entity::OnLoadEntity(simulation, universe_entity);

		return universe_entity;
	}

	void OnUpdateUniverseEntity(Simulation& simulation, entity::EventData& data)
	{
		if (simulation.frame_start_time - data.entity->*&CUniverse::last_config_save > 10s)
		{
			simulation::SaveJsonConfig(data.entity->*&CUniverse::config);
			data.entity->*&CUniverse::last_config_save = simulation.frame_start_time;
		}

		switch (data.entity->*&loading::CStreamable::state)
		{
		case loading::State::Unloaded:
			break;

		case loading::State::Loading:
			break;

		case loading::State::Loaded:
			break;

		case loading::State::Unloading:
			break;
		}
	}

	void OnLoadUniverseEntity(Simulation& simulation, entity::EventData& data)
	{
		simulation.universes.push_back(entity::Ref(data.entity));
	}

	void OnUnloadUniverseEntity(Simulation& simulation, entity::EventData& data)
	{
		unordered_erase(simulation.universes, entity::Ref(data.entity));
	}

	void Initialize(Simulation& simulation)
	{
		InitializeConfigDefaults();

		simulation.universe_type.node_type.AddType<spatial3d::Node>();
		simulation.universe_type.node_type.AddType<spatial3d::PartialNode>();
		simulation.universe_type.node_type.AddType<loading::Node>();
		simulation.universe_type.node_type.AddType<Node>();

		simulation.universe_type.scale_type.AddType<spatial3d::Scale>();
		simulation.universe_type.scale_type.AddType<spatial3d::PartialScale>();
		simulation.universe_type.scale_type.AddType<loading::Scale>();
		simulation.universe_type.scale_type.AddType<Scale>();

		simulation.universe_type.world_type.AddType<spatial3d::World>();
		simulation.universe_type.world_type.AddType<spatial3d::PartialWorld>();
		simulation.universe_type.world_type.AddType<loading::World>();
		simulation.universe_type.world_type.AddType<World>();

		simulation.entity_factory.AddCallback<CUniverse, loading::CStreamable>(entity::Event::Update, cb::Bind<&OnUpdateUniverseEntity>());
		simulation.entity_factory.AddCallback<CUniverse>(entity::Event::BeginLoad, cb::Bind<&OnLoadUniverseEntity>());
		simulation.entity_factory.AddCallback<CUniverse>(entity::Event::BeginUnload, cb::Bind<&OnUnloadUniverseEntity>());
	}

	void Uninitialize(Simulation& simulation)
	{
		simulation.universes.clear();
	}

	bool IsUnloadDone(Simulation& simulation)
	{
		return true;
	}

	void Update(Simulation& simulation)
	{

	}

	void WorkerUpdate(Simulation& simulation, size_t index)
	{

	}

	void WorldUpdate(Simulation& simulation, spatial3d::WorldPtr world)
	{

	}

	void ScaleUpdate(Simulation& simulation, spatial3d::ScalePtr scale)
	{
		UniverseNodeLoaderTest loader{ simulation, scale->*&spatial3d::Scale::world };

		for (const spatial3d::NodeCommand& command : scale->*&spatial3d::PartialScale::load_commands)
		{
			loader.LoadNodePlane(command.node);
		}

		for (const spatial3d::NodeCommand& command : scale->*&spatial3d::PartialScale::unload_commands)
		{
			loader.UnloadNode(command.node);
		}
	}
}