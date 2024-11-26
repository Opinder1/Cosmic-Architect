#pragma once

#include "CommandBuffer.h"
#include "Debug.h"
#include "GodotMemory.h"
#include "Time.h"
#include "PerThread.h"

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/ref.hpp>

#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <TKRZW/tkrzw_thread_util.h>

#include <vector>
#include <deque>
#include <memory>

namespace voxel_game
{
	// Container for a command buffer to write commands for an object in script
	// 
	// The queues methods should only be called by the thread that creates the queue
	class CommandQueue : public godot::RefCounted
	{
		GDCLASS(CommandQueue, godot::RefCounted);

	public:
		// Make a new queue with an associated object which we write to on this thread only.
		// We can then call Flush() to run the commands on the main thread/render thread through the command server
		static godot::Ref<CommandQueue> MakeQueue(const godot::Variant& object);

		CommandQueue();
		~CommandQueue();

		// Get the object that this queue is queueing commands form
		uint64_t GetObject();

		// Get the thread that created this queue. Only this thread should use the queue
		uint64_t GetOwningThread();

		// Register a new command for the queue
		template<class... Args>
		void AddCommand(const godot::StringName& command, Args&&... p_args);

		// Flush the commands to the command server to be run on the specified thread
		void Flush();

	public:
		static void _bind_methods();

	private:
		void _add_command_vararg(const godot::Variant** p_args, GDExtensionInt p_argcount, GDExtensionCallError& error);

	private:
		std::thread::id m_owner_id;
		uint64_t m_owner_gdid = 0;
		uint64_t m_object_id = 0;
		CommandBuffer m_command_buffer;
	};

	// Server which command buffers can be flushed to and processed frame by frame on the main thread or render thread
	// 
	// This class is thread safe but will complain if the wrong thread calls certain methods
	class CommandQueueServer : public godot::Object
	{
		GDCLASS(CommandQueueServer, godot::Object);

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
		static CommandQueueServer* get_singleton();

		CommandQueueServer();
		~CommandQueueServer();

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
		static godot::OptObj<CommandQueueServer> k_singleton;

		alignas(k_cache_line) State m_state;
		alignas(k_cache_line) State m_rendering_state;
	};

	template<class... Args>
	void CommandQueue::AddCommand(const godot::StringName& command, Args&&... p_args)
	{
		DEBUG_ASSERT(m_thread_id == std::this_thread::get_id(), "Should be run by the owning thread");

		return CommandBuffer::AddCommand(m_command_buffer, command, std::forward<Args>(p_args)...);
	}
}