#include "UniverseSimulation.h"

#include "Util/Debug.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetFragmentInfo(UUID fragment_id)
	{
		return {};
	}

	UniverseSimulation::UUID UniverseSimulation::GetCurrentFragment()
	{
		return {};
	}

	void UniverseSimulation::EnterFragment(UUID fragment_id, const godot::Dictionary& method)
	{
		SIM_DEFER_COMMAND(k_commands->enter_fragment, fragment_id, method);
	}
}