#pragma once

#include "UUID.h"

#include "Message/MessageRegistry.h"

#include "Util/Callback.h"

#include <TKRZW/tkrzw_thread_util.h>

#include <robin_hood/robin_hood.h>

namespace sim
{
	class SimulationMessager;
	struct Simulation;

	class SimulationServer : public MessageRegistry
	{
	public:
		enum class Result
		{
			True,
			False,
			Invalid
		};

		using MessagerApplicator = cb::Callback<void(SimulationMessager&)>;
		using SimulationApplicator = cb::Callback<void(Simulation&)>;

		using SimulationPtr = std::unique_ptr<Simulation>;
		using SimulationStorage = robin_hood::unordered_flat_map<UUID, SimulationPtr>;
		using DeleteQueue = std::vector<SimulationPtr>;

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

		// Delete this simulation. If the simulation is running it will be stopped and then deleted. The id handle is imediately invalid
		void DeleteSimulation(UUID id);

		// Add a system to a simulation
		template<class SystemT>
		void AddSystem(UUID id)
		{
			AddSystem(id, SystemT::OnInitialize, SystemT::OnShutdown);
		}

		// Add a system using an emmiter callback
		void AddSystem(UUID id, const SimulationApplicator& initialize, const SimulationApplicator& shutdown);

		// Start this simulation
		bool StartSimulation(UUID id);

		// Stop this simulation. Manually managed simulations are immediately in the stopping state
		void StopSimulation(UUID id);

		// Acquire a simulation that is owned and managed by this thread
		bool ThreadAcquireSimulation(UUID id);

		// Try and get a simulation we acquired if the simulation has paused its internal thread
		// Keep calling after ThreadAcquireSimulation() until we get a non null return
		SimulationMessager* TryGetAcquiredSimulation(UUID id);

		// Release a simulation that was acquired by this thread. The pointer should be released at this point
		bool ThreadReleaseSimulation(UUID id);

		// Send a direct message to this simulation. Ideally messages should be sent between linked simulations
		void SendMessage(UUID id, const MessagePtr& message);

		// Send direct messages to this simulation. Ideally messages should be sent between linked simulations
		void SendMessages(UUID id, const MessageQueue& messages);

		// Is this the handle of a valid simulation
		bool IsSimulation(UUID id);

		// Is this simulation currently running. It can be stopping and still running
		Result IsSimulationRunning(UUID id);

		// Is this simulation externally being ticked by a thread.
		Result IsSimulationExternallyTicked(UUID id);

		// Is this simulation currently unlinking with all its peers
		Result IsSimulationStopping(UUID id);

		// Run some code on a simulation. (Only use this if you know what you are doing)
		bool ApplyToSimulation(UUID id, const MessagerApplicator& callback);

		// Attempt to free memory
		void AttemptFreeMemory();

	private:
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