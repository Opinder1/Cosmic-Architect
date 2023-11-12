#pragma once

#include <robin_hood/robin_hood.h>

#include <entt/fwd.hpp>

namespace sim
{
	struct ServersGlobal
	{
		robin_hood::unordered_flat_map<uint16_t, entt::entity> server_entities; // Map for ports to servers
	};
}