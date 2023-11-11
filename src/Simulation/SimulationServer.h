#pragma once

#include "UUID.h"

#include "Message/MessageRegistry.h"

#include "Util/Callback.h"

#include <TKRZW/tkrzw_thread_util.h>

#include <robin_hood/robin_hood.h>

namespace sim
{
	class Simulation;
	class System;

	class SimulationServer : public MessageRegistry
	{
	public:
		using SystemEmitter = cb::Callback<std::unique_ptr<System>(Simulation&)>;
		using SimulationApplicator = cb::Callback<void(Simulation&)>;

		using SimulationStorage = robin_hood::unordered_flat_map<UUID, std::unique_ptr<Simulation>>;
		using DeleteQueue = std::vector<std::unique_ptr<Simulation>>;

	public:
		static void Initialize();
		static void Uninitialize();
		static SimulationServer* GetSingleton();

	public:
		explicit SimulationServer();
		~SimulationServer();

		// Enable the ability to use networking features
		void StartNetworking();

		// Create a new simulation and receive a handle to it
		UUID CreateSimulation(double ticks_per_second, bool add_standard_systems);

		// Add a system to a simulation
		template<class SystemT, class... Args>
		void AddSystem(UUID id, Args&&... args)
		{
			AddSystem(id, [=](Simulation& simulation) -> std::unique_ptr<System>{ return std::make_unique<SystemT>(simulation, std::forward<Args>(args)...); });
		}

		// Start this simulation
		void StartSimulation(UUID id, bool manually_tick);

		// Stop this simulation
		void StopSimulation(UUID id);

		// Manually tick this simulation
		bool TickSimulation(UUID id);

		// Send a direct message to this simulation. Ideally messages should be sent between linked simulations
		void SendMessage(UUID id, const MessagePtr& message);

		// Send direct messages to this simulation. Ideally messages should be sent between linked simulations
		void SendMessages(UUID id, const MessageQueue& messages);

		// Is this the handle of a valid simulation
		bool IsSimulation(UUID id);

		// Is this simulation currently running. It can be stopping and still running
		bool IsSimulationRunning(UUID id);

		// Is this simulation manually being ticked by a thread.
		bool IsSimulationManuallyTicked(UUID id);

		// Is this simulation currently unlinking with all its peers
		bool IsSimulationStopping(UUID id);

		// Run some code on a simulation. (Only use this if you know what you are doing)
		bool ApplyToSimulation(UUID id, const SimulationApplicator& callback);

		// Delete this simulation. If the simulation is running it will be stopped and then deleted. The id handle is imediately invalid
		void DeleteSimulation(UUID id);

	private:
		// Add a system using an emmiter callback
		void AddSystem(UUID id, const SystemEmitter& emitter);

		// The main loop that the deleter thread will run
		void DeleteThreadFunc();

	private:
		static std::unique_ptr<SimulationServer> s_singleton;

		UUID m_network_simulation;

		std::thread m_deleter_thread; // Thread that deletes simulations added to the queue

		tkrzw::SpinSharedMutex m_mutex; // Mutex to protect the stopped flag, the simulation storage and the delete queue

		bool m_stopped; // Have we called the SimulationServer destructor then stop creating new simulations
		SimulationStorage m_simulations; // Simulations
		DeleteQueue m_delete_queue; // Simulations that are being deleted
	};
}