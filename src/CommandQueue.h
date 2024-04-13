#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/ref.hpp>

#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <TKRZW/tkrzw_thread_util.h>

#include <vector>
#include <memory>

namespace voxel_game
{
	using CommandBuffer = std::vector<uint8_t>;

	class CommandQueue : public godot::RefCounted
	{
		GDCLASS(CommandQueue, godot::RefCounted);

		struct Command
		{
			constexpr static size_t k_max_args = 16;

			godot::StringName command;
			size_t argcount;
		};

	public:
		static godot::Ref<CommandQueue> MakeQueue(const godot::Variant& object);

		CommandQueue();
		~CommandQueue();

		uint64_t GetObject();

		uint64_t GetOwningThread();

		template<class... Args>
		void RegisterCommand(const godot::StringName& command, Args... p_args)
		{
			godot::Variant args[sizeof...(p_args) + 1] = { p_args..., godot::Variant() }; // +1 makes sure zero sized arrays are also supported.
			const godot::Variant* argptrs[sizeof...(p_args) + 1];
			for (uint32_t i = 0; i < sizeof...(p_args); i++) {
				argptrs[i] = &args[i];
			}
			return _register_command(command, sizeof...(p_args) == 0 ? nullptr : (const godot::Variant**)argptrs, sizeof...(p_args));
		}

		void Flush();

		void PopCommandBuffer(CommandBuffer& command_buffer_out);

		static void ProcessCommands(uint64_t object_id, const CommandBuffer& command_buffer);

	public:
		static void _bind_methods();

	private:
		void _register_command_vararg(const godot::Variant** p_args, GDExtensionInt p_argcount, GDExtensionCallError& error);
		void _register_command(const godot::StringName& command, const godot::Variant** args, size_t argcount);

	private:
		uint64_t m_thread_id = 0;
		uint64_t m_object_id = 0;
		CommandBuffer m_command_buffer;
	};

	class CommandQueueServer : public godot::Object
	{
		GDCLASS(CommandQueueServer, godot::Object);

		struct Commands
		{
			uint64_t object_id;
			CommandBuffer command_buffer;
		};

	public:
		static CommandQueueServer* get_singleton();

		CommandQueueServer();
		~CommandQueueServer();

		void AddCommands(uint64_t object_id, CommandBuffer&& command_buffer);

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

		tkrzw::SpinSharedMutex m_mutex;
		std::vector<Commands> m_command_buffers;

		tkrzw::SpinSharedMutex m_rendering_mutex;
		std::vector<Commands> m_rendering_command_buffers;
	};
}