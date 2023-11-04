#include "Simulation.h"
#include "SimulationServer.h"
#include "System.h"
#include "Events.h"

#include "Message/Message.h"

#include "Util/Debug.h"

#include <godot_cpp/variant/utility_functions.hpp>

#include <chrono>

namespace sim
{
	Simulation::Simulation(UUID id, double ticks_per_second) :
		ThreadMessager(id),
		m_running(false),
		m_stopping_cached(false),
		m_ticks_per_second(ticks_per_second),
		m_time_per_tick(std::chrono::duration_cast<Clock::duration>(1s / m_ticks_per_second)),
		m_current_ticks(0),
		m_total_ticks(0),
		m_total_run_time(0),
		m_current_run_time(0)
	{
		if (m_ticks_per_second < 0.0)
		{
			DEBUG_PRINT_ERROR("The ticks per second should be positive or 0");
			return;
		}

		Subscribe(cb::Bind<&Simulation::OnRequestStop>(*this));
		Subscribe(cb::Bind<&Simulation::OnMessagerStop>(*this));
	}

	Simulation::~Simulation()
	{
		if (m_running)
		{
			DEBUG_PRINT_ERROR("This simulation should have been stopped before destroying it");
			Stop();
		}

		m_thread.join();

		Unsubscribe(cb::Bind<&Simulation::OnMessagerStop>(*this));
		Unsubscribe(cb::Bind<&Simulation::OnRequestStop>(*this));
	}
	
	void Simulation::AddSystem(const SystemEmitter& emitter)
	{
		AddSystem(std::move(emitter(*this)));
	}

	void Simulation::AddSystem(std::unique_ptr<System>&& system)
	{
		if (m_running)
		{
			DEBUG_PRINT_ERROR("This simulation should not be running when trying to add a system");
			return;
		}

		m_systems.push_back(std::move(system));
	}

	void Simulation::Start()
	{
		DEBUG_ASSERT(!ObjectOwned(), "This simulation should not be owned by a thread when start is called on it");

		if (m_running)
		{
			DEBUG_PRINT_ERROR("This simulation should not be running when trying to start it");
			return;
		}

		m_thread = std::thread(&Simulation::ThreadLoop, this);
	}

	void Simulation::Stop()
	{
		DEBUG_ASSERT(ObjectOwned(), "This simulation should be owned by a thread when stop is called on it");

		if (!m_running)
		{
			DEBUG_PRINT_ERROR("This simulation should be running when trying to stop it");
			return;
		}

		if (ThreadOwnsObject())
		{
			PostEvent(SimulationRequestStopMessage(*this));
		}
		else
		{
			PostMessageFromUnattested(std::make_shared<SimulationRequestStopMessage>(*this)); // Queue a message to stop this simulation
		}
	}

	bool Simulation::IsRunning() const
	{
		return m_running;
	}

	bool Simulation::IsStoppingCached() const
	{
		return m_stopping_cached;
	}

	double Simulation::GetTicksPerSecond() const
	{
		return m_ticks_per_second;
	}

	size_t Simulation::GetTotalTicks() const
	{
		std::shared_lock lock(m_timings_mutex);

		return m_total_ticks;
	}

	size_t Simulation::GetTotalTicks()
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called non const getter without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return m_total_ticks;
	}

	size_t Simulation::GetCurrentTicks() const
	{
		std::shared_lock lock(m_timings_mutex);

		return m_current_ticks;
	}

	size_t Simulation::GetCurrentTicks()
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called non const getter without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return m_current_ticks;
	}

	Clock::duration Simulation::GetTotalRunTime() const
	{
		std::shared_lock lock(m_timings_mutex);

		return m_total_run_time;
	}

	Clock::duration Simulation::GetTotalRunTime()
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called non const getter without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return m_total_run_time;
	}

	Clock::time_point Simulation::GetCurrentStartTime() const
	{
		std::shared_lock lock(m_timings_mutex);

		return m_current_start_time;
	}

	Clock::time_point Simulation::GetCurrentStartTime()
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called non const getter without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return m_current_start_time;
	}

	Clock::duration Simulation::GetCurrentRunTime() const
	{
		std::shared_lock lock(m_timings_mutex);

		return m_current_run_time;
	}

	Clock::duration Simulation::GetCurrentRunTime()
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called non const getter without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return m_current_run_time;
	}

	bool Simulation::Link(UUID simulation_id)
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called link without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return SimulationServer::GetSingleton()->ApplyToSimulation(simulation_id, [this](Simulation& simulation)
		{
			LinkMessager(simulation);
		});
	}

	bool Simulation::Unlink(UUID simulation_id)
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called unlink without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return SimulationServer::GetSingleton()->ApplyToSimulation(simulation_id, [this](Simulation& simulation)
		{
			UnlinkMessager(simulation);
		});
	}

	entt::registry& Simulation::Registry()
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called non const getter without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return m_registry;
	}

	UUID Simulation::GenerateUUID()
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called non const generator without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return UUID(m_uuid_gen);
	}

	void Simulation::ThreadLoop()
	{
		InternalStart();

		// The ideal time for the next tick to begin
		Clock::time_point next_tick_start = Clock::now();

		while (m_running)
		{
			Clock::time_point tick_start = Clock::now();

			// If the ticks per second is 0 then don't do any time processing
			if (m_ticks_per_second > 0.0f)
			{
				// Get the ideal time for the next frame
				next_tick_start = next_tick_start + m_time_per_tick;

				// If we actually passed the next frames start time then set the next variable to now and skip sleeping
				if (tick_start > next_tick_start)
				{
					next_tick_start = tick_start;
				}
				else
				{
					std::this_thread::sleep_until(next_tick_start);

					// Get now again after sleeping
					tick_start = Clock::now();
				}
			}

			// TODO : Remove
			if (m_total_ticks % size_t(m_ticks_per_second * 10) == 0)
			{
				auto id = m_thread.get_id();
				godot::UtilityFunctions::print(godot::vformat("Thread %d on tick %d", *(unsigned int*)&id, m_total_ticks));
			}

			m_stopping_cached = IsStopping();

			InternalTick(tick_start);
		}

		InternalStop();
	}

	void Simulation::InternalStart()
	{
		DipatcherStart();

		// Set running to true before the start callback so that IsRunning() works
		m_running = true;

		{
			std::unique_lock lock(m_timings_mutex);

			m_current_start_time = Clock::now();
			m_current_ticks = 0;
			m_last_tick_time = Clock::now();
		}

		PostEvent(SimulationStartEvent());
	}

	void Simulation::InternalTick(Clock::time_point tick_start)
	{
		Clock::duration timestep;

		{
			std::unique_lock lock(m_timings_mutex);

			timestep = tick_start - m_last_tick_time;
			m_total_run_time += timestep;
			m_current_run_time += timestep;
			m_last_tick_time = tick_start;

			m_total_ticks++;
			m_current_ticks++;
		}

		// Process incomming messages first in case there is an unlink message in
		// there to remove the queue. We don't want to send to a messager that no longer exists
		ProcessIncommingMessages();

		PostEvent(SimulationTickEvent(tick_start, timestep));

		// Send all outgoing messages that may have been queued during the tick event.
		ProcessOutgoingMessages();
	}

	void Simulation::InternalStop()
	{
		PostEvent(SimulationStopEvent());

		{
			std::unique_lock lock(m_timings_mutex);

			m_current_start_time = Clock::time_point{};
			m_current_ticks = 0;
			m_current_run_time = Clock::duration{};
		}
	}

	void Simulation::OnRequestStop(const SimulationRequestStopMessage& event)
	{
		DipatcherRequestStop(); // Send a unlink message too all that are linked. Once they all reply m_running is set to false.
	}

	void Simulation::OnMessagerStop(const MessagerStopEvent& event)
	{
		m_running = false; // Set running to false that stops the thread
	}

	void Simulation::OnAttemptFreeMemory(const AttemptFreeMemoryEvent& event)
	{
		m_registry.compact();
	}
}