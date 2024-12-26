#include "UniverseCache.h"

#include "Util/Debug.h"

#include <mutex>

namespace voxel_game
{
	UniverseCache::Info UniverseCache::* UniverseCache::GetInfo(Type type)
	{
		switch (type)
		{
		case Type::Galaxy:			return &UniverseCache::galaxy_info;
		case Type::Account:			return &UniverseCache::account_info;
		case Type::Player:			return &UniverseCache::player_info;
		default:					return nullptr;
		}
	}

	UniverseCache::InfoMap UniverseCache::* UniverseCache::GetInfoMap(Type type)
	{
		switch (type)
		{
		case Type::Player:			return &UniverseCache::player_info_map;
		case Type::Fragment:		return &UniverseCache::fragment_info_map;
		case Type::ChatChannel:		return &UniverseCache::chat_channel_info_map;
		case Type::Party:			return &UniverseCache::party_info_map;
		case Type::Entity:			return &UniverseCache::entity_info_map;
		case Type::Volume:			return &UniverseCache::volume_info_map;
		case Type::GalaxyRegion:	return &UniverseCache::galaxy_region_info_map;
		case Type::GalaxyObject:	return &UniverseCache::galaxy_object_info_map;
		case Type::Currency:		return &UniverseCache::currency_info_map;
		case Type::Bank:			return &UniverseCache::bank_info_map;
		case Type::BankInterface:	return &UniverseCache::bank_interface_info_map;
		case Type::Good:			return &UniverseCache::good_info_map;
		case Type::Internet:		return &UniverseCache::internet_info_map;
		case Type::Website:			return &UniverseCache::website_info_map;
		case Type::Webpage:			return &UniverseCache::webpage_info_map;
		case Type::Role:			return &UniverseCache::role_info_map;
		case Type::Permission:		return &UniverseCache::permission_info_map;
		case Type::Faction:			return &UniverseCache::faction_info_map;
		case Type::Language:		return &UniverseCache::language_info_map;
		case Type::Culture:			return &UniverseCache::culture_info_map;
		case Type::Inventory:		return &UniverseCache::inventory_info_map;
		case Type::Ability:			return &UniverseCache::ability_info_map;
		case Type::Spell:			return &UniverseCache::spell_info_map;
		default:					return nullptr;
		}
	}

	UniverseCacheUpdater::UniverseCacheUpdater() {}

#if defined(DEBUG_ENABLED)
	void UniverseCacheUpdater::SetThreads(std::thread::id reader_id, std::thread::id writer_id)
	{
		m_updates.SetThreads(reader_id, writer_id);
	}
#endif

	void UniverseCacheUpdater::UpdateInfo(UniverseCache::Type type, const UniverseCache::Info& info)
	{
		InfoUpdate update;

		update.info = UniverseCache::GetInfo(type);
		update.key = UUID();
		update.value = info;

		if (update.info == nullptr)
		{
			DEBUG_PRINT_ERROR("Invalid universe info type");
			return;
		}

		AddInfoUpdate(std::move(update));
	}

	void UniverseCacheUpdater::UpdateInfoMap(UniverseCache::Type type, UUID id, const UniverseCache::Info& info)
	{
		InfoUpdate update;

		update.info_map = UniverseCache::GetInfoMap(type);
		update.key = id;
		update.value = info;

		if (update.info_map == nullptr)
		{
			DEBUG_PRINT_ERROR("Invalid universe info map type");
			return;
		}

		AddInfoUpdate(std::move(update));
	}

	void UniverseCacheUpdater::AddInfoUpdate(InfoUpdate&& update)
	{
		m_updates.AddCommand(std::move(update));
	}

	void UniverseCacheUpdater::PublishUpdates()
	{
		m_updates.PublishCommands();
	}

	void UniverseCacheUpdater::RetrieveUpdates(UniverseCache& out)
	{
		std::vector<InfoUpdate> updates;

		m_updates.RetrieveCommands(updates);

		std::lock_guard lock(out.mutex);

		for (const InfoUpdate& update : updates)
		{
			if (update.key == UUID())
			{
				(out.*update.info) = update.value;
			}
			else
			{
				(out.*update.info_map)[update.key] = update.value;
			}
		}
	}
}