#include "GodotCommandBuffer.h"

#include "Util/Debug.h"
#include "Util/StackAllocator.h"

namespace voxel_game
{
	template<> VariantType GetVariantType<uint8_t>() { return VariantType::UINT8; }
	template<> VariantType GetVariantType<uint16_t>() { return VariantType::UINT16; }
	template<> VariantType GetVariantType<uint32_t>() { return VariantType::UINT32; }
	template<> VariantType GetVariantType<uint64_t>() { return VariantType::UINT64; }
	template<> VariantType GetVariantType<int8_t>() { return VariantType::INT8; }
	template<> VariantType GetVariantType<int16_t>() { return VariantType::INT16; }
	template<> VariantType GetVariantType<int32_t>() { return VariantType::INT32; }
	template<> VariantType GetVariantType<int64_t>() { return VariantType::INT64; }
	template<> VariantType GetVariantType<float>() { return VariantType::FLOAT32; }
	template<> VariantType GetVariantType<double>() { return VariantType::FLOAT64; }
	template<> VariantType GetVariantType<godot::String>() { return VariantType::STRING; }
	template<> VariantType GetVariantType<godot::Vector2>() { return VariantType::VECTOR2; }
	template<> VariantType GetVariantType<godot::Vector2i>() { return VariantType::VECTOR2I; }
	template<> VariantType GetVariantType<godot::Rect2>() { return VariantType::RECT2; }
	template<> VariantType GetVariantType<godot::Rect2i>() { return VariantType::RECT2I; }
	template<> VariantType GetVariantType<godot::Vector3>() { return VariantType::VECTOR3; }
	template<> VariantType GetVariantType<godot::Vector3i>() { return VariantType::VECTOR3I; }
	template<> VariantType GetVariantType<godot::Transform2D>() { return VariantType::TRANSFORM2D; }
	template<> VariantType GetVariantType<godot::Vector4>() { return VariantType::VECTOR4; }
	template<> VariantType GetVariantType<godot::Vector4i>() { return VariantType::VECTOR4I; }
	template<> VariantType GetVariantType<godot::Plane>() { return VariantType::PLANE; }
	template<> VariantType GetVariantType<godot::Quaternion>() { return VariantType::QUATERNION; }
	template<> VariantType GetVariantType<godot::AABB>() { return VariantType::AABB; }
	template<> VariantType GetVariantType<godot::Basis>() { return VariantType::BASIS; }
	template<> VariantType GetVariantType<godot::Transform3D>() { return VariantType::TRANSFORM3D; }
	template<> VariantType GetVariantType<godot::Projection>() { return VariantType::PROJECTION; }
	template<> VariantType GetVariantType<godot::Color>() { return VariantType::COLOR; }
	template<> VariantType GetVariantType<godot::StringName>() { return VariantType::STRING_NAME; }
	template<> VariantType GetVariantType<godot::NodePath>() { return VariantType::NODE_PATH; }
	template<> VariantType GetVariantType<godot::RID>() { return VariantType::RID; }
	template<> VariantType GetVariantType<godot::ObjectID>() { return VariantType::UINT64; }
	template<> VariantType GetVariantType<godot::Callable>() { return VariantType::CALLABLE; }
	template<> VariantType GetVariantType<godot::Signal>() { return VariantType::SIGNAL; }
	template<> VariantType GetVariantType<godot::Dictionary>() { return VariantType::DICTIONARY; }
	template<> VariantType GetVariantType<godot::Array>() { return VariantType::ARRAY; }
	template<> VariantType GetVariantType<godot::PackedByteArray>() { return VariantType::PACKED_BYTE_ARRAY; }
	template<> VariantType GetVariantType<godot::PackedInt32Array>() { return VariantType::PACKED_INT32_ARRAY; }
	template<> VariantType GetVariantType<godot::PackedInt64Array>() { return VariantType::PACKED_INT64_ARRAY; }
	template<> VariantType GetVariantType<godot::PackedFloat32Array>() { return VariantType::PACKED_FLOAT32_ARRAY; }
	template<> VariantType GetVariantType<godot::PackedFloat64Array>() { return VariantType::PACKED_FLOAT64_ARRAY; }
	template<> VariantType GetVariantType<godot::PackedStringArray>() { return VariantType::PACKED_STRING_ARRAY; }
	template<> VariantType GetVariantType<godot::PackedVector2Array>() { return VariantType::PACKED_VECTOR2_ARRAY; }
	template<> VariantType GetVariantType<godot::PackedVector3Array>() { return VariantType::PACKED_VECTOR3_ARRAY; }
	template<> VariantType GetVariantType<godot::PackedColorArray>() { return VariantType::PACKED_COLOR_ARRAY; }
	template<> VariantType GetVariantType<godot::PackedVector4Array>() { return VariantType::PACKED_VECTOR4_ARRAY; }

	// Read an encoded type in a byte stream. Copies the object to the variant and then destroys the object in the buffer
	template<class T>
	std::byte* ReadVariantInternal(std::byte* buffer_pos, std::byte* buffer_end, godot::Variant& argument)
	{
#if defined(DEBUG_ENABLED)
		if (buffer_pos + sizeof(T) > buffer_end)
		{
			DEBUG_PRINT_ERROR("Not enough space to read another encoded variant value");
			return buffer_end;
		}
#endif

		T* value = reinterpret_cast<T*>(buffer_pos);

		argument = godot::Variant(*value);

		std::destroy_at(value);

		return buffer_pos + sizeof(T);
	}

	// Read an encoded variant in a byte stream. Copies the object to the variant and then destroys the object in the buffer
	std::byte* ReadVariant(std::byte* buffer_pos, std::byte* buffer_end, godot::Variant& argument)
	{
#if defined(DEBUG_ENABLED)
		if (buffer_pos + sizeof(VariantType) > buffer_end)
		{
			DEBUG_PRINT_ERROR("Not enough space to read another encoded variant");
			return buffer_end;
		}
#endif

		VariantType* type = reinterpret_cast<VariantType*>(buffer_pos);
		
		buffer_pos += sizeof(VariantType);

		switch (*type)
		{
		case VariantType::NIL:
			argument = godot::Variant();
			return buffer_pos;

		case VariantType::TRUE:
			argument = godot::Variant(true);
			return buffer_pos;

		case VariantType::FALSE:
			argument = godot::Variant(false);
			return buffer_pos;

		case VariantType::UINT8:
			return ReadVariantInternal<uint8_t>(buffer_pos, buffer_end, argument);

		case VariantType::UINT16:
			return ReadVariantInternal<uint16_t>(buffer_pos, buffer_end, argument);

		case VariantType::UINT32:
			return ReadVariantInternal<uint32_t>(buffer_pos, buffer_end, argument);

		case VariantType::UINT64:
			return ReadVariantInternal<uint64_t>(buffer_pos, buffer_end, argument);

		case VariantType::INT8:
			return ReadVariantInternal<int8_t>(buffer_pos, buffer_end, argument);

		case VariantType::INT16:
			return ReadVariantInternal<int16_t>(buffer_pos, buffer_end, argument);

		case VariantType::INT32:
			return ReadVariantInternal<int32_t>(buffer_pos, buffer_end, argument);

		case VariantType::INT64:
			return ReadVariantInternal<int64_t>(buffer_pos, buffer_end, argument);

		case VariantType::FLOAT32:
			return ReadVariantInternal<float>(buffer_pos, buffer_end, argument);

		case VariantType::FLOAT64:
			return ReadVariantInternal<double>(buffer_pos, buffer_end, argument);

		case VariantType::STRING:
			return ReadVariantInternal<godot::String>(buffer_pos, buffer_end, argument);

		case VariantType::VECTOR2:
			return ReadVariantInternal<godot::Vector2>(buffer_pos, buffer_end, argument);

		case VariantType::VECTOR2I:
			return ReadVariantInternal<godot::Vector2i>(buffer_pos, buffer_end, argument);

		case VariantType::RECT2:
			return ReadVariantInternal<godot::Rect2>(buffer_pos, buffer_end, argument);

		case VariantType::RECT2I:
			return ReadVariantInternal<godot::Rect2i>(buffer_pos, buffer_end, argument);

		case VariantType::VECTOR3:
			return ReadVariantInternal<godot::Vector3>(buffer_pos, buffer_end, argument);

		case VariantType::VECTOR3I:
			return ReadVariantInternal<godot::Vector3i>(buffer_pos, buffer_end, argument);

		case VariantType::TRANSFORM2D:
			return ReadVariantInternal<godot::Transform2D>(buffer_pos, buffer_end, argument);

		case VariantType::VECTOR4:
			return ReadVariantInternal<godot::Vector4>(buffer_pos, buffer_end, argument);

		case VariantType::VECTOR4I:
			return ReadVariantInternal<godot::Vector4i>(buffer_pos, buffer_end, argument);

		case VariantType::PLANE:
			return ReadVariantInternal<godot::Plane>(buffer_pos, buffer_end, argument);

		case VariantType::QUATERNION:
			return ReadVariantInternal<godot::Quaternion>(buffer_pos, buffer_end, argument);

		case VariantType::AABB:
			return ReadVariantInternal<godot::AABB>(buffer_pos, buffer_end, argument);

		case VariantType::BASIS:
			return ReadVariantInternal<godot::Basis>(buffer_pos, buffer_end, argument);

		case VariantType::TRANSFORM3D:
			return ReadVariantInternal<godot::Transform3D>(buffer_pos, buffer_end, argument);

		case VariantType::PROJECTION:
			return ReadVariantInternal<godot::Projection>(buffer_pos, buffer_end, argument);

		case VariantType::COLOR:
			return ReadVariantInternal<godot::Color>(buffer_pos, buffer_end, argument);

		case VariantType::STRING_NAME:
			return ReadVariantInternal<godot::StringName>(buffer_pos, buffer_end, argument);

		case VariantType::NODE_PATH:
			return ReadVariantInternal<godot::NodePath>(buffer_pos, buffer_end, argument);

		case VariantType::RID:
			return ReadVariantInternal<godot::RID>(buffer_pos, buffer_end, argument);

		case VariantType::OBJECT:
			return ReadVariantInternal<godot::Object*>(buffer_pos, buffer_end, argument);

		case VariantType::REFCOUNTED:
			return ReadVariantInternal<godot::Ref<godot::RefCounted>>(buffer_pos, buffer_end, argument);

		case VariantType::CALLABLE:
			return ReadVariantInternal<godot::Callable>(buffer_pos, buffer_end, argument);

		case VariantType::SIGNAL:
			return ReadVariantInternal<godot::Signal>(buffer_pos, buffer_end, argument);

		case VariantType::DICTIONARY:
			return ReadVariantInternal<godot::Dictionary>(buffer_pos, buffer_end, argument);

		case VariantType::ARRAY:
			return ReadVariantInternal<godot::Array>(buffer_pos, buffer_end, argument);

		case VariantType::PACKED_BYTE_ARRAY:
			return ReadVariantInternal<godot::PackedByteArray>(buffer_pos, buffer_end, argument);

		case VariantType::PACKED_INT32_ARRAY:
			return ReadVariantInternal<godot::PackedInt32Array>(buffer_pos, buffer_end, argument);

		case VariantType::PACKED_INT64_ARRAY:
			return ReadVariantInternal<godot::PackedInt64Array>(buffer_pos, buffer_end, argument);

		case VariantType::PACKED_FLOAT32_ARRAY:
			return ReadVariantInternal<godot::PackedFloat32Array>(buffer_pos, buffer_end, argument);

		case VariantType::PACKED_FLOAT64_ARRAY:
			return ReadVariantInternal<godot::PackedFloat64Array>(buffer_pos, buffer_end, argument);

		case VariantType::PACKED_STRING_ARRAY:
			return ReadVariantInternal<godot::PackedStringArray>(buffer_pos, buffer_end, argument);

		case VariantType::PACKED_VECTOR2_ARRAY:
			return ReadVariantInternal<godot::PackedVector2Array>(buffer_pos, buffer_end, argument);

		case VariantType::PACKED_VECTOR3_ARRAY:
			return ReadVariantInternal<godot::PackedVector3Array>(buffer_pos, buffer_end, argument);

		case VariantType::PACKED_COLOR_ARRAY:
			return ReadVariantInternal<godot::PackedColorArray>(buffer_pos, buffer_end, argument);

		case VariantType::PACKED_VECTOR4_ARRAY:
			return ReadVariantInternal<godot::PackedVector4Array>(buffer_pos, buffer_end, argument);

		default:
			DEBUG_PRINT_ERROR("Invalid encoded variant type");
			return buffer_end;
		}
	}

	void WriteGenericVariant(GodotCommandBuffer::Storage& buffer, const godot::Variant& argument)
	{
		switch (argument.get_type())
		{
		case godot::Variant::NIL:
			WriteType<VariantType>(buffer, VariantType::NIL);
			break;

		case godot::Variant::BOOL:
			WriteVariant<bool>(buffer, argument);
			break;

		case godot::Variant::INT:
		{
			int64_t value = argument;

			if (value < 0)
			{
				if (value >= INT8_MIN)
				{
					WriteVariant<int8_t>(buffer, std::move(value));
				}
				else if (value >= INT16_MIN)
				{
					WriteVariant<int16_t>(buffer, std::move(value));
				}
				else if (value >= INT32_MIN)
				{
					WriteVariant<int32_t>(buffer, std::move(value));
				}
				else
				{
					WriteVariant<int64_t>(buffer, std::move(value));
				}
			}
			else
			{
				if (value <= UINT8_MAX)
				{
					WriteVariant<uint8_t>(buffer, std::move(value));
				}
				else if (value <= UINT16_MAX)
				{
					WriteVariant<uint16_t>(buffer, std::move(value));
				}
				else if (value <= UINT32_MAX)
				{
					WriteVariant<uint32_t>(buffer, std::move(value));
				}
				else
				{
					WriteVariant<uint64_t>(buffer, std::move(value));
				}
			}
			break;
		}

		case godot::Variant::FLOAT:
			WriteVariant<double>(buffer, argument);
			break;

		case godot::Variant::STRING:
			WriteVariant<godot::String>(buffer, argument);
			break;

		case godot::Variant::VECTOR2:
			WriteVariant<godot::Vector2>(buffer, argument);
			break;

		case godot::Variant::VECTOR2I:
			WriteVariant<godot::Vector2i>(buffer, argument);
			break;

		case godot::Variant::RECT2:
			WriteVariant<godot::Rect2>(buffer, argument);
			break;

		case godot::Variant::RECT2I:
			WriteVariant<godot::Rect2i>(buffer, argument);
			break;

		case godot::Variant::VECTOR3:
			WriteVariant<godot::Vector3>(buffer, argument);
			break;

		case godot::Variant::VECTOR3I:
			WriteVariant<godot::Vector3i>(buffer, argument);
			break;

		case godot::Variant::TRANSFORM2D:
			WriteVariant<godot::Transform2D>(buffer, argument);
			break;

		case godot::Variant::VECTOR4:
			WriteVariant<godot::Vector4>(buffer, argument);
			break;

		case godot::Variant::VECTOR4I:
			WriteVariant<godot::Vector4i>(buffer, argument);
			break;

		case godot::Variant::PLANE:
			WriteVariant<godot::Plane>(buffer, argument);
			break;

		case godot::Variant::QUATERNION:
			WriteVariant<godot::Quaternion>(buffer, argument);
			break;

		case godot::Variant::AABB:
			WriteVariant<godot::AABB>(buffer, argument);
			break;

		case godot::Variant::BASIS:
			WriteVariant<godot::Basis>(buffer, argument);
			break;

		case godot::Variant::TRANSFORM3D:
			WriteVariant<godot::Transform3D>(buffer, argument);
			break;

		case godot::Variant::PROJECTION:
			WriteVariant<godot::Projection>(buffer, argument);
			break;

		case godot::Variant::COLOR:
			WriteVariant<godot::Color>(buffer, argument);
			break;

		case godot::Variant::STRING_NAME:
			WriteVariant<godot::StringName>(buffer, argument);
			break;

		case godot::Variant::NODE_PATH:
			WriteVariant<godot::NodePath>(buffer, argument);
			break;

		case godot::Variant::RID:
			WriteVariant<godot::RID>(buffer, argument);
			break;

		case godot::Variant::OBJECT:
			WriteVariant<godot::Object*>(buffer, argument);
			break;

		case godot::Variant::CALLABLE:
			WriteVariant<godot::Callable>(buffer, argument);
			break;

		case godot::Variant::SIGNAL:
			WriteVariant<godot::Signal>(buffer, argument);
			break;

		case godot::Variant::DICTIONARY:
			WriteVariant<godot::Dictionary>(buffer, argument);
			break;

		case godot::Variant::ARRAY:
			WriteVariant<godot::Array>(buffer, argument);
			break;

		case godot::Variant::PACKED_BYTE_ARRAY:
			WriteVariant<godot::PackedByteArray>(buffer, argument);
			break;

		case godot::Variant::PACKED_INT32_ARRAY:
			WriteVariant<godot::PackedInt32Array>(buffer, argument);
			break;

		case godot::Variant::PACKED_INT64_ARRAY:
			WriteVariant<godot::PackedInt64Array>(buffer, argument);
			break;

		case godot::Variant::PACKED_FLOAT32_ARRAY:
			WriteVariant<godot::PackedFloat32Array>(buffer, argument);
			break;

		case godot::Variant::PACKED_FLOAT64_ARRAY:
			WriteVariant<godot::PackedFloat64Array>(buffer, argument);
			break;

		case godot::Variant::PACKED_STRING_ARRAY:
			WriteVariant<godot::PackedStringArray>(buffer, argument);
			break;

		case godot::Variant::PACKED_VECTOR2_ARRAY:
			WriteVariant<godot::PackedVector2Array>(buffer, argument);
			break;

		case godot::Variant::PACKED_VECTOR3_ARRAY:
			WriteVariant<godot::PackedVector3Array>(buffer, argument);
			break;

		case godot::Variant::PACKED_COLOR_ARRAY:
			WriteVariant<godot::PackedColorArray>(buffer, argument);
			break;

		case godot::Variant::PACKED_VECTOR4_ARRAY:
			WriteVariant<godot::PackedVector4Array>(buffer, argument);
			break;

		default:
			DEBUG_PRINT_ERROR("Invalid variant type");
			WriteType<VariantType>(buffer, VariantType::NIL);
			break;
		}
	}

	std::byte* ProcessCommand(godot::Variant& object, std::byte* buffer_pos, std::byte* buffer_end)
	{
		DEBUG_ASSERT(buffer_end >= buffer_pos, "The buffer position should not be beyond the end");

#if defined(DEBUG_ENABLED)
		if (buffer_pos + sizeof(Command) > buffer_end)
		{
			DEBUG_PRINT_ERROR(godot::vformat("Command buffer doesn't fit the command and its arguments (%d out of range)", buffer_end - buffer_pos - sizeof(Command)));
			return buffer_end;
		}
#endif

		Command* header = reinterpret_cast<Command*>(buffer_pos);
		buffer_pos += sizeof(Command);

		DEBUG_ASSERT(!header->command.is_empty(), "The command should not be an empty string");

#if defined(DEBUG_ENABLED)
		if (header->argcount > 16)
		{
			DEBUG_PRINT_ERROR(godot::vformat("Command buffers support up to 16 arguments per command (%d out of range)", header->argcount));
			return buffer_end;
		}
#endif

		const size_t k_command_args_alloc_size = (sizeof(const godot::Variant*) * 16) + (sizeof(godot::Variant) * 16);
		StackAllocator<k_command_args_alloc_size> variant_alloc;

		// Read arguments and store them in temporary variants for call
		godot::Variant* args = variant_alloc.NewArray<godot::Variant>(header->argcount);
		const godot::Variant** argptrs = variant_alloc.NewArray<const godot::Variant*>(header->argcount);

		for (size_t i = 0; i < header->argcount; i++)
		{
			buffer_pos = ReadVariant(buffer_pos, buffer_end, args[i]);

#if defined(DEBUG_ENABLED)
			if (buffer_pos > buffer_end)
			{
				DEBUG_PRINT_ERROR(godot::vformat("Command buffer doesn't fit the command and its arguments (%d out of range)", buffer_end - buffer_pos));
				return buffer_end;
			}
#endif

			argptrs[i] = &args[i];
		}

		// Do the call on the object and handle any error
		godot::Variant ret;
		GDExtensionCallError error;
		object.callp(header->command, argptrs, header->argcount, ret, error);

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

			DEBUG_PRINT_ERROR(godot::vformat("Failed to call %s: %s. Error at argument %d. Expected %d arguments. Actual %d arguments", header->command, error_type_str, error.argument, error.expected, header->argcount));
		}

		variant_alloc.DeleteArray<godot::Variant>(args, header->argcount);

		variant_alloc.DeleteArray<const godot::Variant*>(argptrs, header->argcount);

		header->~Command();

		return buffer_pos;
	}

	std::byte* SkipCommand(std::byte* buffer_pos, std::byte* buffer_end)
	{
#if defined(DEBUG_ENABLED)
		if (buffer_pos + sizeof(Command) > buffer_end)
		{
			DEBUG_PRINT_ERROR(godot::vformat("Command buffer doesn't fit the command and its arguments (%d out of range)", buffer_end - buffer_pos - sizeof(Command)));
			return buffer_end;
		}
#endif

		Command* header = reinterpret_cast<Command*>(buffer_pos);
		buffer_pos += sizeof(Command);

		DEBUG_ASSERT(!header->command.is_empty(), "The command should not be an empty string");

#if defined(DEBUG_ENABLED)
		if (header->argcount > 16)
		{
			DEBUG_PRINT_ERROR(godot::vformat("Command buffers support up to 16 arguments per command (%d out of range)", header->argcount));
			return buffer_end;
		}
#endif

		for (size_t i = 0; i < header->argcount; i++)
		{
			godot::Variant var;
			buffer_pos = ReadVariant(buffer_pos, buffer_end, var);

#if defined(DEBUG_ENABLED)
			if (buffer_pos > buffer_end)
			{
				DEBUG_PRINT_ERROR(godot::vformat("Command buffer doesn't fit the command and its arguments (%d out of range)", buffer_end - buffer_pos));
				return buffer_end;
			}
#endif
		}

		header->~Command();

		return buffer_pos;
	}

	GodotCommandBuffer::GodotCommandBuffer()
	{
		m_data.reserve(k_starting_buffer_size);
	}

	GodotCommandBuffer::~GodotCommandBuffer()
	{
		Clear();
	}

	GodotCommandBuffer::GodotCommandBuffer(GodotCommandBuffer&& other) noexcept
	{
		*this = std::move(other);
	}

	GodotCommandBuffer& GodotCommandBuffer::operator=(GodotCommandBuffer&& other) noexcept
	{
		Clear();

		m_data = std::move(other.m_data);
		m_num_commands = other.m_num_commands;
		m_start = other.m_start;

		other.m_data.reserve(k_starting_buffer_size);
		other.m_num_commands = 0;
		other.m_start = 0;

		return *this;
	}

	void GodotCommandBuffer::AddCommandVararg(const godot::StringName& command, const godot::Variant** args, uint8_t argcount)
	{
		DEBUG_ASSERT(!command.is_empty(), "The command should not be an empty string");
		DEBUG_ASSERT(m_start == 0, "We shouldn't be adding commands when we are already processing the buffer");
		DEBUG_ASSERT(argcount == 0 || args != nullptr, "The arguments array should be valid if there are more than 0 arguments");

		size_t command_pos = m_data.size();

		m_data.resize(command_pos + sizeof(Command));

		new(m_data.data() + command_pos) Command{ command, (uint8_t)argcount };

		for (uint8_t arg = 0; arg < argcount; arg++)
		{
			WriteVariant(m_data, *args[arg]);
		}

		m_num_commands++;
	}

	size_t GodotCommandBuffer::ProcessCommands(godot::Object* object, size_t max)
	{
		DEBUG_ASSERT(object != nullptr, "The object we are trying to process on should be valid");

		if (m_num_commands == 0)
		{
			return 0;
		}

		godot::Variant object_var = object;

		size_t num_processed = 0;

		std::byte* buffer_start = m_data.data() + m_start;
		std::byte* buffer_pos = buffer_start;
		std::byte* buffer_end = m_data.data() + m_data.size();

		while (buffer_pos != buffer_end)
		{
			buffer_pos = ProcessCommand(object_var, buffer_pos, buffer_end);
			num_processed++;

			// We reached the max commands we set for ourself
			if (num_processed == max)
			{
				break;
			}
		}

		m_start += buffer_pos - buffer_start;
		DEBUG_ASSERT(m_num_commands >= num_processed, "We processed more commands than we have");
		m_num_commands -= num_processed;

		return num_processed;
	}

	size_t GodotCommandBuffer::ProcessCommandsUntyped(void* object, size_t max)
	{
		return ProcessCommands(reinterpret_cast<godot::Object*>(object), max);
	}

	size_t GodotCommandBuffer::NumCommands() const
	{
		return m_num_commands;
	}

	void GodotCommandBuffer::Clear()
	{
		// Go through the buffer and read command data as if we were processing the commands but only destroy the data

		std::byte* buffer_pos = m_data.data() + m_start;
		std::byte* buffer_end = m_data.data() + m_data.size();
		while (buffer_pos != buffer_end)
		{
			buffer_pos = SkipCommand(buffer_pos, buffer_end);
		}

		m_data.clear();
		m_start = 0;
		m_num_commands = 0;
	}

	void GodotCommandBuffer::ShrinkToFit()
	{
		m_data.shrink_to_fit();
	}
}