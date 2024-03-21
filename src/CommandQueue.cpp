#include "CommandQueue.h"

#include "Util/Debug.h"

#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/os.hpp>

namespace voxel_game
{
	CommandQueue::CommandQueue()
	{
		m_thread_id = std::this_thread::get_id();

		m_command_offsets.reserve(256);
		m_command_buffer.reserve(4096);
	}

	CommandQueue::~CommandQueue()
	{
		CommandQueueProcessor::get_singleton()->AddCommands(*this);
	}

	godot::Ref<CommandQueue> CommandQueue::MakeQueue(const godot::Variant& object)
	{
		if (object.get_type() != godot::Variant::OBJECT)
		{
			return nullptr;
		}

		godot::Ref<CommandQueue> command_queue;

		command_queue.instantiate();

		command_queue->m_object_id = object.operator godot::ObjectID();

		return command_queue;
	}

	void CommandQueue::Flush()
	{
		DEBUG_ASSERT(std::this_thread::get_id() == m_thread_id, "Should be run by the owning thread");

		CommandQueueProcessor::get_singleton()->AddCommands(*this);

		m_command_offsets.reserve(256);
		m_command_buffer.reserve(4096);
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
		DEBUG_ASSERT(std::this_thread::get_id() == m_thread_id, "Should be run by the owning thread");
		DEBUG_ASSERT(argcount <= Command::k_max_args, godot::vformat("The max args that is supported for commands is %d", Command::k_max_args));

		m_command_offsets.push_back(m_command_buffer.size());

		uint32_t command_offset = m_command_offsets.back();
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

	void CommandQueue::_bind_methods()
	{
		godot::ClassDB::bind_vararg_method(godot::METHOD_FLAGS_DEFAULT, "register_command", &CommandQueue::_register_command_vararg, godot::MethodInfo("register_command", godot::PropertyInfo(godot::Variant::STRING, "command")));
		godot::ClassDB::bind_static_method(get_class_static(), godot::D_METHOD("make_queue", "object"), &CommandQueue::MakeQueue);

		ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "object", godot::PROPERTY_HINT_OBJECT_ID), "set_object", "get_object");
	}

	std::unique_ptr<CommandQueueProcessor> CommandQueueProcessor::k_singleton;

	CommandQueueProcessor* CommandQueueProcessor::get_singleton()
	{
		DEBUG_ASSERT(k_singleton != nullptr, "The singleton doesn't exist");
		return k_singleton.get();
	}

	CommandQueueProcessor::CommandQueueProcessor()
	{}

	CommandQueueProcessor::~CommandQueueProcessor()
	{}

	void CommandQueueProcessor::AddCommands(CommandQueue& command_queue)
	{
		std::lock_guard lock(m_mutex);

		Commands& commands = m_command_buffers.emplace_back();

		commands.object_id = command_queue.m_object_id;
		commands.command_buffer.swap(command_queue.m_command_buffer);
		commands.command_offsets.swap(command_queue.m_command_offsets);
	}

	void CommandQueueProcessor::ProcessCommands(const Commands& commands)
	{
		godot::Variant object = godot::ObjectDB::get_instance(commands.object_id);

		for (uint32_t command_offset : commands.command_offsets)
		{
			DEBUG_ASSERT(commands.command_buffer.size() > command_offset, "Command offset submitted is outside of the range of the buffer");

			const uint8_t* buffer_pos = &commands.command_buffer[command_offset];

			const Command* command_ptr = reinterpret_cast<const Command*>(buffer_pos);

			DEBUG_ASSERT(commands.command_buffer.size() > command_offset + sizeof(Command) + (sizeof(godot::Variant) * command_ptr->argcount), "Command buffer doesn't fit the command and its arguments");

			const godot::Variant* args = reinterpret_cast<const godot::Variant*>(buffer_pos + sizeof(Command));

			const godot::Variant* argptrs[Command::k_max_args];
			if (command_ptr->argcount > 0)
			{
				for (size_t arg = 0; arg < command_ptr->argcount; arg++)
				{
					argptrs[arg] = &args[arg];
				}
			}

			godot::Variant ret;
			GDExtensionCallError error;
			object.callp(command_ptr->command, argptrs, command_ptr->argcount, ret, error);

			command_ptr->~Command();

			for (size_t arg = 0; arg < command_ptr->argcount; arg++)
			{
				args[arg].~Variant();
			}
		}
	}

	void CommandQueueProcessor::Flush()
	{
		DEBUG_ASSERT(godot::OS::get_singleton()->get_thread_caller_id() == godot::OS::get_singleton()->get_main_thread_id(), "The processor should only be flushed on the main thread");

		std::lock_guard lock(m_mutex);

		for (Commands& commands : m_command_buffers)
		{
			ProcessCommands(commands);
		}

		m_command_buffers.clear();
	}

	void CommandQueueProcessor::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("flush"), &CommandQueueProcessor::Flush);

		k_singleton = std::make_unique<CommandQueueProcessor>();
	}

	void CommandQueueProcessor::_cleanup_methods()
	{
		k_singleton.reset();
	}
}