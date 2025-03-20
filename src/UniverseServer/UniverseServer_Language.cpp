#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetLanguageInfo(const ID& language_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		
		auto it = m_info_cache.language_info_map.find(language_id);

		if (it != m_info_cache.language_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	void UniverseServer::GetLanguageTranslation(const ID& language_id, const godot::StringName& string)
	{
		if (DeferCommand<&UniverseServer::GetLanguageTranslation>(language_id, string))
		{
			return;
		}
	}

	void UniverseServer::GetLanguageString(const ID& language_id, uint64_t string_id)
	{
		if (DeferCommand<&UniverseServer::GetLanguageString>(language_id, string_id))
		{
			return;
		}
	}
}