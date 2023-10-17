#pragma once

#include "Simulation/System.h"

namespace sim
{
	class ByteStream;

	struct SimulationTickEvent;

	class NetworkPeerSystem : public System
	{
	public:
		NetworkPeerSystem(Simulation& simulation);
		~NetworkPeerSystem();

	private:
		void OnSimulationTick(const SimulationTickEvent& event);

	private:
		bool LoadMessage(ByteStream& stream);
	};
}