#include "UniverseSimulation.h"
#include "Components.h"

#include "Util/Debug.h"

#include <godot_cpp/core/class_db.hpp>

namespace voxel_world
{
	UniverseSimulation::UniverseSimulation()
	{}

	UniverseSimulation::~UniverseSimulation()
	{}

	void UniverseSimulation::StartLocalGalaxy(const godot::String& galaxy_path)
	{
		m_world.reset();
		m_path = galaxy_path;
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

	void UniverseSimulation::CreateAccount(const godot::String& username, const godot::String& password_hash)
	{

	}

	void UniverseSimulation::LoginAccount(const godot::String& username, const godot::String& password_hash)
	{

	}

	void UniverseSimulation::LogoutAccount()
	{

	}

	uint64_t UniverseSimulation::CreateInstance(godot::RID mesh)
	{
		auto entity = m_world.entity();

		entity.emplace<Instance>(mesh);
		entity.add<Position>();

		return entity.id();
	}

	void UniverseSimulation::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("start_local_galaxy", "galaxy_path"), &UniverseSimulation::StartLocalGalaxy);
		godot::ClassDB::bind_method(godot::D_METHOD("start_local_fragment", "fragment_path", "fragment_type"), &UniverseSimulation::StartLocalFragment);
		godot::ClassDB::bind_method(godot::D_METHOD("start_remote_galaxy", "galaxy_path"), &UniverseSimulation::StartRemoteGalaxy);
		godot::ClassDB::bind_method(godot::D_METHOD("stop_galaxy"), &UniverseSimulation::StopGalaxy);

		godot::ClassDB::bind_method(godot::D_METHOD("create_account"), &UniverseSimulation::CreateAccount);
		godot::ClassDB::bind_method(godot::D_METHOD("login_account"), &UniverseSimulation::LoginAccount);
		godot::ClassDB::bind_method(godot::D_METHOD("logout_account"), &UniverseSimulation::LogoutAccount);

		godot::ClassDB::bind_method(godot::D_METHOD("create_instance", "mesh"), &UniverseSimulation::CreateInstance);
	}
}