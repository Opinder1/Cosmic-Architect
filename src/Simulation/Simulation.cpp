#include "Simulation.h"

#include "Util/CommandQueue.h"
#include "Util/Debug.h"
#include "Util/PropertyMacros.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/type_info.hpp>

#include <easy/profiler.h>

#define BIND_METHOD godot::ClassDB::bind_method

namespace voxel_game
{
	const size_t k_simulation_ticks_per_second = 20;

	Simulation::Simulation()
	{
#if DEBUG
		m_owner_id = std::this_thread::get_id();
#endif
	}

	Simulation::~Simulation()
	{
		WaitUntilStopped();
	}

	bool Simulation::IsThreaded()
	{
		return m_thread.joinable();
	}

	void Simulation::StartSimulation(ThreadMode thread_mode)
	{
		DEBUG_ASSERT(m_owner_id == std::this_thread::get_id(), "StartSimulation() should be called by the thread that created the simulation");

		WaitUntilStopped();

		if (!CanSimulationStart())
		{
			return;
		}

		m_running.store(true, std::memory_order_release);

		if (thread_mode == THREAD_MODE_MULTI_THREADED)
		{
			m_thread = std::thread(&Simulation::ThreadLoop, this);
		}
		else
		{
			EASY_MAIN_THREAD;

			DoSimulationLoad();
		}
	}

	void Simulation::StopSimulation()
	{
		DEBUG_ASSERT(m_owner_id == std::this_thread::get_id(), "StopSimulation() should be called by the thread that created the simulation");

		if (!m_running.load(std::memory_order_acquire))
		{
			return;
		}

		if (!IsThreaded())
		{
			DoSimulationUnload();
		}

		m_running.store(false);
	}

	void Simulation::WaitUntilStopped()
	{
		StopSimulation();

		// Join the thread just in case it hasn't stopped just yet
		if (IsThreaded())
		{
			m_thread.join();
		}
	}

	bool Simulation::Progress(real_t delta)
	{
		DEBUG_ASSERT(m_owner_id == std::this_thread::get_id(), "Progress() should always be called by the thread that created the simulation");

		if (IsThreaded())
		{
			DoSimulationProgress(delta);

			return true; // We always keep running when threaded as the thread will stop at its own pace
		}
		else
		{
			bool keep_running = DoSimulationProgress(delta);

			// Process signals here as we don't need to defer them
			m_deferred_signals.ProcessCommands(this);

			return keep_running;
		}
	}

	void Simulation::ThreadLoop()
	{
		EASY_MAIN_THREAD;

		DEBUG_ASSERT(m_thread.get_id() == std::this_thread::get_id(), "FinishedLoading() should be called by the simulations thread");

		DoSimulationLoad();

		while (m_running.load(std::memory_order_acquire))
		{
			CommandBuffer command_buffer;
			{
				std::lock_guard lock(m_commands_mutex);
				command_buffer = std::move(m_deferred_commands);
			}

			// Process the deferred commands sent by other threads
			command_buffer.ProcessCommands(this);

			DoSimulationThreadProgress();

			// Flush signals to be executed on main thread
			CommandQueueServer::get_singleton()->AddCommands(get_instance_id(), std::move(m_deferred_signals));
		}

		DoSimulationUnload();
	}

	bool Simulation::CanSimulationStart()
	{
		bool should_load = false;
		GDVIRTUAL_CALL(_can_simulation_start, should_load);
		return should_load;
	}

	void Simulation::DoSimulationLoad()
	{
		GDVIRTUAL_CALL(_do_simulation_load);
	}

	void Simulation::DoSimulationUnload()
	{
		GDVIRTUAL_CALL(_do_simulation_unload);
	}

	bool Simulation::DoSimulationProgress(real_t delta)
	{
		bool keep_running = false;
		GDVIRTUAL_CALL(_simulation_progress, delta, keep_running);
		return keep_running;
	}

	void Simulation::DoSimulationThreadProgress()
	{
		if (!GDVIRTUAL_CALL(_simulation_thread_progress))
		{
			std::this_thread::yield(); // Avoid eating up tons of cpu running the progress loop constantly
		}
	}

	void Simulation::_bind_methods()
	{
		BIND_ENUM_CONSTANT(THREAD_MODE_SINGLE_THREADED);
		BIND_ENUM_CONSTANT(THREAD_MODE_MULTI_THREADED);

		BIND_METHOD(godot::D_METHOD("start_simulation", "thread_mode"), &Simulation::StartSimulation);
		BIND_METHOD(godot::D_METHOD("stop_simulation"), &Simulation::StopSimulation);
		BIND_METHOD(godot::D_METHOD("is_threaded"), &Simulation::IsThreaded);
		BIND_METHOD(godot::D_METHOD("progress", "delta"), &Simulation::Progress);

		GDVIRTUAL_BIND(_can_simulation_start, "thread_mode");
		GDVIRTUAL_BIND(_do_simulation_load);
		GDVIRTUAL_BIND(_do_simulation_unload);
		GDVIRTUAL_BIND(_simulation_progress, "delta");
		GDVIRTUAL_BIND(_simulation_thread_progress);

		ADD_SIGNAL(godot::MethodInfo("load_state_changed", ENUM_PROPERTY("state", Simulation::LoadState)));
	}
}