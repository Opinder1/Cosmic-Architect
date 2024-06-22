#include "CommandQueue.h"

#include "Util/VariableLengthArray.h"

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include <godot_cpp/core/class_db.hpp>

namespace voxel_game
{
	void CommandBuffer::AddCommandArgArray(CommandBuffer& command_buffer, const godot::StringName& command, const godot::Variant** args, size_t argcount)
	{
		uint32_t command_offset = command_buffer.size();
		uint32_t command_size = sizeof(Command) + (sizeof(godot::Variant) * argcount);

		command_buffer.resize(command_offset + command_size);

		uint8_t* buffer_pos = &command_buffer[command_offset];

		Command* command_ptr = memnew_placement(buffer_pos, Command);
		command_ptr->command = command;
		command_ptr->argcount = argcount;
		buffer_pos += sizeof(Command);

		for (size_t arg = 0; arg < argcount; arg++)
		{
			godot::Variant* arg_ptr = memnew_placement(buffer_pos, godot::Variant);
			*arg_ptr = *args[arg];
			buffer_pos += sizeof(godot::Variant);
		}
	}

	CommandBuffer::iterator ProcessCommand(godot::Variant& object, CommandBuffer::iterator buffer_pos, CommandBuffer::iterator buffer_end)
	{
		if (buffer_pos + sizeof(Command) <= buffer_end)
		{
			DEBUG_PRINT_ERROR("Command buffer doesn't fit the command and its arguments");
			return buffer_end;
		}

		const Command& command = reinterpret_cast<const Command&>(*buffer_pos);
		buffer_pos += sizeof(Command);

		VariableLengthArray<const godot::Variant*> argptrs = MakeVariableLengthArray(const godot::Variant*, command.argcount);

		for (size_t i = 0; i < argptrs.size(); i++)
		{
			if (buffer_pos + sizeof(godot::Variant) <= buffer_end)
			{
				DEBUG_PRINT_ERROR("Command buffer doesn't fit the command and its arguments");
				return buffer_end;
			}

			argptrs[i] = &reinterpret_cast<const godot::Variant&>(*buffer_pos);
			buffer_pos += sizeof(godot::Variant);
		}

		godot::Variant ret;
		GDExtensionCallError error;
		object.callp(command.command, argptrs.data(), argptrs.size(), ret, error);

		if (error.error != GDExtensionCallErrorType::GDEXTENSION_CALL_OK)
		{
			const char* error_type_str;

			switch (error.error)
			{
			case GDEXTENSION_CALL_OK: error_type_str = "GDEXTENSION_CALL_OK"; break;
			case GDEXTENSION_CALL_ERROR_INVALID_METHOD: error_type_str = "GDEXTENSION_CALL_ERROR_INVALID_METHOD"; break;
			case GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT: error_type_str = "GDEXTENSION_CALL_ERROR_INVALID_ARGUMENT"; break;
			case GDEXTENSION_CALL_ERROR_TOO_MANY_ARGUMENTS: error_type_str = "GDEXTENSION_CALL_ERROR_TOO_MANY_ARGUMENTS"; break;
			case GDEXTENSION_CALL_ERROR_TOO_FEW_ARGUMENTS: error_type_str = "GDEXTENSION_CALL_ERROR_TOO_FEW_ARGUMENTS"; break;
			case GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL: error_type_str = "GDEXTENSION_CALL_ERROR_INSTANCE_IS_NULL"; break;
			case GDEXTENSION_CALL_ERROR_METHOD_NOT_CONST: error_type_str = "GDEXTENSION_CALL_ERROR_METHOD_NOT_CONST"; break;
			default: error_type_str = "GDEXTENSION_UNKNOWN_ERROR"; break;
			}

			godot::UtilityFunctions::print(godot::vformat("%s: Error at argument %d. Expected %d arguments", error_type_str, error.argument, error.expected));
		}

		// We can call destructors on const objects for some reason but the memory is indeed owned by us
		for (size_t i = 0; i < argptrs.size(); i++)
		{
			argptrs[i]->~Variant();
		}

		command.~Command();

		return buffer_pos;
	}

	void CommandBuffer::ProcessCommands(uint64_t object_id, CommandBuffer&& command_buffer)
	{
		godot::Variant object = godot::ObjectDB::get_instance(object_id);

		if (!object)
		{
			DEBUG_PRINT_ERROR("The object that the command queue was queueing for was deleted");
			return;
		}

		auto buffer_pos = command_buffer.begin();
		while (buffer_pos != command_buffer.end())
		{
			buffer_pos = ProcessCommand(object, buffer_pos, command_buffer.end());
		}

		command_buffer.clear();
	}

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
	{
		m_command_buffer.reserve(64);
	}

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
		DEBUG_ASSERT(godot::OS::get_singleton()->get_thread_caller_id() == m_thread_id, "Should be run by the owning thread");

		if (p_argcount == 0)
		{
			error.error = GDEXTENSION_CALL_ERROR_TOO_FEW_ARGUMENTS;
			error.argument = 0;
			error.expected = 1;
			DEBUG_PRINT_WARN("The command argument is missing in register_command");
			return;
		}

		return CommandBuffer::AddCommandArgArray(m_command_buffer, *p_args[0], p_args + 1, p_argcount - 1);
	}

	void CommandQueue::Flush()
	{
		DEBUG_ASSERT(m_object_id != 0, "Command queue should have an assigned object");
		DEBUG_ASSERT(godot::OS::get_singleton()->get_thread_caller_id() == m_thread_id, "Should be run by the owning thread");

		CommandQueueServer::get_singleton()->AddCommands(m_object_id, std::move(m_command_buffer)); // m_command_buffers guaranteed to be empty()
		m_command_buffer.reserve(64);
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
		DEBUG_ASSERT(k_singleton, "The singleton doesn't exist");
		return k_singleton.ptr();
	}

	CommandQueueServer::CommandQueueServer()
	{
		m_command_buffers.reserve(16);
		m_rendering_command_buffers.reserve(16);
	}

	CommandQueueServer::~CommandQueueServer()
	{
		DEBUG_ASSERT(m_command_buffers.size() == 0, "Commands left over when exiting the application");
		DEBUG_ASSERT(m_rendering_command_buffers.size() == 0, "Render commands left over when exiting the application");

		Flush();
	}

	void CommandQueueServer::AddCommands(uint64_t object_id, CommandBuffer&& command_buffer)
	{
		if (command_buffer.empty())
		{
			return;
		}

		if (object_id == godot::RenderingServer::get_singleton()->get_instance_id())
		{
			std::lock_guard lock(m_mutex);

			Commands& commands = m_command_buffers.emplace_back();

			commands.object_id = object_id;
			commands.command_buffer = std::move(command_buffer);
		}
		else
		{
			std::lock_guard lock(m_rendering_mutex);

			Commands& commands = m_rendering_command_buffers.emplace_back();

			commands.object_id = object_id;
			commands.command_buffer = std::move(command_buffer);
		}
	}

	bool CommandQueueServer::HasCommands()
	{
		std::shared_lock lock(m_mutex);
		return m_command_buffers.size() > 0;
	}

	bool CommandQueueServer::HasRenderingCommands()
	{
		std::shared_lock lock(m_rendering_mutex);
		return m_rendering_command_buffers.size() > 0;
	}

	void CommandQueueServer::Flush()
	{
		DEBUG_ASSERT(godot::OS::get_singleton()->get_thread_caller_id() == godot::OS::get_singleton()->get_main_thread_id(), "The processor should only be flushed on the main thread");

		if (HasRenderingCommands())
		{
			godot::RenderingServer::get_singleton()->call_on_render_thread(godot::create_custom_callable_function_pointer(this, &CommandQueueServer::FlushRenderingCommands));
		}

		if (HasCommands())
		{
			FlushCommands();
		}
	}

	void CommandQueueServer::FlushCommands()
	{
		std::vector<Commands> command_buffers;

		{
			std::lock_guard lock(m_mutex);

			command_buffers = std::move(m_command_buffers); // m_command_buffers guaranteed to be empty()
			m_command_buffers.reserve(16);
		}

		for (Commands& commands : command_buffers)
		{
			CommandBuffer::ProcessCommands(commands.object_id, std::move(commands.command_buffer));
		}
	}

	void CommandQueueServer::FlushRenderingCommands()
	{
		std::vector<Commands> command_buffers;

		{
			std::lock_guard lock(m_rendering_mutex);

			command_buffers = std::move(m_rendering_command_buffers); // m_rendering_command_buffers guaranteed to be empty()
			m_rendering_command_buffers.reserve(16);
		}

		for (Commands& commands : command_buffers)
		{
			CommandBuffer::ProcessCommands(commands.object_id, std::move(commands.command_buffer));
		}
	}

	void CommandQueueServer::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("flush"), &CommandQueueServer::Flush);

		k_singleton.instantiate();
	}

	void CommandQueueServer::_cleanup_methods()
	{
		k_singleton.reset();
	}
}