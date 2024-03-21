#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/ref.hpp>

#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <godot_cpp/templates/local_vector.hpp>

#include <memory>
#include <deque>
#include <thread>
#include <mutex>

namespace voxel_game
{
	struct Command
	{
		constexpr static size_t k_max_args = 16;

		godot::StringName command;
		size_t argcount;
	};

	class CommandQueue : public godot::RefCounted
	{
		GDCLASS(CommandQueue, godot::RefCounted);

		friend class CommandQueueProcessor;

	public:
		CommandQueue();
		~CommandQueue();

		static godot::Ref<CommandQueue> MakeQueue(const godot::Variant& object);

		template<class... Args>
		void RegisterCommand(const godot::StringName& command, Args... args)
		{
			godot::Variant args[sizeof...(args) + 1] = { args..., godot::Variant() }; // +1 makes sure zero sized arrays are also supported.
			const godot::Variant* argptrs[sizeof...(args) + 1];
			for (uint32_t i = 0; i < sizeof...(args); i++) {
				argptrs[i] = &args[i];
			}
			return _register_command(command, sizeof...(args) == 0 ? nullptr : (const godot::Variant**)argptrs, sizeof...(args));
		}

		void Flush();

	public:
		static void _bind_methods();

	private:
		void _register_command_vararg(const godot::Variant** p_args, GDExtensionInt p_argcount, GDExtensionCallError& error);
		void _register_command(const godot::StringName& command, const godot::Variant** args, size_t argcount);

	private:
		std::thread::id m_thread_id;

		godot::ObjectID m_object_id;

		std::vector<uint8_t> m_command_buffer;
		std::vector<uint32_t> m_command_offsets;
	};

	class CommandQueueProcessor : public godot::Object
	{
		GDCLASS(CommandQueueProcessor, godot::Object);

		struct Commands
		{
			godot::ObjectID object_id;

			std::vector<uint8_t> command_buffer;
			std::vector<uint32_t> command_offsets;
		};

	public:
		static CommandQueueProcessor* get_singleton();

		CommandQueueProcessor();
		~CommandQueueProcessor();

		void AddCommands(CommandQueue& command_queue);

		void ProcessCommands(const Commands& commands);

		void Flush();

	public:
		static void _bind_methods();
		static void _cleanup_methods();

	private:
		static std::unique_ptr<CommandQueueProcessor> k_singleton;

		std::mutex m_mutex;
		std::vector<Commands> m_command_buffers;
	};
}