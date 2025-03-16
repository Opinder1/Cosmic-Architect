#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

#include "Render/RenderComponents.h"

#include "Galaxy/GalaxyModule.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetUniverseInfo()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.universe_info;
	}

	void UniverseServer::ConnectToGalaxyList(const godot::String& ip)
	{
		if (DeferCommand<&UniverseServer::ConnectToGalaxyList>(ip))
		{
			return;
		}
	}

	void UniverseServer::DisconnectFromGalaxyList()
	{
		if (DeferCommand<&UniverseServer::DisconnectFromGalaxyList>())
		{
			return;
		}
	}

	void UniverseServer::QueryGalaxyList(const godot::Dictionary& query)
	{
		if (DeferCommand<&UniverseServer::QueryGalaxyList>(query))
		{
			return;
		}
	}

	void UniverseServer::PingRemoteGalaxy(const godot::String& ip)
	{
		if (DeferCommand<&UniverseServer::PingRemoteGalaxy>(ip))
		{
			return;
		}
	}

	void UniverseServer::StartLocalGalaxy(const godot::String& path, const godot::String& fragment_type, ServerType server_type, godot::RID scenario)
	{
		if (DeferCommand<&UniverseServer::StartLocalGalaxy>(path, fragment_type, server_type, scenario))
		{
			return;
		}

		flecs::entity(m_world, m_universe_entity).emplace<rendering::CScenario>(scenario);

		m_galaxy_entity = galaxy::CreateNewSimulatedGalaxy(m_world, path, m_universe_entity);

		// Implement player module
		// m_player_entity = player::CreateNewEntity(m_world, m_galaxy_entity, "localuser");
		m_player_entity = ecs_new(m_world);

		flecs::entity_t world_entity = ecs_get_parent(m_world, m_player_entity);

		while (ecs_is_valid(m_world, world_entity) && world_entity != m_galaxy_entity)
		{
			m_world_entities.push_front(world_entity);
			world_entity = ecs_get_parent(m_world, world_entity);
		}

		QueueSignal(k_signals->connected_to_galaxy);
	}

	void UniverseServer::ConnectToGalaxy(const godot::String& path, const godot::String& ip, godot::RID scenario)
	{
		if (DeferCommand<&UniverseServer::ConnectToGalaxy>(path, ip, scenario))
		{
			return;
		}
	}

	void UniverseServer::DisconnectFromGalaxy()
	{
		if (DeferCommand<&UniverseServer::DisconnectFromGalaxy>())
		{
			return;
		}

		ecs_delete(m_world, m_galaxy_entity);

		QueueSignal(k_signals->disconnected_from_galaxy);
	}
}