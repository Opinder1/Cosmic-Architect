#pragma once

#include "CommandBufferEntry.h"

#include "Util/Nocopy.h"
#include "Util/Debug.h"

#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

#include <vector>

namespace voxel_game
{
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
	class GodotCommandBuffer : Nocopy
	{
	public:
		using Storage = std::vector<std::byte>;

	public:
		GodotCommandBuffer();
		~GodotCommandBuffer();

		GodotCommandBuffer(GodotCommandBuffer&& other) noexcept;

		GodotCommandBuffer& operator=(GodotCommandBuffer&& other) noexcept;

		// Register a new command for the queue
		template<class... Args>
		void AddCommand(const godot::StringName& command, Args&&... p_args);

		void AddCommandVararg(const godot::StringName& command, const godot::Variant** args, uint8_t argcount);

		// Process only up to a certain number of commands and return how many were processed (0 for max to process all)
		size_t ProcessCommands(godot::Object* object, size_t max = k_process_all_commands);

		size_t NumCommands() const;

		// Destroy all remaining commands and arguments
		void Clear();

		void ShrinkToFit();

	private:
		Storage m_data;
		size_t m_start = 0;
		size_t m_num_commands = 0;
	};

	// Get a variant type id for a C++ type
	template<class T>
	VariantType GetVariantType();

	// Write a variant to a buffer
	void WriteGenericVariant(GodotCommandBuffer::Storage& buffer, const godot::Variant& argument);

	// Write a type interpreted as a variant to a buffer
	template<class T>
	void WriteVariant(GodotCommandBuffer::Storage& buffer, T&& argument)
	{
		// Get the plain type
		using PlainT = std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>;

		if constexpr (std::is_same_v<PlainT, bool>) // Efficiently write booleans in just the type enum
		{
			WriteType<VariantType>(buffer, argument ? VariantType::TRUE : VariantType::FALSE);
		}
		else if constexpr (std::is_same_v<PlainT, godot::Variant>) // We were given an actual variant
		{
			WriteGenericVariant(buffer, argument);
		}
		else if constexpr (std::is_base_of_v<godot::RefCounted, PlainT>) // Specific handle for classes that inherit from refcounted
		{
			WriteType<VariantType>(buffer, VariantType::REFCOUNTED);
			WriteType<godot::Ref<PlainT>>(buffer, argument);
		}
		else if constexpr (std::is_base_of_v<godot::Object, PlainT> && std::is_pointer_v<T>) // Specific handle for classes that inherit from object
		{
			WriteType<VariantType>(buffer, VariantType::OBJECT);
			WriteType<PlainT*>(buffer, std::forward<T>(argument));
		}
		else // Write a C++ type that can be stored in a variant
		{
			WriteType<VariantType>(buffer, GetVariantType<PlainT>());
			WriteType<PlainT>(buffer, std::forward<T>(argument));
		}
	}

	template<class... Args>
	void GodotCommandBuffer::AddCommand(const godot::StringName& command, Args&&... args)
	{
		DEBUG_ASSERT(!command.is_empty(), "The command should not be an empty string");
		DEBUG_ASSERT(m_start == 0, "We shouldn't be adding commands when we are already processing the buffer");

		size_t command_pos = m_data.size();
		m_data.resize(command_pos + sizeof(Command));

		new(m_data.data() + command_pos) Command{ command, sizeof...(args) };

		(WriteVariant<Args>(m_data, std::forward<Args>(args)), ...);

		m_num_commands++;
	}
}