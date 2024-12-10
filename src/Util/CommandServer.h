#pragma once

#include "CommandBuffer.h"
#include "Debug.h"
#include "GodotMemory.h"
#include "Time.h"
#include "PerThread.h"

#include <godot_cpp/classes/object.hpp>

#include <TKRZW/tkrzw_thread_util.h>

#include <vector>
#include <deque>
#include <memory>

namespace voxel_game
{
	// Server which command buffers can be flushed to and processed frame by frame on the main thread or render thread
	// 
	// This class is thread safe but will complain if the wrong thread calls certain methods
	class CommandServer : public godot::Object
	{
		GDCLASS(CommandServer, godot::Object);

		struct Commands
		{
			godot::ObjectID object_id;
			CommandBuffer command_buffer;
		};

		struct State
		{
			tkrzw::SpinSharedMutex buffers_mutex; // Protect command buffers
			std::deque<Commands> command_buffers;

			// These should always be accessed by same thread
			bool flushing_in_progress = false;
			bool buffer_in_progress = false;
			Commands current_buffer;
		};

	public:
		static CommandServer* get_singleton();

		CommandServer();
		~CommandServer();

		// Add commands and an object to run them on to be executed when flushing
		void AddCommands(uint64_t object_id, CommandBuffer&& command_buffer);

		// Run all commands on the main thread and all rendering server commands on the render thread
		void Flush();

		// Check if there are any commands left to be processed
		bool HasCommandsLeft();

	public:
		static void _bind_methods();
		static void _cleanup_methods();

	private:
		static void RenderingFlush();

		static void FlushState(State& state, godot::Object* object, Clock::duration max_flush_time);

	private:
		static godot::OptObj<CommandServer> k_singleton;

		alignas(k_cache_line) State m_state;
		alignas(k_cache_line) State m_rendering_state;
	};
}