#include "Debug.h"

#include <robin_hood/robin_hood.h>

#include <TKRZW/tkrzw_thread_util.h>

#include <thread>

#if DEBUG
namespace
{
	tkrzw::SpinSharedMutex objects_mutex;
	robin_hood::unordered_node_map<void*, std::thread::id> object_threads;
}

void DebugWriteCheckerWrite(void* object)
{
	{
		std::shared_lock read_lock(objects_mutex);

		auto it = object_threads.find(object);

		if (it != object_threads.end())
		{
			DEBUG_ASSERT(it->second == std::this_thread::get_id(), "Only one thread should write to this between each sync");
			return;
		}
	}

	{
		std::unique_lock write_lock(objects_mutex);

		object_threads.emplace(object, std::this_thread::get_id()).first;
	}
}

void DebugWriteCheckerSync()
{
	std::unique_lock write_lock(objects_mutex);

	object_threads.clear();
}
#endif