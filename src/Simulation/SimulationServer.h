#pragma once

#include "Commands/TypedCommandBuffer.h"

#include "Util/Debug.h"
#include "Util/PerThread.h"

#include <godot_cpp/classes/ref_counted.hpp>

#include <godot_cpp/core/gdvirtual.gen.inc>

#include <TKRZW/tkrzw_thread_util.h>

#include <optional>
#include <thread>

namespace voxel_game
{
	// A simulation object that runs a simulation for which commands are given as an input and
	// signals are given as an output. When running in threaded mode, the commands and signals
	// are efficiently buffered and sent between threads with minimal blocking
	// 
	// The simulations methods should only be called by the thread that creates the simulation
	class SimulationServer : public godot::RefCounted
	{
		GDCLASS(SimulationServer, godot::RefCounted);

	public:
		enum State
		{
			Unloaded,
			Loading,
			Loaded,
			Unloading
		};

		enum ThreadMode
		{
			THREAD_MODE_SINGLE_THREADED,
			THREAD_MODE_MULTI_THREADED
		};

	public:
		SimulationServer();
		~SimulationServer();

		// Start the simulation. Make sure to call FinishedLoading() on the thread when done loading
		void StartSimulation(ThreadMode thread_mode);

		// Stop the simulation. Make sure to call FinishedUnloading() when done unloading
		void StopSimulation();

		// When destroying the simulation one should make sure the thread is stopped before destroying
		void WaitUntilStopped();

		// Check if we are in threaded mode
		bool IsThreaded();

		// Progress the simulation from the owning thread. When in thread mode, this is just to communicate with the thread
		bool Progress(real_t delta);

	protected:
		// If the following methods are overridden then the script overrides will no longer work.

		// Control if the simulation will start or not
		virtual bool CanSimulationStart();

		// Called when initializing the simulation. In threading mode will be called on the worker thread.
		virtual void DoSimulationLoad();

		// Called when initializing the simulation. In threading mode will be called on the worker thread.
		virtual void DoSimulationUnload();

		// Called on the thread that calls Progress()
		virtual bool DoSimulationProgress(real_t delta);

		// Called on the simulations worker thread if threading is enabled
		virtual void DoSimulationThreadProgress();

		// Queue a signal to be broadcast to the main thread. Call this on the simulation thread
		template<class... Args>
		void QueueSignal(const godot::StringName& signal, Args&&... p_args);

		// Send a command to the simulation. Call on any thread that is not the simulation thread
		template<auto Method, class... Args>
		bool DeferCommand(Args&&... p_args);

	private:
		void ThreadLoop();

		GDVIRTUAL0R(bool, _can_simulation_start);
		GDVIRTUAL0(_do_simulation_load);
		GDVIRTUAL0(_do_simulation_unload);
		GDVIRTUAL1R(bool, _simulation_progress, real_t);
		GDVIRTUAL0(_simulation_thread_progress);

	public:
		static void _bind_methods();

	private:
		// Internal thread used by the simulation to run in parallel with the main thread and other threads
		std::thread m_thread;

		// The load state to control the initial loading and final unloading of the simulation
		std::atomic<State> m_state = State::Unloaded;

		// Commands to be deferred and processed by the internal thread
		tkrzw::SpinMutex m_commands_mutex;
		alignas(k_cache_line) TCommandBuffer<SimulationServer> m_deferred_commands;

		// Signals sent by the internal thread and deferred to be run by the main thread
		alignas(k_cache_line) TCommandBuffer<SimulationServer> m_deferred_signals;
	};

	template<class... Args>
	void SimulationServer::QueueSignal(const godot::StringName& signal, Args&&... args)
	{
		DEBUG_ASSERT(!IsThreaded() || std::this_thread::get_id() == m_thread.get_id(), "When in threaded mode this should only be called by the worker");

		if (IsThreaded())
		{
			m_deferred_signals.AddCommand<&SimulationServer::emit_signal<Args...>>(signal, std::forward<Args>(args)...);
		}
		else
		{
			emit_signal(signal, std::forward<Args>(args)...);
		}
	}

	template<auto Method, class... Args>
	bool SimulationServer::DeferCommand(Args&&... args)
	{
		if (IsThreaded() && std::this_thread::get_id() != m_thread.get_id())
		{
			std::lock_guard lock(m_commands_mutex);
			m_deferred_commands.AddCommand<Method>(std::forward<Args>(args)...);
			return true;
		}
		else
		{
			return false;
		}
	}
}

VARIANT_ENUM_CAST(voxel_game::SimulationServer::ThreadMode);