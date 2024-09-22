#pragma once

#include "Simulation/CommandBuffer.h"

#include "Util/Debug.h"
#include "Util/PerThread.h"

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/weak_ref.hpp>
#include <godot_cpp/classes/dir_access.hpp>

#include <godot_cpp/core/gdvirtual.gen.inc>

#include <TKRZW/tkrzw_thread_util.h>

#include <optional>
#include <thread>

namespace voxel_game
{
	// Simulation of a section of the universe
	class Simulation : public godot::RefCounted
	{
		GDCLASS(Simulation, godot::RefCounted);

	public:
		enum ThreadMode
		{
			THREAD_MODE_SINGLE_THREADED,
			THREAD_MODE_MULTI_THREADED
		};

		enum LoadState
		{
			LOAD_STATE_LOADING,
			LOAD_STATE_LOADED,
			LOAD_STATE_UNLOADING,
			LOAD_STATE_UNLOADED
		};

	public:
		Simulation();
		~Simulation();

		void StartSimulation(ThreadMode thread_mode);
		void StopSimulation();
		void FinishedLoading();
		void FinishedUnloading();
		bool IsThreaded();
		bool Progress(real_t delta);

	protected:
		virtual bool OnSimulationLoading();
		virtual void OnSimulationLoaded();
		virtual void OnSimulationUnloading();
		virtual void OnSimulationUnloaded();
		virtual bool DoSimulationProgress(real_t delta);
		virtual void DoSimulationThreadProgress();

		template<class... Args>
		void QueueSignal(const godot::StringName& signal, Args&&... p_args);

		template<class... Args>
		bool DeferCommand(const godot::StringName& command, Args&&... p_args);

	private:
		void ThreadLoop();

		GDVIRTUAL0R(bool, _simulation_loading);
		GDVIRTUAL0(_simulation_loaded);
		GDVIRTUAL0(_simulation_unloading);
		GDVIRTUAL0(_simulation_unloaded);
		GDVIRTUAL1R(bool, _simulation_progress, real_t);
		GDVIRTUAL0(_simulation_thread_progress);

	public:
		static void _bind_methods();

	private:
		// Internal thread used by the simulation to run in parallel with the main thread and other threads
		std::thread m_thread;

		// The load state to control the initial loading and final unloading of the simulation
		std::atomic<LoadState> m_load_state = LOAD_STATE_UNLOADED;

		// Commands to be deferred and processed by the internal thread
		tkrzw::SpinMutex m_commands_mutex;
		alignas(k_cache_line) CommandBuffer m_deferred_commands;

		// Signals sent by the internal thread and deferred to be run by the main thread
		alignas(k_cache_line) CommandBuffer m_deferred_signals;
	};

	template<class... Args>
	void Simulation::QueueSignal(const godot::StringName& signal, Args&&... args)
	{
		DEBUG_ASSERT(!IsThreaded() || std::this_thread::get_id() == m_thread.get_id(), "When in threaded mode this should only be called by the worker");

		m_deferred_signals.AddCommand(*k_emit_signal, signal, std::forward<Args>(args)...);
	}

	template<class... Args>
	bool Simulation::DeferCommand(const godot::StringName& command, Args&&... args)
	{
		if (IsThreaded() && std::this_thread::get_id() != m_thread.get_id())
		{
			std::lock_guard lock(m_commands_mutex);
			m_deferred_commands.AddCommand(command, std::forward<Args>(args)...);
			return true;
		}
		else
		{
			return false;
		}
	}
}

VARIANT_ENUM_CAST(voxel_game::Simulation::ThreadMode);
VARIANT_ENUM_CAST(voxel_game::Simulation::LoadState);