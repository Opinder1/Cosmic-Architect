#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/ref.hpp>

#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <godot_cpp/templates/local_vector.hpp>

#include <memory>
#include <shared_mutex>

namespace voxel_game
{
	struct Command
	{
		constexpr static size_t k_max_args = 16;

		godot::StringName command;
		godot::Callable callback;
		size_t argcount;
	};

	class CommandQueue : public godot::RefCounted
	{
		GDCLASS(CommandQueue, godot::RefCounted);

	public:
		static godot::Ref<CommandQueue> MakeQueue(const godot::Variant& object);

		CommandQueue();
		~CommandQueue();

		uint64_t GetObject();

		uint64_t GetOwningThread();

		bool IsRenderingQueue();

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
		uint64_t m_thread_id;
		uint64_t m_object_id;
		bool m_rendering_queue;
		std::vector<uint8_t> m_command_buffer;
	};

	class CommandQueueServer : public godot::Object
	{
		GDCLASS(CommandQueueServer, godot::Object);

		struct Commands
		{
			uint64_t object_id;
			std::vector<uint8_t> command_buffer;
		};

	public:
		static CommandQueueServer* get_singleton();

		CommandQueueServer();
		~CommandQueueServer();

		void AddCommands(uint64_t object_id, std::vector<uint8_t>& command_buffer);
		void AddRenderingCommands(uint64_t object_id, std::vector<uint8_t>& command_buffer);

		void ProcessCommands(uint64_t object_id, const std::vector<uint8_t>& command_buffer);

		void Flush();

	public:
		static void _bind_methods();
		static void _cleanup_methods();

	private:
		bool HasCommands();
		bool HasRenderingCommands();

		void FlushCommands();
		void FlushRenderingCommands();

	private:
		static std::unique_ptr<CommandQueueServer> k_singleton;

		std::shared_mutex m_mutex;
		std::vector<Commands> m_command_buffers;

		std::shared_mutex m_rendering_mutex;
		std::vector<Commands> m_rendering_command_buffers;
	};
}