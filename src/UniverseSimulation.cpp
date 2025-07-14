#include "UniverseSimulation.h"

#include "Universe/UniverseModule.h"
#include "Galaxy/GalaxyModule.h"
#include "GalaxyRender/GalaxyRenderModule.h"
#include "Player/PlayerModule.h"
#include "Spatial3D/SpatialModule.h"
#include "Voxel/VoxelModule.h"
#include "VoxelRender/VoxelRenderModule.h"
#include "Render/RenderModule.h"
#include "DebugRender/DebugRenderModule.h"
#include "Simulation/SimulationModule.h"
#include "Physics3D/PhysicsModule.h"
#include "Entity/EntityModule.h"

#include "Entity/EntityComponents.h"

#include "Simulation/Config.h"

#include "Util/SmallVector.h"

#include <godot_cpp/classes/worker_thread_pool.hpp>

namespace voxel_game
{
	simulation::ConfigDefaults GetConfigDefaults()
	{
		return
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

	// The simulation task callback executor
	void TaskCallback(void* userdata, uint32_t element)
	{
		TaskData* taskdata = reinterpret_cast<TaskData*>(userdata);

		taskdata->callback(taskdata->simulation, element);
	}

	// Run a single task group and wait for it after
	void SimulationDoTasks(Simulation& simulation, TaskData& task_data)
	{
		if (simulation.processor_count == 0)
		{
			for (size_t index = 0; index < task_data.count; index++)
			{
				task_data.callback(simulation, index);
			}
		}
		else
		{
			simulation.thread_mode = true;

			godot::WorkerThreadPool* thread_pool = godot::WorkerThreadPool::get_singleton();

			uint64_t id = thread_pool->add_native_group_task(
				&TaskCallback,
				&task_data,
				task_data.count,
				simulation.processor_count,
				simulation.high_priority);

			thread_pool->wait_for_group_task_completion(id);

			simulation.thread_mode = false;
		}
	}

	// Run multiple task groups at the same time and wait for them all after all have been started
	void SimulationDoMultitasks(Simulation& simulation, Span<TaskData> task_data)
	{
		if (task_data.Size() == 1)
		{
			SimulationDoTasks(simulation, *task_data.Data());
		}
		else if (simulation.processor_count == 0)
		{
			for (TaskData& data : task_data)
			{
				SimulationDoTasks(simulation, data);
			}
		}
		else
		{
			simulation.thread_mode = true;

			godot::WorkerThreadPool* thread_pool = godot::WorkerThreadPool::get_singleton();

			GrowingSmallVector<uint64_t, 16> ids;

			for (TaskData& data : task_data)
			{
				uint64_t id = thread_pool->add_native_group_task(
					&TaskCallback,
					&data,
					data.count,
					simulation.processor_count,
					simulation.high_priority);

				ids.push_back(id);
			}

			for (size_t index = 0; index < task_data.Size(); index++)
			{
				thread_pool->wait_for_group_task_completion(ids[index]);
			}

			simulation.thread_mode = false;
		}
	}

	void SimulationUniverseWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.universe_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		debugrender::WorldUpdate(simulation, world);
		universe::WorldUpdate(simulation, world);
	}

	void SimulationGalaxyWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.galaxy_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		debugrender::WorldUpdate(simulation, world);
		galaxy::WorldUpdate(simulation, world);
	}

	void SimulationStarSystemWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.star_system_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		debugrender::WorldUpdate(simulation, world);
	}

	void SimulationPlanetWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.planet_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		debugrender::WorldUpdate(simulation, world);
	}

	void SimulationSpaceStationWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.space_station_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		debugrender::WorldUpdate(simulation, world);
	}

	void SimulationSpaceShipWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.space_ship_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		debugrender::WorldUpdate(simulation, world);
	}

	void SimulationVehicleWorldUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::WorldPtr world = simulation.vehicle_type.worlds[index];

		DEBUG_THREAD_CHECK_WRITE(world.Data());

		spatial3d::WorldUpdate(simulation, world);
		debugrender::WorldUpdate(simulation, world);
	}

	// Do parallel spatial world updates
	void SimulationWorldUpdate(Simulation& simulation)
	{
		TaskData world_tasks[7] = {
			{ simulation, &SimulationUniverseWorldUpdateTask, simulation.universe_type.worlds.size() },
			{ simulation, &SimulationGalaxyWorldUpdateTask, simulation.galaxy_type.worlds.size() },
			{ simulation, &SimulationStarSystemWorldUpdateTask, simulation.star_system_type.worlds.size() },
			{ simulation, &SimulationPlanetWorldUpdateTask, simulation.planet_type.worlds.size() },
			{ simulation, &SimulationSpaceStationWorldUpdateTask, simulation.space_station_type.worlds.size() },
			{ simulation, &SimulationSpaceShipWorldUpdateTask, simulation.space_ship_type.worlds.size() },
			{ simulation, &SimulationVehicleWorldUpdateTask, simulation.vehicle_type.worlds.size() },
		};

		SimulationDoMultitasks(simulation, world_tasks);
	}

	void SimulationUniverseScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.universe_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		debugrender::ScaleUpdate(simulation, scale);
		universe::ScaleUpdate(simulation, scale);
	}

	void SimulationGalaxyScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.galaxy_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		debugrender::ScaleUpdate(simulation, scale);
		galaxy::ScaleUpdate(simulation, scale);
	}

	void SimulationStarSystemScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.star_system_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		debugrender::ScaleUpdate(simulation, scale);
	}

	void SimulationPlanetScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.planet_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		debugrender::ScaleUpdate(simulation, scale);
	}

	void SimulationSpaceStationScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.space_station_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		debugrender::ScaleUpdate(simulation, scale);
	}

	void SimulationSpaceShipScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.space_ship_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		debugrender::ScaleUpdate(simulation, scale);
	}

	void SimulationVehicleScaleUpdateTask(Simulation& simulation, size_t index)
	{
		spatial3d::ScalePtr scale = simulation.vehicle_type.scales[index];

		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		spatial3d::ScaleUpdate(simulation, scale);
		debugrender::ScaleUpdate(simulation, scale);
	}

	// Do parallel spatial scale updates
	void SimulationScaleUpdate(Simulation& simulation)
	{
		TaskData scale_tasks[7] = {
			{ simulation, &SimulationUniverseScaleUpdateTask, simulation.universe_type.scales.size() },
			{ simulation, &SimulationGalaxyScaleUpdateTask, simulation.galaxy_type.scales.size() },
			{ simulation, &SimulationStarSystemScaleUpdateTask, simulation.star_system_type.scales.size() },
			{ simulation, &SimulationPlanetScaleUpdateTask, simulation.planet_type.scales.size() },
			{ simulation, &SimulationSpaceStationScaleUpdateTask, simulation.space_station_type.scales.size() },
			{ simulation, &SimulationSpaceShipScaleUpdateTask, simulation.space_ship_type.scales.size() },
			{ simulation, &SimulationVehicleScaleUpdateTask, simulation.vehicle_type.scales.size() },
		};

		SimulationDoMultitasks(simulation, scale_tasks);
	}

	void SimulationEntityUpdateTask(Simulation& simulation, size_t index)
	{
		entity::WRef entity = simulation.updating_entities[index];

		DEBUG_THREAD_CHECK_WRITE(entity.Data());

		simulation.entity_factory.DoEvent(PolyEvent::TaskUpdate, entity);
	}

	// Run all entities in parallel doing entity specific code
	void SimulationEntityUpdate(Simulation& simulation)
	{
		TaskData entity_task = { simulation, &SimulationEntityUpdateTask, simulation.updating_entities.size() };

		SimulationDoTasks(simulation, entity_task);
	}
	
	void SimulationWorkerUpdateTask(Simulation& simulation, size_t index)
	{
		simulation::SetContext(simulation.thread_contexts[index]);

		simulation::WorkerUpdate(simulation, index);
		rendering::WorkerUpdate(simulation, index);
		debugrender::WorkerUpdate(simulation, index);
		spatial3d::WorkerUpdate(simulation, index);
		universe::WorkerUpdate(simulation, index);
		galaxy::WorkerUpdate(simulation, index);
	}

	// Run worker tasks in parallel for systems that need them. There is usually one per CPU core
	void SimulationWorkerUpdate(Simulation& simulation)
	{
		TaskData worker_task{ simulation, &SimulationWorkerUpdateTask, simulation.worker_count };

		SimulationDoTasks(simulation, worker_task);
	}

	bool IsSimulationUnloadDone(Simulation& simulation)
	{
		return	simulation::IsUnloadDone(simulation) &&
				debugrender::IsUnloadDone(simulation) &&
				spatial3d::IsUnloadDone(simulation) &&
				universe::IsUnloadDone(simulation) &&
				galaxy::IsUnloadDone(simulation);
	}

	// Do singlethreaded update
	void SimulationSingleUpdate(Simulation& simulation)
	{
		simulation::Update(simulation);
		rendering::Update(simulation);
		debugrender::Update(simulation);
		spatial3d::Update(simulation);
		universe::Update(simulation);
		galaxy::Update(simulation);

		for (ThreadContext& context : simulation.thread_contexts)
		{
			for (entity::WRef entity : context.load_commands)
			{
				simulation.entity_factory.DoEvent(PolyEvent::BeginLoad, entity);
			}

			context.load_commands.clear();

			for (entity::WRef entity : context.unload_commands)
			{
				simulation.entity_factory.DoEvent(PolyEvent::BeginUnload, entity);
			}

			context.unload_commands.clear();
		}

		for (entity::WRef entity : simulation.updating_entities)
		{
			simulation.entity_factory.DoEvent(PolyEvent::MainUpdate, entity);
		}
	}

	void SimulationInitialize(Simulation& simulation)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation); // Should be called singlethreaded
		
		godot::print_line("Initializing simulation");

		simulation::Initialize(simulation);
		rendering::Initialize(simulation);
		debugrender::Initialize(simulation);
		spatial3d::Initialize(simulation);
		universe::Initialize(simulation);
		galaxy::Initialize(simulation);
	}

	void SimulationUnload(Simulation& simulation)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation); // Should be called singlethreaded

		godot::print_line("Unloading simulation");

		simulation.unloading = true;

		// Unload all entities in the world. The entities memory still exists while there are references
		std::vector<entity::Ref> updating_entities = std::move(simulation.updating_entities);
		for (entity::WRef entity : updating_entities)
		{
			SimulationUnloadEntity(simulation, entity);
		}
		updating_entities.clear();

		// Loop update while unloading
		while (!IsSimulationUnloadDone(simulation))
		{
			SimulationUpdate(simulation);
		}

		simulation.unloading = false;
	}

	void SimulationUninitialize(Simulation& simulation)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation); // Should be called singlethreaded

		// Unload modules
		galaxy::Uninitialize(simulation);
		universe::Uninitialize(simulation);
		spatial3d::Uninitialize(simulation);
		debugrender::Uninitialize(simulation);
		rendering::Uninitialize(simulation);
		simulation::Uninitialize(simulation);

		// Finally cleanup the entity factory after all references should have been removed
		simulation.entity_factory.Cleanup();
		DEBUG_ASSERT(simulation.entity_factory.GetCount() == 0, "We should have destroyed all entities");
	}

	void SimulationUpdate(Simulation& simulation)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation); // Should be called singlethreaded

		SimulationWorldUpdate(simulation);
		SimulationScaleUpdate(simulation);
		SimulationEntityUpdate(simulation);
		SimulationWorkerUpdate(simulation);
		SimulationSingleUpdate(simulation);

		simulation.entity_factory.Cleanup();
	}

	entity::Ref SimulationCreateEntity(Simulation& simulation, UUID id, entity::TypeID types)
	{
		entity::Ref entity = simulation.entity_factory.GetPoly(id);

		simulation.entity_factory.AddTypes(entity.GetID(), types);

		simulation::GetContext().load_commands.push_back(entity.Reference());

		return entity;
	}

	void SimulationUnloadEntity(Simulation& simulation, entity::WRef entity)
	{
		simulation::GetContext().unload_commands.push_back(entity::Ref(entity));
	}
}