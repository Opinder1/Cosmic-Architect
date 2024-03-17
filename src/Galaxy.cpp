#include "UniverseSimulation.h"
#include "Signals.h"

#include "Components.h"

#include <godot_cpp/classes/dir_access.hpp>

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetGalaxyInfo()
	{
		return {};
	}

	void UniverseSimulation::StartLocalGalaxy(const godot::String& galaxy_path)
	{
		m_world.reset();
		m_path = galaxy_path;
		m_directory = godot::DirAccess::open(m_path);
		m_galaxy_load_state = LoadState::Loading;
		emit_signal(k_signals->galaxy_load_state_changed, m_galaxy_load_state);

	}

	void UniverseSimulation::StartLocalFragment(const godot::String& fragment_path, const godot::String& fragment_type)
	{
		m_world.reset();
		m_path = fragment_path;
		m_fragment_type = fragment_type;
	}

	void UniverseSimulation::StartRemoteGalaxy(const godot::String& galaxy_path)
	{
		m_world.reset();
		m_path = galaxy_path;
	}

	void UniverseSimulation::StopGalaxy()
	{
		m_world.reset();
		m_path = godot::String();
	}

	UniverseSimulation::LoadState UniverseSimulation::GetGalaxyLoadState()
	{
		return m_galaxy_load_state;
	}
}