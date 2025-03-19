#pragma once

#include "Util/GodotUUID.h"
#include "Util/PerThread.h"
#include "Util/Debug.h"
#include "Util/GodotHash.h"

#include <godot_cpp/variant/dictionary.hpp>

#include <robin_hood/robin_hood.h>

#include <TKRZW/tkrzw_thread_util.h>

#include <vector>
#include <atomic>

namespace voxel_game
{
	// The cache for a UniverseSimulation that stores all cached objects to be read in godot code
	struct UniverseCache
	{
		using Info = godot::Dictionary;
		using InfoMap = robin_hood::unordered_flat_map<UUID, Info, UUIDHash>;

		enum class Type
		{
			Universe,
			Galaxy,
			Account,
			Player,
			Fragment,
			ChatChannel,
			Party,
			Entity,
			Volume,
			GalaxyRegion,
			GalaxyObject,
			Currency,
			Bank,
			BankInterface,
			Good,
			Internet,
			Website,
			Webpage,
			Role,
			Permission,
			Faction,
			Language,
			Culture,
			Inventory,
			Ability,
			Spell
		};

		tkrzw::SpinSharedMutex mutex;

		Info universe_info;
		Info galaxy_info;
		Info account_info;
		Info player_info;

		InfoMap player_info_map;
		InfoMap fragment_info_map;
		InfoMap chat_channel_info_map;
		InfoMap party_info_map;
		InfoMap entity_info_map;
		InfoMap volume_info_map;
		InfoMap galaxy_region_info_map;
		InfoMap galaxy_object_info_map;
		InfoMap currency_info_map;
		InfoMap bank_info_map;
		InfoMap bank_interface_info_map;
		InfoMap good_info_map;
		InfoMap internet_info_map;
		InfoMap website_info_map;
		InfoMap webpage_info_map;
		InfoMap role_info_map;
		InfoMap permission_info_map;
		InfoMap faction_info_map;
		InfoMap language_info_map;
		InfoMap culture_info_map;
		InfoMap inventory_info_map;
		InfoMap ability_info_map;
		InfoMap spell_info_map;

		static Info UniverseCache::* GetInfo(Type type);

		static InfoMap UniverseCache::* GetInfoMap(Type type);
	};

	// A cache updater that queues cache updates and executes them in a thread efficient way
	// 
	// This class is thread safe but will complain if the wrong thread calls certain methods
	class UniverseCacheUpdater
	{
		struct InfoUpdate
		{
			union
			{
				UniverseCache::Info UniverseCache::* info = nullptr;
				UniverseCache::InfoMap UniverseCache::* info_map;
			};
			UUID key;
			UniverseCache::Info value;
		};

	public:
		UniverseCacheUpdater();

#if defined(DEBUG_ENABLED)
		void SetWriterThread(std::thread::id writer_id);
#endif

		// Update a info entry of a singleton type
		void UpdateInfo(UniverseCache::Type type, const UniverseCache::Info& info);

		// Update a info entry of a map type
		void UpdateInfoMap(UniverseCache::Type type, UUID id, const UniverseCache::Info& info);

		// Write the changes to the exchange buffer
		void PublishUpdates();

		// Obtain the latest changes made by the writer if there are any
		void RetrieveUpdates(UniverseCache& cache);

	private:
		void AddInfoUpdate(InfoUpdate&& update);

	private:
		alignas(k_cache_line) std::vector<InfoUpdate> m_updates_write;
		alignas(k_cache_line) SwapBuffer<std::vector<InfoUpdate>> m_updates_swap;

#if defined(DEBUG_ENABLED)
		std::thread::id m_writer_id; // The thread that does updates and calls Write() and Publish() on it
#endif
	};
}