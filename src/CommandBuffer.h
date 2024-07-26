#pragma once

#include "Util/Nocopy.h"
#include "Util/Debug.h"

#include <godot_cpp/variant/variant.hpp>

#include <vector>

namespace voxel_game
{
	constexpr const size_t k_starting_buffer_size = 4096;

	struct alignas(1) Command
	{
		godot::StringName command;
		uint8_t argcount = 0;
	};

	enum class VariantType : uint8_t
	{
		NIL,
		TRUE,
		FALSE,
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

	template<class T>
	VariantType GetVariantType();

	template<class T>
	void WriteType(T&& data, std::vector<std::byte>& buffer)
	{
		using RawT = std::remove_cv_t<std::remove_reference_t<T>>;

		size_t pos = buffer.size();
		buffer.resize(pos + sizeof(RawT));

		new (buffer.data() + pos) RawT(std::forward<T>(data));
	}

	void WriteGenericVariant(const godot::Variant& argument, std::vector<std::byte>& buffer);

	template<class T>
	void WriteVariant(T&& argument, std::vector<std::byte>& buffer)
	{
		using RawT = std::remove_cv_t<std::remove_reference_t<T>>;

		if constexpr (std::is_same_v<RawT, bool>)
		{
			WriteType<VariantType>(argument ? VariantType::TRUE : VariantType::FALSE, buffer);
		}
		else if constexpr (std::is_same_v<RawT, godot::Variant>)
		{
			WriteGenericVariant(argument, buffer);
		}
		else if constexpr (std::is_same_v<RawT, godot::Object*>)
		{
			WriteType<VariantType>(VariantType::OBJECT, buffer);
			WriteGenericVariant(argument->get_instance_id(), buffer);
		}
		else
		{
			WriteType<VariantType>(GetVariantType<RawT>(), buffer);
			WriteType<T>(std::forward<T>(argument), buffer);
		}
	}

	// Buffer which commands can be added to and processed in the order they are added
	class CommandBuffer : Nocopy
	{
		using Storage = std::vector<std::byte>;

	public:
		CommandBuffer();
		~CommandBuffer();

		CommandBuffer& operator=(CommandBuffer&& other) noexcept;

		// Register a new command for the queue
		template<class... Args>
		void AddCommand(const godot::StringName& command, Args&&... p_args)
		{
			DEBUG_ASSERT(!command.is_empty(), "The command should not be an empty string");
			DEBUG_ASSERT(m_start == 0, "We shouldn't be adding commands when we are already processing the buffer");

			size_t command_pos = m_data.size();
			m_data.resize(command_pos + sizeof(Command));

			new(m_data.data() + command_pos) Command{ command, sizeof...(p_args) };

			(WriteVariant<Args>(std::forward<Args>(p_args), m_data), ...);

			m_num_commands++;
		}

		void AddCommandVararg(const godot::StringName& command, const godot::Variant** args, uint8_t argcount);

		// Process only up to a certain number of commands and return how many were processed (0 for max to process all)
		size_t ProcessCommands(uint64_t object_id, size_t max = 0);
		size_t ProcessCommands(godot::Object* object, size_t max = 0);

		size_t NumCommands() const;

		void Clear(bool reallocate = true);

	private:
		Storage m_data;
		size_t m_start = 0;
		size_t m_num_commands = 0;
	};
}