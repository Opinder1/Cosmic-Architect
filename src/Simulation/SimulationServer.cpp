#include "SimulationServer.h"

#include "Commands/CommandServer.h"

#include "Util/Debug.h"
#include "Util/GodotPropertyMacros.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/type_info.hpp>

#include <easy/profiler.h>

#define BIND_METHOD godot::ClassDB::bind_method

namespace voxel_game
{
	const size_t k_ticks_per_second = 20;

	SimulationServer::SimulationServer()
	{}

	SimulationServer::~SimulationServer()
	{
		WaitUntilStopped(); // The worker thread is using our memory so make sure its stopped before we are deleted
	}

	bool SimulationServer::IsThreaded()
	{
		return m_thread.joinable();
	}

	void SimulationServer::StartSimulation(ThreadMode thread_mode)
	{
		WaitUntilStopped();

		if (!CanSimulationStart())
		{
			return;
		}

		m_state.store(State::Loading, std::memory_order_release); // If in thread mode the thread will set to loaded

		if (thread_mode == THREAD_MODE_MULTI_THREADED)
		{
			m_thread = std::thread(&SimulationServer::ThreadLoop, this);
		}
		else
		{
			EASY_MAIN_THREAD;

			DoSimulationLoad();

			m_state.store(State::Loaded, std::memory_order_release);
		}
	}

	void SimulationServer::StopSimulation()
	{
		if (m_state.load(std::memory_order_acquire) != State::Loaded)
		{
			return;
		}

		m_state.store(State::Unloading); // If in thread mode the thread will set to unloaded

		if (!IsThreaded())
		{
			DoSimulationUnload();

			m_state.store(State::Unloaded);
		}
	}

	void SimulationServer::WaitUntilStopped()
	{
		StopSimulation();

		// Join the thread just in case it hasn't stopped just yet
		if (IsThreaded())
		{
			m_thread.join();
		}
	}

	void SimulationServer::Progress(real_t delta)
	{
		if (m_state.load(std::memory_order_acquire) == State::Loaded) // Don't progress until loaded
		{
			DoSimulationProgress(delta);
		}
	}

	void SimulationServer::ThreadLoop()
	{
		EASY_MAIN_THREAD;

		DEBUG_ASSERT(m_thread.get_id() == std::this_thread::get_id(), "FinishedLoading() should be called by the SimulationServers thread");

		DoSimulationLoad();

		m_state.store(State::Loaded, std::memory_order_release);

		while (m_state.load(std::memory_order_acquire) == State::Loaded)
		{
			TCommandBuffer<SimulationServer> command_buffer;
			{
				std::lock_guard lock(m_commands_mutex);
				command_buffer = std::move(m_deferred_commands);
			}

			// Process the deferred commands sent by other threads
			command_buffer.ProcessCommands(*this);

			DoSimulationThreadProgress();

			if (m_deferred_signals.NumCommands() > 0)
			{
				// Flush signals to be executed on main thread
				CommandServer::get_singleton()->AddCommands(get_instance_id(), std::move(m_deferred_signals));
			}
		}

		DoSimulationUnload();

		m_state.store(State::Unloaded);
	}

	void SimulationServer::_bind_methods()
	{
		BIND_ENUM_CONSTANT(THREAD_MODE_SINGLE_THREADED);
		BIND_ENUM_CONSTANT(THREAD_MODE_MULTI_THREADED);

		BIND_METHOD(godot::D_METHOD("start_simulation", "thread_mode"), &SimulationServer::StartSimulation);
		BIND_METHOD(godot::D_METHOD("stop_simulation"), &SimulationServer::StopSimulation);
		BIND_METHOD(godot::D_METHOD("wait_until_stopped"), &SimulationServer::WaitUntilStopped);
		BIND_METHOD(godot::D_METHOD("is_threaded"), &SimulationServer::IsThreaded);
		BIND_METHOD(godot::D_METHOD("progress", "delta"), &SimulationServer::Progress);

		ADD_SIGNAL(godot::MethodInfo("load_state_changed", ENUM_PROPERTY("state", SimulationServer::LoadState)));
	}
}