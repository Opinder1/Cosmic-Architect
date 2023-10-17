#include "NetworkMessager.h"

#include "Simulation/SimulationServer.h"

#include "NetworkServerSystem.h"

#include "NetworkPeerSystem.h"

namespace sim
{
	UUID NetworkMessager::m_simulation_id;

	void NetworkMessager::Initialize()
	{
		m_simulation_id = SimulationServer::GetSingleton()->CreateSimulation(60);

		SimulationServer::GetSingleton()->AddSystem<NetworkServerSystem>(m_simulation_id);
		SimulationServer::GetSingleton()->AddSystem<NetworkPeerSystem>(m_simulation_id);

		SimulationServer::GetSingleton()->StartSimulation(m_simulation_id);
	}

	void NetworkMessager::Uninitialize()
	{
		SimulationServer::GetSingleton()->StopAndDeleteSimulation(m_simulation_id);
	}

	UUID NetworkMessager::GetSimulation()
	{
		return m_simulation_id;
	}
}