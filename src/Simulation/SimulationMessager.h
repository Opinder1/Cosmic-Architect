#pragma once

#include "Thread/ThreadMessager.h"

#include "Util/Time.h"

#include <atomic>
#include <thread>

namespace sim
{
	struct Simulation;
	class SimulationServer;
	class SimulationBuilder;

	struct SimulationRequestStopMessage;
	struct SimulationThreadAcquireMessage;
	struct MessagerStopEvent;

	// Simulates an environment by ticking every set timestep.
	// Has SimulationSystems that define its behaviour. The systems allow dynamic adding of behavior.
	// Can run in a seperate thread and handle messages it recieves.
	class SimulationMessager final : public ThreadMessager
	{
		friend class SimulationServer;

	public:
		SimulationMessager(SimulationServer& server, UUID id, EventDispatcher& dispatcher);
		~SimulationMessager();

		void SetTargetTicksPerSecond(double ticks_per_second);

		// Is this simulation currently starting up
		bool IsStarting() const;

		// Is this simulation currently running (includes starting and stopping)
		bool IsRunning() const;

		// Is this simulation being ticked manually by an external thread that succesfully acquired this simulation
		bool IsExternallyTicked() const;

		// Is this simulation being ticked by the calling thread
		bool ThreadOwnsSimulation() const;

		// Get the amount of ticks this simulation aims to complete per second (only relevant in threaded mode)
		double GetTicksPerSecond() const;

		// Get the total amount of ticks this simulation has done since creation
		size_t GetTotalTicks();

		// Get the amount of ticks since Start() was called
		size_t GetCurrentTicks();

		// Get the total time this simulation has been running from creation not regarding this current run
		Clock::duration GetTotalRunTime();

		// Get the time that Start() was called
		Clock::time_point GetCurrentStartTime();

		// Get the time we have been running since Start() was called
		Clock::duration GetCurrentRunTime();

		// Manually tick from the thread that acquired this simulation
		bool ManualTick();

		// Link to another simulation using its uuid
		void Link(UUID simulation);

		// Unlink from another simulation using its uuid
		void Unlink(UUID simulation);

		// Send event to messager. Run by the owner thread
		template<class EventT>
		void PostEventFromUnattested(const EventT& event)
		{
			static_assert(std::is_base_of_v<Event, EventT>);

			return PostEventFromUnattestedGeneric(static_cast<const Event&>(event), GetEventType<EventT>());
		}

		// Send event to messager. Run by the owner thread
		void PostEventFromUnattestedGeneric(const Event& event, Event::Type event_type);

	private:
		SimulationServer& GetSimulationServer();

		// Start this simulation
		bool Start();

		// Start this simulation and build
		bool BuildStart(std::unique_ptr<SimulationBuilder>&& builder, Simulation& simulation);

		// Stop this simulation (call from SimulationServer)
		void Stop();

		// Acquire the simulation for the current thread
		bool ThreadAcquire();

		// Release the simulation from the current thread
		bool ThreadRelease();

		// Main thread loop of this simulation that manages ticks and timings for the owner thread
		void ThreadLoop();

		// Internal start that is called by the thread loop
		void InternalStart();

		// Internal tick that is called by the thread loop
		void InternalTick(Clock::time_point tick_start);

		// Internal stop that is called by the thread loop
		void InternalStop();

	private:
		void OnRequestStop(const SimulationRequestStopMessage& event);

		void OnThreadAcquire(const SimulationThreadAcquireMessage& event);

		void OnMessagerStop(const MessagerStopEvent& event);

	private:
		// Threading
		std::thread						m_internal_thread;
		std::atomic_bool				m_thread_paused; // Is our internal thread paused
		std::atomic_bool				m_keep_looping; // Should we continue to loop our internal thread

		std::atomic_bool				m_starting; // Is this simulation starting
		std::atomic_bool				m_running; // Is this simulation running

		std::thread::id					m_external_thread; // The external thread that will be ticking this simulation if the internal thread is paused

		// Options
		double							m_ticks_per_second; // Ticks this simulation aims to execute per second
		Clock::duration					m_time_per_tick; // Max time per tick this simulation aims to take

		// Timings
		size_t							m_total_ticks; // Total number of ticks this simulation has executed
		size_t							m_current_ticks; // Current number of ticks this simulation has executed since calling Start()

		Clock::time_point				m_current_start_time; // Time when Start() was called
		Clock::time_point				m_last_tick_time; // The exact time the last tick began

		Clock::duration					m_total_run_time; // Total amount of time this simulation has run
		Clock::duration					m_current_run_time; // Current time this simulation has run since calling Start()
	};
}