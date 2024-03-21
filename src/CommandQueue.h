#pragma once

#include <godot_cpp/classes/ref_counted.hpp>

#include <godot_cpp/templates/local_vector.hpp>

#include <deque>

namespace voxel_game
{
	class CommandQueue : public godot::RefCounted
	{
		GDCLASS(CommandQueue, godot::RefCounted);

	public:
		CommandQueue();
		~CommandQueue();

		void SetObject(uint64_t object_id);

		uint64_t GetObject();

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
		static void _cleanup_methods();

	private:
		void _register_command_vararg(const godot::Variant** p_args, GDExtensionInt p_argcount, GDExtensionCallError& error);
		void _register_command(const godot::StringName& command, const godot::Variant** args, size_t argcount);

	private:
		godot::ObjectID m_object_id;

		std::vector<uint8_t> m_command_buffer;
		std::vector<uint32_t> m_command_offsets;
	};
}