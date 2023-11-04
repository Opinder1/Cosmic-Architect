#pragma once

#include "Simulation/System.h"

namespace sim
{
	class ByteStream;

	struct TickEvent;

	class NetworkPeerSystem : public System
	{
	public:
		NetworkPeerSystem(Simulation& simulation);
		~NetworkPeerSystem();

	private:
		void OnTick(const TickEvent& event);

	private:
		bool LoadMessage(ByteStream& stream);
	};
}