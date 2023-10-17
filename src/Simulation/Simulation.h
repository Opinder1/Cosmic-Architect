#pragma once

#include "Time.h"

#include "Message/MessageSender.h"

#include "Event/EventDispatcher.h"

#include "Thread/ThreadMessager.h"

#include "Util/Callback.h"

#include <TKRZW/tkrzw_thread_util.h>

#include <entt/entt.hpp>

#include <atomic>
#include <thread>
#include <vector>

namespace sim
{
	class Context;
	class System;

	struct MessagerStopEvent;
	struct RequestStopMessage;
	struct RequestStopDeleteMessage;

	// Simulates an environment by ticking every set timestep.
	// Has SimulationSystems that define its behaviour. The systems allow dynamic adding of behavior.
	// Can run in a seperate thread and handle messages it recieves.
	class Simulation final : public ThreadMessager
	{
		friend class SimulationServer;

		using SystemStorage = std::vector<std::unique_ptr<System>>;

	public:
		using SystemEmitter = cb::TempCallback<std::unique_ptr<System>(Simulation&)>;

	public:
		Simulation(UUID id, double ticks_per_second);
		~Simulation();

		// Is this simulation currently running (not including short time when stopping)
		bool IsRunning() const;

		// Get the amount of ticks this simulation aims to complete per second (only relevant in threaded mode)
		double GetTicksPerSecond() const;

		// Get the total amount of ticks this simulation has done since creation
		size_t GetTotalTicks() const;
		size_t GetTotalTicks();

		// Get the amount of ticks since Start() was called
		size_t GetCurrentTicks() const;
		size_t GetCurrentTicks();

		// Get the total time this simulation has been running from creation not regarding this current run
		Clock::duration GetTotalRunTime() const;
		Clock::duration GetTotalRunTime();

		// Get the time that Start() was called
		Clock::time_point GetCurrentStartTime() const;
		Clock::time_point GetCurrentStartTime();

		// Get the time we have been running since Start() was called
		Clock::duration GetCurrentRunTime() const;
		Clock::duration GetCurrentRunTime();

		// Link to another simulation using its uuid
		bool Link(UUID simulation);

		// Unlink from another simulation using its uuid
		bool Unlink(UUID simulation);

		// Get the entity registry
		entt::registry& GetRegistry();

		// Generate a uuid
		UUID GenerateUUID();

	private:
		// Add a system to this simulation. Don't call when the simulation is running
		void AddSystem(const SystemEmitter& emitter);

		// Start this simulation
		void Start();

		// Stop this simulation (call from SimulationServer)
		void Stop();

		// Stop this simulation and delete itself when stopped (call from SimulationServer)
		void StopAndDelete();

	private:
		// Main thread loop of this simulation that manages ticks and timings for the owner thread
		void ThreadLoop();

		// Internal start that is called by the thread loop
		void InternalStart();

		// Internal tick that is called by the thread loop
		void InternalTick(Clock::time_point tick_start);

		// Internal stop that is called by the thread loop
		void InternalStop();

		void OnMessagerStop(const MessagerStopEvent& event);

		void OnRequestStop(const RequestStopMessage& event);

		void OnRequestStopDelete(const RequestStopDeleteMessage& event);

	private:
		// Threading
		std::thread						m_thread;

		std::atomic_bool				m_running; // Is this simulation running
		std::atomic_bool				m_delete_on_stop; // Should we delete ourself when we stop

		// Options
		const double					m_ticks_per_second; // Ticks this simulation aims to execute per second
		const Clock::duration			m_time_per_tick; // Max time per tick this simulation aims to take

		// Timings
		mutable tkrzw::SpinSharedMutex	m_timings_mutex;

		size_t							m_total_ticks; // Total number of ticks this simulation has executed
		size_t							m_current_ticks; // Current number of ticks this simulation has executed since calling Start()

		Clock::time_point				m_current_start_time; // Time when Start() was called
		Clock::time_point				m_last_tick_time; // The exact time the last tick began

		Clock::duration					m_total_run_time; // Total amount of time this simulation has run
		Clock::duration					m_current_run_time; // Current time this simulation has run since calling Start()

		// Data
		entt::registry					m_registry;

		SystemStorage					m_systems;

		Random							m_uuid_gen;
	};
}