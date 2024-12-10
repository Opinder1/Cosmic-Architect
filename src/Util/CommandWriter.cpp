#include "CommandWriter.h"
#include "CommandServer.h"

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

namespace voxel_game
{
	godot::Ref<CommandWriter> CommandWriter::MakeWriter(const godot::Variant& object)
	{
		if (object.get_type() != godot::Variant::OBJECT)
		{
			DEBUG_PRINT_WARN("Tried to create a command queue for a variant that is not an object");
			return nullptr;
		}

		godot::Ref<CommandWriter> command_queue;

		command_queue.instantiate();

		command_queue->m_owner_id = godot::OS::get_singleton()->get_thread_caller_id();
		command_queue->m_object_id = object;

		return command_queue;
	}

	CommandWriter::CommandWriter()
	{}

	CommandWriter::~CommandWriter()
	{
		CommandServer::get_singleton()->AddCommands(m_object_id, std::move(m_command_buffer));
	}

	uint64_t CommandWriter::GetOwningThread()
	{
		return m_owner_id;
	}

	uint64_t CommandWriter::GetObject()
	{
		return m_object_id;
	}

	void CommandWriter::_add_command_vararg(const godot::Variant** p_args, GDExtensionInt p_argcount, GDExtensionCallError& error)
	{
		if (m_owner_id != godot::OS::get_singleton()->get_thread_caller_id())
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

	void CommandWriter::Flush()
	{
		DEBUG_ASSERT(godot::ObjectID(m_object_id).is_valid(), "Command queue should have an assigned object");

		if (m_owner_id != godot::OS::get_singleton()->get_thread_caller_id())
		{
			DEBUG_PRINT_ERROR("Should be run by the owning thread");
			return;
		}

		CommandServer::get_singleton()->AddCommands(m_object_id, std::move(m_command_buffer)); // m_command_buffers guaranteed to be empty()
	}

	void CommandWriter::_bind_methods()
	{
		godot::ClassDB::bind_vararg_method(godot::METHOD_FLAGS_DEFAULT, "add_command", &CommandWriter::_add_command_vararg, godot::MethodInfo("add_command", godot::PropertyInfo(godot::Variant::STRING, "command")));
		godot::ClassDB::bind_static_method(get_class_static(), godot::D_METHOD("make_writer", "object"), &CommandWriter::MakeWriter);
		godot::ClassDB::bind_method(godot::D_METHOD("get_owner_thread"), &CommandWriter::GetObject);
		godot::ClassDB::bind_method(godot::D_METHOD("get_object"), &CommandWriter::GetObject);
	}
}