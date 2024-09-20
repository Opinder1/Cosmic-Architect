#pragma once

#include "Util/Nocopy.h"
#include "Util/Debug.h"

#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

#include <vector>

namespace voxel_game
{
	constexpr const size_t k_starting_buffer_size = 4096;

	// Command header
	struct alignas(1) Command
	{
		godot::StringName command;
		uint8_t argcount = 0;
	};

	// Encoded variant type header
	enum class VariantType : uint8_t
	{
		NIL,
		TRUE, // Boolean true which is more efficiently stored
		FALSE, // Boolean false which is more efficiently stored
		UINT8,
		UINT16,
		UINT32,
		UINT64,
		INT8,
		INT16,
		INT32,
		INT64,
		FLOAT32,
		FLOAT64,
		STRING,
		VECTOR2,
		VECTOR2I,
		RECT2,
		RECT2I,
		VECTOR3,
		VECTOR3I,
		TRANSFORM2D,
		VECTOR4,
		VECTOR4I,
		PLANE,
		QUATERNION,
		AABB,
		BASIS,
		TRANSFORM3D,
		PROJECTION,
		COLOR,
		STRING_NAME,
		NODE_PATH,
		RID,
		OBJECT,
		REFCOUNTED,
		CALLABLE,
		SIGNAL,
		DICTIONARY,
		ARRAY,
		PACKED_BYTE_ARRAY,
		PACKED_INT32_ARRAY,
		PACKED_INT64_ARRAY,
		PACKED_FLOAT32_ARRAY,
		PACKED_FLOAT64_ARRAY,
		PACKED_STRING_ARRAY,
		PACKED_VECTOR2_ARRAY,
		PACKED_VECTOR3_ARRAY,
		PACKED_COLOR_ARRAY,
		PACKED_VECTOR4_ARRAY,
	};

	// Buffer which commands can be added to and processed in the order they are added.
	// We pay a cost for expensive types when writing the command name StringName and expensive arguments.
	// We also pay a cost when moving expensive types into variants for execution but not inexpensive types nor the command name StringName
	class CommandBuffer : Nocopy
	{
	public:
		using Storage = std::vector<std::byte>;

	public:
		CommandBuffer();
		~CommandBuffer();

		CommandBuffer& operator=(CommandBuffer&& other) noexcept;

		// Register a new command for the queue
		template<class... Args>
		void AddCommand(const godot::StringName& command, Args&&... p_args);

		void AddCommandVararg(const godot::StringName& command, const godot::Variant** args, uint8_t argcount);

		// Process only up to a certain number of commands and return how many were processed (0 for max to process all)
		size_t ProcessCommands(uint64_t object_id, size_t max = 0);
		size_t ProcessCommands(godot::Object* object, size_t max = 0);

		size_t NumCommands() const;

		// Destroy all remaining commands and arguments. Optionally reallocate to free memory if we just had a huge buffer
		void Clear(bool reallocate = true);

	private:
		Storage m_data;
		size_t m_start = 0;
		size_t m_num_commands = 0;
	};

	// Get a variant type id for a C++ type
	template<class T>
	VariantType GetVariantType();

	// Write a variant to a buffer
	void WriteGenericVariant(const godot::Variant& argument, CommandBuffer::Storage& buffer);

	// Write a type to a buffer
	template<class T>
	void WriteType(T&& data, CommandBuffer::Storage& buffer)
	{
		// Get the plain type while allowing pointers
		using PlainT = std::remove_cv_t<std::remove_reference_t<T>>;

		size_t pos = buffer.size();
		buffer.resize(pos + sizeof(PlainT));

		new (buffer.data() + pos) PlainT(std::forward<T>(data));
	}

	// Write a type interpreted as a variant to a buffer
	template<class T>
	void WriteVariant(T&& argument, CommandBuffer::Storage& buffer)
	{
		// Get the plain type
		using PlainT = std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>;

		if constexpr (std::is_same_v<PlainT, bool>) // Efficiently write booleans in just the type enum
		{
			WriteType<VariantType>(argument ? VariantType::TRUE : VariantType::FALSE, buffer);
		}
		else if constexpr (std::is_same_v<PlainT, godot::Variant>) // We were given an actual variant
		{
			WriteGenericVariant(argument, buffer);
		}
		else if constexpr (std::is_base_of_v<godot::RefCounted, PlainT>) // Specific handle for classes that inherit from refcounted
		{
			WriteType<VariantType>(VariantType::REFCOUNTED, buffer);
			WriteType<godot::Ref<PlainT>>(argument, buffer);
		}
		else if constexpr (std::is_base_of_v<godot::Object, PlainT> && std::is_pointer_v<T>) // Specific handle for classes that inherit from object
		{
			WriteType<VariantType>(VariantType::OBJECT, buffer);
			WriteType<T>(std::forward<T>(argument), buffer);
		}
		else // Write a C++ type that can be stored in a variant
		{
			WriteType<VariantType>(GetVariantType<PlainT>(), buffer);
			WriteType<T>(std::forward<T>(argument), buffer);
		}
	}

	template<class... Args>
	void CommandBuffer::AddCommand(const godot::StringName& command, Args&&... args)
	{
		DEBUG_ASSERT(!command.is_empty(), "The command should not be an empty string");
		DEBUG_ASSERT(m_start == 0, "We shouldn't be adding commands when we are already processing the buffer");

		size_t command_pos = m_data.size();
		m_data.resize(command_pos + sizeof(Command));

		new(m_data.data() + command_pos) Command{ command, sizeof...(args) };

		(WriteVariant<Args>(std::forward<Args>(args), m_data), ...);

		m_num_commands++;
	}
}