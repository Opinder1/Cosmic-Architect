#include "Debug.h"
#include "Nocopy.h"

#include <robin_hood/robin_hood.h>

#include <TKRZW/tkrzw_thread_util.h>

#include <flecs/flecs.h>

#include <thread>

#if DEBUG
namespace
{
	struct CheckGroup : Nocopy
	{
		CheckGroup() noexcept {}

		CheckGroup(CheckGroup&& other) noexcept
		{
			std::shared_lock read_lock(other.mutex);

			objects = std::move(other.objects);
		}

		tkrzw::SpinSharedMutex mutex;
		robin_hood::unordered_map<const void*, std::thread::id> objects;
	};

	struct ThreadObjectMap
	{
		tkrzw::SpinSharedMutex mutex;
		robin_hood::unordered_map<const void*, CheckGroup> groups;
	}
	thread_object_map;

	CheckGroup& GetCheckGroup(const void* key)
	{
		{
			std::shared_lock read_lock(thread_object_map.mutex);

			auto it = thread_object_map.groups.find(key);

			if (it != thread_object_map.groups.end())
			{
				return it->second;
			}
		}

		{
			std::unique_lock write_lock(thread_object_map.mutex);

			auto&& [it, emplaced] = thread_object_map.groups.emplace(key, CheckGroup{});

			DEBUG_ASSERT(emplaced, "We just checked that this key doesn't exist so this shouldn't happen");

			return it->second;
		}
	}

	bool CheckGroupObject(CheckGroup& group, const void* object)
	{
		std::shared_lock read_lock(group.mutex);

		auto it = group.objects.find(object);

		if (it != group.objects.end())
		{
			DEBUG_ASSERT(it->second == std::this_thread::get_id(), "Only one thread should write to this between each sync");
			return true;
		}

		return false;
	}
}

void DebugThreadCheckRead(const void* group, const void* object)
{
	CheckGroup& check_group = GetCheckGroup(group);

	CheckGroupObject(check_group, object);
}

void DebugThreadCheckWrite(const void* group, const void* object)
{
	CheckGroup& check_group = GetCheckGroup(group);

	if (!CheckGroupObject(check_group, object))
	{
		std::unique_lock write_lock(check_group.mutex);

		check_group.objects.emplace(object, std::this_thread::get_id()).first;
	}
}

void DebugThreadCheckSync(const void* group)
{
	CheckGroup& check_group = GetCheckGroup(group);

	std::unique_lock write_lock(check_group.mutex);

	check_group.objects.clear();
}
#endif // DEBUG