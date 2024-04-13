#include "UniverseSimulation.h"
#include "CommandQueue.h"

#include "Util/Debug.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetFragmentInfo(UUID fragment_id)
	{
		std::shared_lock lock(m_mutex);
		return {};
	}

	UniverseSimulation::UUID UniverseSimulation::GetCurrentFragment()
	{
		std::shared_lock lock(m_mutex);
		return {};
	}

	void UniverseSimulation::EnterFragment(UUID fragment_id, const godot::Dictionary& method)
	{
		if (m_thread.joinable() && std::this_thread::get_id() != m_thread.get_id())
		{
			m_command_queue->RegisterCommand("enter_fragment", fragment_id, method);
			return;
		}
	}
}