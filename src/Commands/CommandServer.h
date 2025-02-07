#pragma once

#include "CommandBufferEntry.h"

#include "Util/Util.h"
#include "Util/Debug.h"
#include "Util/GodotMemory.h"
#include "Util/PerThread.h"

#include <godot_cpp/classes/object.hpp>

#include <TKRZW/tkrzw_thread_util.h>

#include <vector>
#include <deque>
#include <memory>
#include <variant>

namespace voxel_game
{
	// Server which command buffers can be flushed to and processed frame by frame on the main thread or render thread
	// 
	// This class is thread safe but will complain if the wrong thread calls certain methods
	class CommandServer : public godot::Object
	{
		GDCLASS(CommandServer, godot::Object);

		struct Entry
		{
			godot::ObjectID object_id;
			Clock::time_point added_time;
			std::unique_ptr<CommandBufferEntryBase> buffer;
		};

		struct State
		{
			tkrzw::SpinSharedMutex buffers_mutex; // Protect command buffers
			std::deque<Entry> command_buffers;
			size_t num_commands = 0;
			size_t lifetime_commands = 0;

			// These should always be accessed by same thread
			bool flushing_in_progress = false;
			bool buffer_in_progress = false;
			Entry current_buffer;
		};

	public:
		static CommandServer* get_singleton();

		CommandServer();
		~CommandServer();

		template<class T>
		void AddCommands(uint64_t object_id, T&& command_buffer)
		{
			AddCommandBuffer(object_id, std::make_unique<CommandBufferEntry<T>>(std::move(command_buffer)));
		}

		// Run all commands on the main thread and all rendering server commands on the render thread
		void Flush();

		// Check if there are any commands left to be processed
		bool HasCommandsLeft();

	public:
		static void _bind_methods();
		static void _cleanup_methods();

	private:
		// Add commands and an object to run them on to be executed when flushing
		void AddCommandBuffer(uint64_t object_id, std::unique_ptr<CommandBufferEntryBase>&& command_buffer);

		static void RenderingFlush();

		static void FlushState(State& state, godot::Object* object_override, Clock::duration max_flush_time);

	private:
		static godot::OptObj<CommandServer> k_singleton;

		alignas(k_cache_line) State m_state;
		alignas(k_cache_line) State m_rendering_state;
	};
}