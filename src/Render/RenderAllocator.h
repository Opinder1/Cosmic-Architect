#pragma once

#include "Util/GodotMemory.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/rid.hpp>

#include <TKRZW/tkrzw_thread_util.h>

#include <optional>

namespace voxel_game::rendering
{
	// A server that keeps some pools of render objects preallocated for any allocators to take
	// Any taken render objects will automatically be refilled
	class AllocatorServer : public godot::Object
	{
		GDCLASS(AllocatorServer, godot::Object);

	public:
		static AllocatorServer* get_singleton();

		AllocatorServer();
		~AllocatorServer();

		void Process(std::vector<godot::RID>& read_instances);

	public:
		static void _bind_methods();
		static void _cleanup_methods();

	private:
		void AllocateRIDs();

	private:
		static godot::OptObj<AllocatorServer> k_singleton;

		tkrzw::SpinMutex m_mutex; // Mutex to protect the write lists

		std::vector<godot::RID> m_write_instances;
	};

	// An allocator that will get instances from the server so instances can be created without
	// any locking of mutexes or even atomics
	class Allocator
	{
	public:
		Allocator();
		~Allocator();

		void Process();

		godot::RID CreateInstance();

	private:
		std::vector<godot::RID> m_read_instances;
	};
}