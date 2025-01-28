#pragma once

#include "GodotCommandBuffer.h"

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
	// Container for a command buffer to write commands for an object in script
	// 
	// The queues methods should only be called by the thread that creates the queue
	class CommandWriter : public godot::RefCounted
	{
		GDCLASS(CommandWriter, godot::RefCounted);

	public:
		// Make a new queue with an associated object which we write to on this thread only.
		// We can then call Flush() to run the commands on the main thread/render thread through the command server
		static godot::Ref<CommandWriter> MakeWriter(const godot::Variant& object);

		CommandWriter();
		~CommandWriter();

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
		uint64_t m_owner_id = 0;
		uint64_t m_object_id = 0;
		GodotCommandBuffer m_command_buffer;
	};

	template<class... Args>
	void CommandWriter::AddCommand(const godot::StringName& command, Args&&... p_args)
	{
		DEBUG_ASSERT(godot::OS::get_singleton()->get_thread_caller_id() == m_thread_id, "Should be run by the owning thread");

		return CommandBuffer::AddCommand(m_command_buffer, command, std::forward<Args>(p_args)...);
	}
}