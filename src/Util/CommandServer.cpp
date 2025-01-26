#include "CommandServer.h"

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include <easy/profiler.h>

namespace voxel_game
{
	constexpr const size_t k_max_commands_per_iter = 64;
	constexpr const Clock::duration k_max_time_per_flush = 100ms;
	constexpr const Clock::duration k_max_time_per_render_flush = 10ms;

	godot::OptObj<CommandServer> CommandServer::k_singleton;

	CommandServer* CommandServer::get_singleton()
	{
		return &k_singleton.value();
	}

	CommandServer::CommandServer()
	{}

	CommandServer::~CommandServer()
	{
		DEBUG_ASSERT(!HasCommandsLeft(), "Commands left over when exiting the application");
	}

	void CommandServer::AddCommandBuffer(uint64_t object_id, std::unique_ptr<CommandBufferEntryBase>&& command_buffer)
	{
		DEBUG_ASSERT(godot::ObjectID(object_id).is_valid(), "Should be adding commands for an object");

		if (command_buffer->NumCommands() == 0)
		{
			return;
		}

		State& state = godot::RenderingServer::get_singleton()->get_instance_id() == object_id ? m_rendering_state : m_state;

		std::lock_guard lock(state.buffers_mutex);

		Entry& commands = state.command_buffers.emplace_back();

		state.num_commands += command_buffer->NumCommands();
		state.lifetime_commands += command_buffer->NumCommands();

		commands.object_id = object_id;
		commands.buffer = std::move(command_buffer);
	}

	void CommandServer::Flush()
	{
		EASY_BLOCK("MainFlush");

		DEBUG_ASSERT(godot::OS::get_singleton()->get_thread_caller_id() == godot::OS::get_singleton()->get_main_thread_id(), "The processor should only be flushed on the main thread");

		FlushState(m_state, nullptr, k_max_time_per_flush);

		// The rendering flush should always be called while we are alive
		// When the game starts shutting down, the render server should no longer do any calls
		godot::RenderingServer::get_singleton()->call_on_render_thread(callable_mp_static(&CommandServer::RenderingFlush));
	}

	bool CommandServer::HasCommandsLeft()
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

	void CommandServer::RenderingFlush()
	{
		EASY_BLOCK("RenderingFlush");

		CommandServer* cqserver = CommandServer::get_singleton();
		godot::RenderingServer* rserver = godot::RenderingServer::get_singleton();

		DEBUG_ASSERT(rserver->is_on_render_thread(), "The rendering flush should only be done on the rendering thread");

		FlushState(cqserver->m_rendering_state, rserver, k_max_time_per_render_flush);
	}

	void CommandServer::FlushState(State& state, godot::Object* object_override, Clock::duration max_flush_time)
	{
		if (state.flushing_in_progress)
		{
			return;
		}

		state.flushing_in_progress = true;

		Clock::time_point end_time = Clock::now() + max_flush_time;

		size_t processed_commands = 0;

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

			godot::Object* object;

			// Use the provided object if given else use the buffers object
			if (object_override != nullptr)
			{
				object = object_override;
			}
			else
			{
				DEBUG_ASSERT(godot::ObjectID(state.current_buffer.object_id).is_valid(), "The command should be run on a valid object");

				object = godot::ObjectDB::get_instance(state.current_buffer.object_id);
			}

			if (object != nullptr)
			{
				DEBUG_ASSERT(state.current_buffer.buffer->NumCommands() > 0, "We should have commands in the buffer else why did we add it");

				processed_commands += state.current_buffer.buffer->ProcessCommands(object, k_max_commands_per_iter);
			}
			else
			{
				DEBUG_PRINT_ERROR("The object that the command queue was queueing for was deleted");
				state.current_buffer.buffer->Clear();
				state.current_buffer.buffer->ShrinkToFit();
				state.buffer_in_progress = false;
			}

			// We finished the current buffer
			if (state.current_buffer.buffer->NumCommands() == 0)
			{
				state.buffer_in_progress = false;
			}

			// If we exit the loop we will start on the current buffer in the next iteration
		}

		state.flushing_in_progress = false;

		{
			std::lock_guard lock(state.buffers_mutex);

			DEBUG_ASSERT(state.num_commands - processed_commands >= 0, "We processed more commands than we have?");

			state.num_commands -= processed_commands;
		}
	}

	void CommandServer::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("flush"), &CommandServer::Flush);
		godot::ClassDB::bind_method(godot::D_METHOD("has_commands_left"), &CommandServer::HasCommandsLeft);

		k_singleton.instantiate();
	}

	void CommandServer::_cleanup_methods()
	{
		k_singleton.reset();
	}
}