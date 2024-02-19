#pragma once

#include "SimulationMessager.h"

#include "Util/Callback.h"

#include <entt/entity/registry.hpp>

#include <vector>

namespace sim
{
	// Simulation data root
	struct Simulation
	{
		using BuilderPtr = std::unique_ptr<SimulationBuilder>;

		using Registry = entt::registry;

		using Globals = entt::registry::context;

		using SimulationApplicator = cb::Callback<void(Simulation&)>;

		using SystemDeleteStorage = std::vector<SimulationApplicator>;
		using UUIDToEntityStorage = robin_hood::unordered_map<UUID, entt::entity>;
		using EntityToUUIDStorage = robin_hood::unordered_map<entt::entity, UUID>;

		Simulation(SimulationServer& server, UUID id);
		~Simulation();

		SimulationServer&		server;

		SimulationMessager		messager;

		UUIDGenerator			uuid_gen;

		UUIDToEntityStorage     uuid_to_entity;
		EntityToUUIDStorage     entity_to_uuid;

		SystemDeleteStorage		system_shutdowns;

		Registry				registry;

		Globals					globals;
	};
}