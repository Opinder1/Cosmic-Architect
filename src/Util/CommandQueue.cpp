#include "CommandQueue.h"

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

namespace voxel_game
{
	constexpr const size_t k_max_commands_per_iter = 64;
	constexpr const Clock::duration k_max_time_per_flush = 100ms;
	constexpr const Clock::duration k_max_time_per_render_flush = 10ms;

	godot::Ref<CommandQueue> CommandQueue::MakeQueue(const godot::Variant& object)
	{
		if (object.get_type() != godot::Variant::OBJECT)
		{
			DEBUG_PRINT_WARN("Tried to create a command queue for a variant that is not an object");
			return nullptr;
		}

		godot::Ref<CommandQueue> command_queue;

		command_queue.instantiate();

		command_queue->m_thread_id = godot::OS::get_singleton()->get_thread_caller_id();
		command_queue->m_object_id = object;

		return command_queue;
	}

	CommandQueue::CommandQueue()
	{}

	CommandQueue::~CommandQueue()
	{
		CommandQueueServer::get_singleton()->AddCommands(m_object_id, std::move(m_command_buffer));
	}

	uint64_t CommandQueue::GetOwningThread()
	{
		return m_thread_id;
	}

	uint64_t CommandQueue::GetObject()
	{
		return m_object_id;
	}

	void CommandQueue::_add_command_vararg(const godot::Variant** p_args, GDExtensionInt p_argcount, GDExtensionCallError& error)
	{
		if (godot::OS::get_singleton()->get_thread_caller_id() != m_thread_id)
		{
			error.error = GDEXTENSION_CALL_ERROR_INVALID_METHOD;
			DEBUG_PRINT_ERROR("Should be run by the owning thread");
			return;
		}

		if (p_argcount <= 0)
		{
			error.error = GDEXTENSION_CALL_ERROR_TOO_FEW_ARGUMENTS;
			error.argument = 0;
			error.expected = 1;
			DEBUG_PRINT_ERROR("The command argument is missing in register_command");
			return;
		}

		if (p_argcount > 15)
		{
			error.error = GDEXTENSION_CALL_ERROR_TOO_MANY_ARGUMENTS;
			error.argument = 16;
			error.expected = 15;
			DEBUG_PRINT_ERROR("The command queue only supports a maximum of 15 arguments per command");
			return;
		}

		if (p_args == nullptr)
		{
			error.error = GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT;
			error.argument = 0;
			DEBUG_PRINT_ERROR("We were given null arg array");
			return;
		}

		return m_command_buffer.AddCommandVararg(*p_args[0], p_args + 1, p_argcount - 1);
	}

	void CommandQueue::Flush()
	{
		DEBUG_ASSERT(godot::ObjectID(m_object_id).is_valid(), "Command queue should have an assigned object");
		DEBUG_ASSERT(godot::OS::get_singleton()->get_thread_caller_id() == m_thread_id, "Should be run by the owning thread");

		CommandQueueServer::get_singleton()->AddCommands(m_object_id, std::move(m_command_buffer)); // m_command_buffers guaranteed to be empty()
	}

	void CommandQueue::_bind_methods()
	{
		godot::ClassDB::bind_vararg_method(godot::METHOD_FLAGS_DEFAULT, "add_command", &CommandQueue::_add_command_vararg, godot::MethodInfo("add_command", godot::PropertyInfo(godot::Variant::STRING, "command")));
		godot::ClassDB::bind_static_method(get_class_static(), godot::D_METHOD("make_queue", "object"), &CommandQueue::MakeQueue);
		godot::ClassDB::bind_method(godot::D_METHOD("get_owner_thread"), &CommandQueue::GetObject);
		godot::ClassDB::bind_method(godot::D_METHOD("get_object"), &CommandQueue::GetObject);
	}

	godot::OptObj<CommandQueueServer> CommandQueueServer::k_singleton;

	CommandQueueServer* CommandQueueServer::get_singleton()
	{
		return &k_singleton.value();
	}

	CommandQueueServer::CommandQueueServer()
	{}

	CommandQueueServer::~CommandQueueServer()
	{
		DEBUG_ASSERT(!HasCommandsLeft(), "Commands left over when exiting the application");
	}

	void CommandQueueServer::AddCommands(uint64_t object_id, CommandBuffer&& command_buffer)
	{
		DEBUG_ASSERT(godot::ObjectID(object_id).is_valid(), "Should be adding commands for an object");

		if (command_buffer.NumCommands() == 0)
		{
			return;
		}

		if (object_id == godot::RenderingServer::get_singleton()->get_instance_id())
		{
			std::lock_guard lock(m_rendering_state.buffers_mutex);

			Commands& commands = m_rendering_state.command_buffers.emplace_back();

			commands.command_buffer = std::move(command_buffer);
		}
		else
		{
			std::lock_guard lock(m_state.buffers_mutex);

			Commands& commands = m_state.command_buffers.emplace_back();

			commands.object_id = object_id;
			commands.command_buffer = std::move(command_buffer);
		}
	}

	void CommandQueueServer::Flush()
	{
		DEBUG_ASSERT(godot::OS::get_singleton()->get_thread_caller_id() == godot::OS::get_singleton()->get_main_thread_id(), "The processor should only be flushed on the main thread");

		FlushState(m_state, nullptr, k_max_time_per_flush);

		// The rendering flush should always be called while we are alive
		// When the game starts shutting down, the render server should no longer do any calls
		godot::RenderingServer::get_singleton()->call_on_render_thread(callable_mp_static(&CommandQueueServer::RenderingFlush));
	}

	bool CommandQueueServer::HasCommandsLeft()
	{
		{
			std::shared_lock lock(m_rendering_state.buffers_mutex);
			if (m_rendering_state.command_buffers.size() > 0)
			{
				return true;
			}
		}

		{
			std::shared_lock lock(m_state.buffers_mutex);
			if (m_state.command_buffers.size() > 0)
			{
				return true;
			}
		}

		return false;
	}

	void CommandQueueServer::RenderingFlush()
	{
		CommandQueueServer* cqserver = CommandQueueServer::get_singleton();
		godot::RenderingServer* rserver = godot::RenderingServer::get_singleton();

		DEBUG_ASSERT(rserver->is_on_render_thread(), "The rendering flush should only be done on the rendering thread");

		FlushState(cqserver->m_rendering_state, rserver, k_max_time_per_render_flush);
	}

	void CommandQueueServer::FlushState(State& state, godot::Object* object, Clock::duration max_flush_time)
	{
		if (state.flushing_in_progress)
		{
			return;
		}

		Clock::time_point end_time = Clock::now() + max_flush_time;

		state.flushing_in_progress = true;

		// Keep processing buffers until we run out of time
		while (Clock::now() < end_time)
		{
			if (!state.buffer_in_progress)
			{
				// Try to get a new buffer to process as we don't have one
				std::lock_guard lock(state.buffers_mutex);

				if (state.command_buffers.size() == 0)
				{
					break; // No new buffers
				}

				state.current_buffer = std::move(state.command_buffers.front()); // command_buffers guaranteed to be empty() after
				state.command_buffers.pop_front();
				state.buffer_in_progress = true;
			}

			size_t commands_processed;

			// Use the provided object if given else use the buffers object
			if (object != nullptr)
			{
				commands_processed = state.current_buffer.command_buffer.ProcessCommands(object, k_max_commands_per_iter);
			}
			else
			{
				commands_processed = state.current_buffer.command_buffer.ProcessCommands(state.current_buffer.object_id, k_max_commands_per_iter);
			}

			// We finished the current buffer
			if (state.current_buffer.command_buffer.NumCommands() == 0)
			{
				state.buffer_in_progress = false;
			}
			else if (commands_processed == 0) // We have commands but didn't process any so the buffer is broken
			{
				state.current_buffer.command_buffer.Clear();
				state.buffer_in_progress = false;
			}

			// If we exit the loop we will start on the current buffer in the next iteration
		}

		state.flushing_in_progress = false;
	}

	void CommandQueueServer::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("flush"), &CommandQueueServer::Flush);
		godot::ClassDB::bind_method(godot::D_METHOD("has_commands_left"), &CommandQueueServer::HasCommandsLeft);

		k_singleton.instantiate();
	}

	void CommandQueueServer::_cleanup_methods()
	{
		k_singleton.reset();
	}
}