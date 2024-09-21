#include "Simulation.h"

#include "Simulation/CommandQueue.h"

#include "Util/Debug.h"
#include "Util/PropertyMacros.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/type_info.hpp>

#define BIND_METHOD godot::ClassDB::bind_method

namespace voxel_game
{
	const size_t k_simulation_ticks_per_second = 20;

	Simulation::Simulation()
	{}

	Simulation::~Simulation()
	{
		DEBUG_ASSERT(m_load_state.load(std::memory_order_acquire) == LOAD_STATE_UNLOADED, "We did not fully unload the simuation before destroying it");

		// Join the thread just in case
		if (IsThreaded())
		{
			m_thread.join();
		}
	}

	bool Simulation::IsThreaded()
	{
		return m_thread.joinable();
	}

	void Simulation::StartSimulation(ThreadMode thread_mode)
	{
		if (m_load_state.load(std::memory_order_acquire) != LOAD_STATE_UNLOADED)
		{
			DEBUG_PRINT_ERROR("This galaxy should not be loaded when we start");
			return;
		}

		if (!OnSimulationLoading())
		{
			return;
		}

		m_load_state.store(LOAD_STATE_LOADING, std::memory_order_release);
		emit_signal("load_state_changed", LOAD_STATE_LOADING);

		if (thread_mode == THREAD_MODE_MULTI_THREADED)
		{
			if (IsThreaded())
			{
				m_thread.join();
			}

			m_thread = std::thread(&Simulation::ThreadLoop, this);
		}
	}

	void Simulation::FinishedLoading()
	{
		m_load_state.store(LOAD_STATE_LOADED, std::memory_order_release);
		emit_signal("load_state_changed", LOAD_STATE_LOADED);

		OnSimulationLoaded();
	}

	void Simulation::StopSimulation()
	{
		LoadState load_state = m_load_state.load(std::memory_order_acquire);

		if (load_state == LOAD_STATE_UNLOADED)
		{
			DEBUG_PRINT_ERROR("This galaxy shouldn't be unloaded if we want to start unloading");
			return;
		}

		if (load_state == LOAD_STATE_UNLOADING) // We are already unloading
		{
			return;
		}

		OnSimulationUnloading();

		m_load_state.store(LOAD_STATE_UNLOADING, std::memory_order_release);
		emit_signal("load_state_changed", LOAD_STATE_UNLOADING);
	}

	void Simulation::FinishedUnloading()
	{
		DEBUG_ASSERT(m_load_state.load(std::memory_order_acquire) == LOAD_STATE_UNLOADING, "This simulation should not be loaded or uninitialized when we uninitialize");

		m_load_state.store(LOAD_STATE_UNLOADED, std::memory_order_release);
		emit_signal("load_state_changed", LOAD_STATE_UNLOADED);

		// First stop our thread
		if (IsThreaded())
		{
			m_thread.join();
		}

		OnSimulationUnloaded();
	}

	bool Simulation::Progress(real_t delta)
	{
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
		while (m_load_state.load(std::memory_order_acquire) != LOAD_STATE_UNLOADED)
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
	}

	bool Simulation::OnSimulationLoading()
	{
		bool should_load;
		GDVIRTUAL_CALL(_simulation_loading, should_load);
		return should_load;
	}

	void Simulation::OnSimulationLoaded()
	{
		GDVIRTUAL_CALL(_simulation_loaded);
	}

	void Simulation::OnSimulationUnloading()
	{
		GDVIRTUAL_CALL(_simulation_unloading);
	}

	void Simulation::OnSimulationUnloaded()
	{
		GDVIRTUAL_CALL(_simulation_unloaded);
	}

	bool Simulation::DoSimulationProgress(real_t delta)
	{
		bool keep_running;
		GDVIRTUAL_CALL(_simulation_progress, delta, keep_running);
		return keep_running;
	}

	void Simulation::DoSimulationThreadProgress()
	{
		GDVIRTUAL_CALL(_simulation_thread_progress);
	}

	void Simulation::_bind_methods()
	{
		BIND_ENUM_CONSTANT(THREAD_MODE_SINGLE_THREADED);
		BIND_ENUM_CONSTANT(THREAD_MODE_MULTI_THREADED);

		BIND_ENUM_CONSTANT(LOAD_STATE_LOADING);
		BIND_ENUM_CONSTANT(LOAD_STATE_LOADED);
		BIND_ENUM_CONSTANT(LOAD_STATE_UNLOADING);
		BIND_ENUM_CONSTANT(LOAD_STATE_UNLOADED);

		BIND_METHOD(godot::D_METHOD("start_simulation", "thread_mode"), &Simulation::StartSimulation);
		BIND_METHOD(godot::D_METHOD("stop_simulation"), &Simulation::StopSimulation);
		BIND_METHOD(godot::D_METHOD("finished_loading"), &Simulation::FinishedLoading);
		BIND_METHOD(godot::D_METHOD("finished_unloading"), &Simulation::FinishedUnloading);
		BIND_METHOD(godot::D_METHOD("is_threaded"), &Simulation::IsThreaded);
		BIND_METHOD(godot::D_METHOD("progress", "delta"), &Simulation::Progress);

		GDVIRTUAL_BIND(_simulation_loading, "thread_mode");
		GDVIRTUAL_BIND(_simulation_loaded);
		GDVIRTUAL_BIND(_simulation_unloading);
		GDVIRTUAL_BIND(_simulation_unloaded);
		GDVIRTUAL_BIND(_simulation_progress, "delta");
		GDVIRTUAL_BIND(_simulation_thread_progress);

		ADD_SIGNAL(godot::MethodInfo("load_state_changed", ENUM_PROPERTY("state", Simulation::LoadState)));
	}
}