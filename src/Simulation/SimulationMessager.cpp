#include "SimulationMessager.h"
#include "SimulationServer.h"
#include "SimulationBuilder.h"
#include "Events.h"

#include "Event/EventDispatcher.h"

#include "Message/Message.h"

#include "Util/Debug.h"

namespace sim
{
	SimulationMessager::SimulationMessager(SimulationServer& server, UUID id, EventDispatcher& dispatcher) :
		ThreadMessager(server, id, dispatcher),
		m_running(false),
		m_keep_looping(false),
		m_thread_paused(false),
		m_ticks_per_second(0),
		m_time_per_tick(0),
		m_current_ticks(0),
		m_total_ticks(0),
		m_total_run_time(0),
		m_current_run_time(0)
	{
		GetDispatcher().Subscribe(cb::Bind<&SimulationMessager::OnRequestStop>(this));
		GetDispatcher().Subscribe(cb::Bind<&SimulationMessager::OnThreadAcquire>(this));
		GetDispatcher().Subscribe(cb::Bind<&SimulationMessager::OnMessagerStop>(this));
	}

	SimulationMessager::~SimulationMessager()
	{
		if (m_starting || m_running)
		{
			DEBUG_PRINT_ERROR("This simulation should have been stopped before destroying it");
		}

		if (m_internal_thread.joinable())
		{
			m_internal_thread.join();
		}

		GetDispatcher().Unsubscribe(cb::Bind<&SimulationMessager::OnMessagerStop>(this));
		GetDispatcher().Unsubscribe(cb::Bind<&SimulationMessager::OnThreadAcquire>(this));
		GetDispatcher().Unsubscribe(cb::Bind<&SimulationMessager::OnRequestStop>(this));
	}

	void SimulationMessager::SetTargetTicksPerSecond(double ticks_per_second)
	{
		DEBUG_ASSERT(ObjectOwned(), "This simulation should be owned by a thread when stop is called on it");

		if (m_ticks_per_second < 0.0)
		{
			DEBUG_PRINT_ERROR("The ticks per second should be positive or 0");
			ticks_per_second = 0.0;
		}

		m_ticks_per_second = ticks_per_second;

		if (ticks_per_second == 0.0)
		{
			m_time_per_tick = 0s;
		}
		else
		{
			m_time_per_tick = std::chrono::duration_cast<Clock::duration>(1s / ticks_per_second);
		}
	}

	SimulationServer& SimulationMessager::GetSimulationServer()
	{
		return static_cast<SimulationServer&>(GetMessageRegistry());
	}

	bool SimulationMessager::Start()
	{
		DEBUG_ASSERT(!ObjectOwned(), "This simulation should not be owned by a thread when start is called on it");

		if (m_starting) // We are already starting
		{
			DEBUG_PRINT_INFO("We are already starting");
			return false;
		}

		if (m_running)
		{
			DEBUG_PRINT_ERROR("This simulation should not be running when trying to start it");
			return false;
		}

		m_starting = true;

		// Start our internal thread
		m_internal_thread = std::thread([this]()
		{
			DEBUG_ASSERT(m_starting, "We should be starting");

			InternalStart();

			ThreadLoop();

			InternalStop();
		});

		return true;
	}

	bool SimulationMessager::BuildStart(std::unique_ptr<SimulationBuilder>&& builder, Simulation& simulation)
	{
		DEBUG_ASSERT(!m_starting && !m_running && m_total_run_time == 0s && m_total_ticks == 0, "This simulation should not have been started before");

		m_starting = true;

		// Start our internal thread
		m_internal_thread = std::thread([this, builder = std::move(builder), &simulation]()
		{
			DEBUG_ASSERT(m_starting, "We should be starting");
			DEBUG_ASSERT(builder != nullptr, "The builder should be a valid object");

			InternalStart();

			builder->Build(simulation);

			ThreadLoop();

			InternalStop();
		});

		return true;
	}

	void SimulationMessager::Stop()
	{
		if (m_starting)
		{
			DEBUG_PRINT_ERROR("We are currently starting this simulation. Don't stop it too quickly");
			return;
		}

		DEBUG_ASSERT(m_running, "This simulation should be owned by a thread when stop is called on it");
		DEBUG_ASSERT(ObjectOwned(), "This simulation should be owned by a thread when stop is called on it");

		// If we are being externally ticked then release the thread so that the internal thread handles the stop message
		if (IsExternallyTicked())
		{
			DEBUG_PRINT_WARN("This simulation should have stopped being externally ticked when we stopped. If not then make sure we don't call Stop() inside ManualTick()");
			ThreadRelease();
		}

		PostMessageFromUnattested(std::make_shared<SimulationRequestStopMessage>()); // Queue a message to stop this simulation
	}

	bool SimulationMessager::ThreadAcquire()
	{
		DEBUG_ASSERT(m_running, "Simulation should be running when trying to acquire");
		DEBUG_ASSERT(ObjectOwned(), "This simulation should be owned when trying to acquire");
		DEBUG_ASSERT(GetOwnerID() == m_internal_thread.get_id(), "This simulations owner should be the internal thread when trying to acquire");

		if (m_external_thread != std::thread::id{})
		{
			DEBUG_PRINT_ERROR("This simulation is already externally ticked");
			return false;
		}

		if (!m_running)
		{
			DEBUG_PRINT_ERROR("This simulation should be running when trying to acquire");
			return false;
		}

		if (IsStopping())
		{
			DEBUG_PRINT_ERROR("This simulation should not be stopping when we try to acquire");
			return false;
		}

		// Set the external thread that will acquire and send a message for the internal thread to handle
		m_external_thread = std::this_thread::get_id();
		PostMessageFromUnattested(std::make_shared<SimulationThreadAcquireMessage>());

		return true;
	}

	bool SimulationMessager::ThreadRelease()
	{
		DEBUG_ASSERT(m_running, "Simulation should be running when trying to release");
		DEBUG_ASSERT(ThreadOwnsObject(), "The simulation should be released by the owner thread");
		DEBUG_ASSERT(GetOwnerID() == m_external_thread, "The releaser should released by an external thread");

		if (!IsExternallyTicked())
		{
			DEBUG_PRINT_ERROR("This simulation should be externally ticked");
			return false;
		}

		// Unpause the internal thread and give ownership back to it
		ThreadTransferObject(m_internal_thread.get_id());
		m_external_thread = std::thread::id{};
		m_thread_paused = false;

		PostMessageFromUnattested(std::make_shared<SimulationThreadReleaseMessage>());

		return true;
	}

	bool SimulationMessager::ManualTick()
	{
		DEBUG_ASSERT(m_running, "Simulation should be running when trying to manually tick");
		DEBUG_ASSERT(ThreadOwnsObject(), "The simulation should be manually ticked by the owner thread");
		DEBUG_ASSERT(GetOwnerID() == m_external_thread, "The simulation should be manually ticked by an external thread");

		if (!IsExternallyTicked())
		{
			DEBUG_PRINT_ERROR("This simulation is not manually ticked by this thread");
			return false;
		}

		InternalTick(Clock::now());

		return true;
	}

	bool SimulationMessager::IsStarting() const
	{
		return m_starting;
	}

	bool SimulationMessager::IsRunning() const
	{
		return IsStarting() || m_running;
	}

	bool SimulationMessager::IsExternallyTicked() const
	{
		return m_thread_paused;
	}

	bool SimulationMessager::ThreadOwnsSimulation() const
	{
		return ThreadOwnsObject();
	}

	double SimulationMessager::GetTicksPerSecond() const
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called non const getter without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return IsExternallyTicked() ? 0.0f : m_ticks_per_second;
	}

	size_t SimulationMessager::GetTotalTicks()
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called non const getter without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return m_total_ticks;
	}

	size_t SimulationMessager::GetCurrentTicks()
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called non const getter without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return m_current_ticks;
	}

	Clock::duration SimulationMessager::GetTotalRunTime()
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called non const getter without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return m_total_run_time;
	}

	Clock::time_point SimulationMessager::GetCurrentStartTime()
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called non const getter without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return m_current_start_time;
	}

	Clock::duration SimulationMessager::GetCurrentRunTime()
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called non const getter without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		return m_current_run_time;
	}

	void SimulationMessager::Link(UUID simulation_id)
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called link without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		GetSimulationServer().ApplyToSimulation(simulation_id, [this](SimulationMessager& simulation)
		{
			LinkMessager(simulation);
		});
	}

	void SimulationMessager::Unlink(UUID simulation_id)
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "Called unlink without owning simulation"); // If we are the owner thread then this can't be changed while we are accessing it

		GetSimulationServer().ApplyToSimulation(simulation_id, [this](SimulationMessager& simulation)
		{
			UnlinkMessager(simulation);
		});
	}

	void SimulationMessager::PostEventFromUnattestedGeneric(const Event& event, Event::Type event_type)
	{
		DEBUG_ASSERT(m_running, "Immediate events should only be sent to a simulation that is running");
		DEBUG_ASSERT(ThreadOwnsObject(), "The owning thread should be posting immediate events for this messager");

		if (ThreadOwnsObject()) // If we own this then don't bother queuing as we won't be reading the queue right now
		{
			GetDispatcher().PostEventGeneric(event, event_type);
		}
	}

	void SimulationMessager::ThreadLoop()
	{
		DEBUG_ASSERT(ThreadOwnsObject() && GetOwnerID() == m_internal_thread.get_id(), "The internal thread should be calling internal loop");

		// The ideal time for the next tick to begin
		Clock::time_point next_tick_start = Clock::now();

		while (m_keep_looping)
		{
			Clock::time_point tick_start = Clock::now();

			// If the ticks per second is 0 then don't do any time processing
			if (m_ticks_per_second != 0.0)
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

			if (!m_thread_paused)
			{
				InternalTick(tick_start);

				if (m_thread_paused) // If the thread was paused during this tick
				{
					ThreadTransferObject(m_external_thread);
				}
			}
		}
	}

	void SimulationMessager::InternalStart()
	{
		DEBUG_ASSERT(m_starting, "We should be starting");

		MessagerStart();

		DEBUG_ASSERT(ThreadOwnsObject() && GetOwnerID() == m_internal_thread.get_id(), "The internal thread should be calling internal start");

		// Set running to true before the start callback so that IsRunning() works
		m_running = true;
		m_starting = false;
		m_keep_looping = true;

		m_current_start_time = Clock::now();
		m_current_ticks = 0;
		m_last_tick_time = Clock::now();

		GetDispatcher().PostEvent(SimulationStartEvent());
	}

	void SimulationMessager::InternalTick(Clock::time_point tick_start)
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "This simulation should be owned by the object that is calling internal tick");
		DEBUG_ASSERT(m_running, "We should be running");

		Clock::duration timestep = tick_start - m_last_tick_time;
		m_total_run_time += timestep;
		m_current_run_time += timestep;
		m_last_tick_time = tick_start;

		m_total_ticks++;
		m_current_ticks++;

		// Process incomming messages first in case there is an unlink message in
		// there to remove the queue. We don't want to send to a messager that no longer exists
		ProcessIncommingMessages();

		GetDispatcher().PostEvent(SimulationTickEvent(tick_start, timestep));

		// Send all outgoing messages that may have been queued during the tick event.
		// Do this after as they will have been queued during the tick
		ProcessOutgoingMessages();
	}

	void SimulationMessager::InternalStop()
	{
		DEBUG_ASSERT(ThreadOwnsObject() && GetOwnerID() == m_internal_thread.get_id(), "This internal thread should be calling internal stop");
		DEBUG_ASSERT(IsStopping(), "We should be stopping");
		DEBUG_ASSERT(m_running, "We should be running");

		GetDispatcher().PostEvent(SimulationStopEvent());

		m_current_start_time = Clock::time_point{};
		m_current_ticks = 0;
		m_current_run_time = Clock::duration{};

		MessagerStop();

		m_running = false;
	}

	void SimulationMessager::OnRequestStop(const SimulationRequestStopMessage& event)
	{
		DEBUG_ASSERT(ThreadOwnsObject() && GetOwnerID() == m_internal_thread.get_id(), "This simulation should be owned by the internal thread when handling a stop request");

		if (IsStopping()) // We may recieve this message more than once but that is fine since all senders will get what they requested
		{
			return;
		}

		if (!m_running)
		{
			DEBUG_PRINT_ERROR("This simulation should be running when trying to stop it");
			return;
		}

		MessagerRequestStop(); // Send a unlink message too all that are linked. Once they all reply m_running is set to false.
	}

	void SimulationMessager::OnThreadAcquire(const SimulationThreadAcquireMessage& event)
	{
		DEBUG_ASSERT(ThreadOwnsObject() && GetOwnerID() == m_internal_thread.get_id(), "This simulation should be owned by the internal thread when trying to acquire");

		if (IsStopping()) // If we are stopping then don't acquire
		{
			ThreadRelease();
			return;
		}

		m_thread_paused = true; // We pause ourself to allow the external thread to start ticking
	}

	void SimulationMessager::OnMessagerStop(const MessagerStopEvent& event)
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "This message should be sent by the owning thread");
		DEBUG_ASSERT(IsStopping(), "We should be stopping");

		m_keep_looping = false; // Set this to false so we exit the thread loop. This should be executed within a tick
	}
}