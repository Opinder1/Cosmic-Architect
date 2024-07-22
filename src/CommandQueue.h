#pragma once

#include "Util/Debug.h"
#include "Util/GodotMemory.h"

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
	struct Command
	{
		godot::StringName command;
		size_t argcount = 0;
	};

	class CommandBuffer : Nocopy
	{
		using Storage = std::vector<uint8_t>;

	public:
		using iterator = Storage::iterator;

	public:
		CommandBuffer();
		~CommandBuffer();

		CommandBuffer& operator=(CommandBuffer&& other) noexcept;
		
		// Register a new command for the queue
		template<class... Args>
		void AddCommand(const godot::StringName& command, const Args&... p_args)
		{
			godot::Variant args[sizeof...(p_args) + 1] = { p_args..., godot::Variant() }; // +1 makes sure zero sized arrays are also supported.
			const godot::Variant* argptrs[sizeof...(p_args) + 1];
			for (uint32_t i = 0; i < sizeof...(p_args); i++) {
				argptrs[i] = &args[i];
			}
			return AddCommandInternal(command, sizeof...(p_args) == 0 ? nullptr : (const godot::Variant**)argptrs, sizeof...(p_args));
		}

		void AddCommandInternal(const godot::StringName& command, const godot::Variant** args, size_t argcount);

		// Process only up to a certain number of commands and return how many were processed (0 for max to process all)
		size_t ProcessCommands(uint64_t object_id, size_t max = 0);

		size_t NumCommands() const;

		void Clear();

	private:
		Storage m_data;
		size_t m_start = 0;
		size_t m_num_commands = 0;
	};

	class CommandQueue : public godot::RefCounted
	{
		GDCLASS(CommandQueue, godot::RefCounted);

	public:
		// Make a new queue with an associated object. We can then call Flush() to run the commands on the main thread/render thread
		static godot::Ref<CommandQueue> MakeQueue(const godot::Variant& object);

		CommandQueue();
		~CommandQueue();

		// Get the object that this queue is queueing commands form
		uint64_t GetObject();

		// Get the thread that created this queue. Only this thread should use the queue
		uint64_t GetOwningThread();

		// Register a new command for the queue
		template<class... Args>
		void AddCommand(const godot::StringName& command, const Args&... p_args)
		{
			DEBUG_ASSERT(godot::OS::get_singleton()->get_thread_caller_id() == m_thread_id, "Should be run by the owning thread");

			return CommandBuffer::AddCommand(m_command_buffer, command, p_args);
		}

		// Flush the commands to the main thread or the render thread if the object is the rendering server
		void Flush();

	public:
		static void _bind_methods();

	private:
		void _add_command_vararg(const godot::Variant** p_args, GDExtensionInt p_argcount, GDExtensionCallError& error);

	private:
		uint64_t m_thread_id = 0;
		godot::ObjectID m_object_id;
		CommandBuffer m_command_buffer;
	};

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
			bool processing_current = false;
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

	public:
		static void _bind_methods();
		static void _cleanup_methods();

	private:
		void RenderingFlush();

		static void FlushState(State& state);

	private:
		static godot::OptObj<CommandQueueServer> k_singleton;

		State m_state;
		State m_rendering_state;
	};
}