#include "CommandQueue.h"

#include "Util/Debug.h"

#include <godot_cpp/core/class_db.hpp>

namespace voxel_game
{
	CommandQueue::CommandQueue()
	{
		Flush();
	}

	CommandQueue::~CommandQueue()
	{}

	void CommandQueue::SetObject(uint64_t object_id)
	{
		m_object_id = object_id;
	}

	uint64_t CommandQueue::GetObject()
	{
		return m_object_id;
	}

	void CommandQueue::Flush()
	{
		// Swap(m_command_buffer);

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
		m_command_offsets.push_back(m_command_buffer.size());

		uint32_t command_offset = m_command_offsets.back();
		uint32_t command_size = sizeof(godot::StringName) + (sizeof(godot::Variant) * argcount);

		m_command_buffer.resize(command_offset + command_size);

		uint8_t* buffer_pos = &m_command_buffer[command_offset];

		godot::StringName* command_ptr = memnew_placement(buffer_pos, godot::StringName);
		*command_ptr = command;
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
		godot::ClassDB::bind_method(godot::D_METHOD("set_object", "object_id"), &CommandQueue::SetObject);
		godot::ClassDB::bind_method(godot::D_METHOD("get_object"), &CommandQueue::GetObject);

		ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "object", godot::PROPERTY_HINT_OBJECT_ID), "set_object", "get_object");
	}

	void CommandQueue::_cleanup_methods()
	{

	}
}