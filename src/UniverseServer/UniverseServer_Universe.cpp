#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

#include "Universe/UniverseModule.h"
#include "Galaxy/GalaxyModule.h"

#include "Player/PlayerModule.h"
#include "Galaxy/GalaxyModule.h"
#include "Entity/EntityModule.h"
#include "Spatial3D/SpatialModule.h"

#include "Components.h"
#include "UniverseSimulation.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetUniverseInfo()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.universe_info;
	}

	void UniverseServer::ConnectToUniverseList(const godot::String& ip)
	{
		if (DeferCommand<&UniverseServer::ConnectToUniverseList>(ip))
		{
			return;
		}
	}

	void UniverseServer::DisconnectFromUniverseList()
	{
		if (DeferCommand<&UniverseServer::DisconnectFromUniverseList>())
		{
			return;
		}
	}

	void UniverseServer::QueryUniverseList(const godot::Dictionary& query)
	{
		if (DeferCommand<&UniverseServer::QueryUniverseList>(query))
		{
			return;
		}
	}

	void UniverseServer::PingRemoteUniverse(const godot::String& ip)
	{
		if (DeferCommand<&UniverseServer::PingRemoteUniverse>(ip))
		{
			return;
		}
	}

	void UniverseServer::StartLocalUniverse(const godot::String& path, const godot::String& fragment_type, ServerType server_type, godot::RID scenario)
	{
		if (DeferCommand<&UniverseServer::StartLocalUniverse>(path, fragment_type, server_type, scenario))
		{
			return;
		}

		SimulationSetPath(*m_simulation, path);

		m_universe_entity = universe::CreateUniverse(*m_simulation, UUID{ 0, 0 });

		m_universe_entity->*&CScenario::id = scenario;

		m_galaxy_entity = galaxy::CreateSimulatedGalaxy(*m_simulation, UUID{ 0, 0 }, spatial3d::GetEntityWorld(m_universe_entity));

		m_player_entity = player::CreateLocalPlayer(*m_simulation, m_universe_entity, "localuser");

		QueueSignal(k_signals->connected_to_universe);
	}

	void UniverseServer::ConnectToUniverse(const godot::String& path, const godot::String& ip, godot::RID scenario)
	{
		if (DeferCommand<&UniverseServer::ConnectToUniverse>(path, ip, scenario))
		{
			return;
		}
	}

	void UniverseServer::DisconnectFromUniverse()
	{
		if (DeferCommand<&UniverseServer::DisconnectFromUniverse>())
		{
			return;
		}

		SimulationUnloadEntity(*m_simulation, m_universe_entity);

		QueueSignal(k_signals->disconnected_from_universe);
	}
}