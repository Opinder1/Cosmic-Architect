#include "Debug.h"
#include "Nocopy.h"
#include "SmallVector.h"
#include "Util.h"

#include <robin_hood/robin_hood.h>

#include <TKRZW/tkrzw_thread_util.h>

#include <thread>

#if defined(DEBUG_THREAD_CHECK)
namespace
{
	struct ThreadObject
	{
		std::thread::id write;
		size_t write_recursion = 0;
		SmallVector<std::thread::id, 16> read;
	};

	struct ThreadObjectMap
	{
		tkrzw::SpinSharedMutex mutex;
		robin_hood::unordered_map<const void*, ThreadObject> objects;
	}
	thread_object_map;
}

DebugThreadChecker::DebugThreadChecker(const void* object, bool write) :
	m_object(object),
	m_write(write)
{
	std::unique_lock write_lock(thread_object_map.mutex);

	ThreadObject& object_data = thread_object_map.objects[object];

	std::thread::id thread_id = std::this_thread::get_id();

	if (write)
	{
		if (!(object_data.read.size() == 1 && object_data.read.back() == thread_id)) // If we are the only one reading then we can write
		{
			DEBUG_ASSERT(object_data.read.size() == 0, "No other thread should be reading if we want to write");
		}

		if (object_data.write_recursion++ == 0)
		{
			object_data.write = thread_id;
		}
	}
	else
	{
		if (object_data.write == thread_id) // If we are already writing then just continue writing
		{
			object_data.write_recursion++;
			m_write = true;
		}
		else
		{
			DEBUG_ASSERT(object_data.write == std::thread::id{}, "No other thread should be writing if we want to read");

			object_data.read.push_back(thread_id);
		}
	}
}

DebugThreadChecker::~DebugThreadChecker()
{
	std::unique_lock write_lock(thread_object_map.mutex);

	auto it = thread_object_map.objects.find(m_object);

	DEBUG_ASSERT(it != thread_object_map.objects.end(), "The object should exist in the map");

	ThreadObject& object_data = it->second;

	if (m_write)
	{
		if (object_data.write_recursion-- == 0)
		{
			object_data.write = std::thread::id{};
		}
	}
	else
	{
		unordered_erase(object_data.read, std::this_thread::get_id());
	}

	if (object_data.read.is_empty() && object_data.write == std::thread::id{})
	{
		thread_object_map.objects.erase(it);
	}
}

#endif // DEBUG_THREAD_CHECK