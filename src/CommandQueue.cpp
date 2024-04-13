#include "CommandQueue.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include <godot_cpp/core/class_db.hpp>

namespace voxel_game
{
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
		command_queue->m_rendering_queue = command_queue->m_object_id = godot::RenderingServer::get_singleton()->get_instance_id();
		command_queue->m_command_buffer.reserve(4096);

		return command_queue;
	}

	CommandQueue::CommandQueue()
	{
		m_command_buffer.reserve(64);
	}

	CommandQueue::~CommandQueue()
	{
		if (m_rendering_queue)
		{
			CommandQueueServer::get_singleton()->AddRenderingCommands(m_object_id, std::move(m_command_buffer));
		}
		else
		{
			CommandQueueServer::get_singleton()->AddCommands(m_object_id, std::move(m_command_buffer));
		}
	}

	uint64_t CommandQueue::GetOwningThread()
	{
		return m_thread_id;
	}

	uint64_t CommandQueue::GetObject()
	{
		return m_object_id;
	}

	bool CommandQueue::IsRenderingQueue()
	{
		return m_rendering_queue;
	}

	void CommandQueue::_register_command_vararg(const godot::Variant** p_args, GDExtensionInt p_argcount, GDExtensionCallError& error)
	{
		if (p_argcount == 0)
		{
			error.error = GDEXTENSION_CALL_ERROR_TOO_FEW_ARGUMENTS;
			error.argument = 0;
			error.expected = 1;
			DEBUG_PRINT_WARN("The command argument is missing in register_command");
			return;
		}

		return _register_command(*p_args[0], p_args + 1, p_argcount - 1);
	}

	void CommandQueue::_register_command(const godot::StringName& command, const godot::Variant** args, size_t argcount)
	{
		DEBUG_ASSERT(godot::OS::get_singleton()->get_thread_caller_id() == m_thread_id, "Should be run by the owning thread");
		DEBUG_ASSERT(argcount <= Command::k_max_args, godot::vformat("The max args that is supported for commands is %d", Command::k_max_args));

		uint32_t command_offset = m_command_buffer.size();
		uint32_t command_size = sizeof(godot::StringName) + (sizeof(godot::Variant) * argcount);

		m_command_buffer.resize(command_offset + command_size);

		uint8_t* buffer_pos = &m_command_buffer[command_offset];

		Command* command_ptr = memnew_placement(buffer_pos, Command);
		command_ptr->command = command;
		command_ptr->argcount = argcount;
		buffer_pos += sizeof(godot::StringName);

		for (size_t arg = 0; arg < argcount; arg++)
		{
			godot::Variant* arg_ptr = memnew_placement(buffer_pos, godot::Variant);
			buffer_pos += sizeof(godot::Variant);
			*arg_ptr = *args[arg];
		}
	}

	void CommandQueue::PopCommandBuffer(CommandBuffer& command_buffer_out)
	{
		m_command_buffer.swap(command_buffer_out);

		m_command_buffer.clear();
		m_command_buffer.reserve(64);
	}

	void CommandQueue::Flush()
	{
		DEBUG_ASSERT(godot::OS::get_singleton()->get_thread_caller_id() == m_thread_id, "Should be run by the owning thread");

		std::vector<uint8_t> command_buffer;

		PopCommandBuffer(command_buffer);

		if (m_rendering_queue)
		{
			CommandQueueServer::get_singleton()->AddRenderingCommands(m_object_id, std::move(command_buffer));
		}
		else
		{
			CommandQueueServer::get_singleton()->AddCommands(m_object_id, std::move(command_buffer));
		}
	}

	void CommandQueue::ProcessCommands(uint64_t object_id, const CommandBuffer& command_buffer)
	{
		godot::Object* object_ptr = godot::ObjectDB::get_instance(object_id);

		if (object_ptr == nullptr)
		{
			DEBUG_PRINT_ERROR("The object that the command queue was queueing for was deleted");
			return;
		}

		godot::Variant object = object_ptr;

		auto buffer_pos = command_buffer.begin();
		while (buffer_pos != command_buffer.end())
		{
			DEBUG_ASSERT(buffer_pos + sizeof(Command) <= command_buffer.end(), "Command buffer doesn't fit the command and its arguments");

			const Command& command_ptr = reinterpret_cast<const Command&>(*buffer_pos);
			buffer_pos += sizeof(Command);

			const godot::Variant* argptrs[Command::k_max_args];

			for (size_t i = 0; i < command_ptr.argcount; i++)
			{
				DEBUG_ASSERT(buffer_pos + sizeof(godot::Variant) <= command_buffer.end(), "Command buffer doesn't fit the command and its arguments");

				const godot::Variant& arg = reinterpret_cast<const godot::Variant&>(buffer_pos);
				buffer_pos += sizeof(godot::Variant);

				argptrs[i] = &arg;
			}

			godot::Variant ret;
			GDExtensionCallError error;
			object.callp(command_ptr.command, argptrs, command_ptr.argcount, ret, error);

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

			command_ptr.~Command();

			for (size_t i = 0; i < command_ptr.argcount; i++)
			{
				argptrs[i]->~Variant();
			}
		}
	}

	void CommandQueue::_bind_methods()
	{
		godot::ClassDB::bind_vararg_method(godot::METHOD_FLAGS_DEFAULT, "register_command", &CommandQueue::_register_command_vararg, godot::MethodInfo("register_command", godot::PropertyInfo(godot::Variant::STRING, "command")));
		godot::ClassDB::bind_static_method(get_class_static(), godot::D_METHOD("make_queue", "object"), &CommandQueue::MakeQueue);
		godot::ClassDB::bind_method(godot::D_METHOD("get_owner_thread"), &CommandQueue::GetObject);
		godot::ClassDB::bind_method(godot::D_METHOD("get_object"), &CommandQueue::GetObject);
		godot::ClassDB::bind_method(godot::D_METHOD("is_rendering_queue"), &CommandQueue::IsRenderingQueue);
	}

	std::unique_ptr<CommandQueueServer> CommandQueueServer::k_singleton;

	CommandQueueServer* CommandQueueServer::get_singleton()
	{
		DEBUG_ASSERT(k_singleton != nullptr, "The singleton doesn't exist");
		return k_singleton.get();
	}

	CommandQueueServer::CommandQueueServer()
	{}

	CommandQueueServer::~CommandQueueServer()
	{
		DEBUG_ASSERT(m_command_buffers.size() == 0, "Commands left over when exiting the application");
		DEBUG_ASSERT(m_rendering_command_buffers.size() == 0, "Render commands left over when exiting the application");

		Flush();
	}

	void CommandQueueServer::AddCommands(uint64_t object_id, CommandBuffer&& command_buffer)
	{
		std::lock_guard lock(m_mutex);

		Commands& commands = m_command_buffers.emplace_back();

		commands.object_id = object_id;
		commands.command_buffer.swap(command_buffer);
	}

	void CommandQueueServer::AddRenderingCommands(uint64_t object_id, CommandBuffer&& command_buffer)
	{
		std::lock_guard lock(m_rendering_mutex);

		Commands& commands = m_rendering_command_buffers.emplace_back();

		commands.object_id = object_id;
		commands.command_buffer.swap(command_buffer);
	}

	bool CommandQueueServer::HasCommands()
	{
		std::lock_guard lock(m_mutex);
		return m_command_buffers.size() > 0;
	}

	bool CommandQueueServer::HasRenderingCommands()
	{
		std::lock_guard lock(m_rendering_mutex);
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
			m_command_buffers.swap(command_buffers);
		}

		for (Commands& commands : command_buffers)
		{
			CommandQueue::ProcessCommands(commands.object_id, commands.command_buffer);
		}
	}

	void CommandQueueServer::FlushRenderingCommands()
	{
		std::vector<Commands> command_buffers;

		{
			std::lock_guard lock(m_rendering_mutex);
			m_rendering_command_buffers.swap(command_buffers);
		}

		for (Commands& commands : command_buffers)
		{
			CommandQueue::ProcessCommands(commands.object_id, commands.command_buffer);
		}
	}

	void CommandQueueServer::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("flush"), &CommandQueueServer::Flush);

		k_singleton = std::make_unique<CommandQueueServer>();
	}

	void CommandQueueServer::_cleanup_methods()
	{
		k_singleton.reset();
	}
}