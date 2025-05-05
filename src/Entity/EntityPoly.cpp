#include "EntityPoly.h"

#include "EntityComponents.h"
#include "Render/RenderComponents.h"
#include "Physics3D/PhysicsComponents.h"
#include "Networking/NetworkingComponents.h"
#include "Loading/LoadingComponents.h"
#include "Player/PlayerComponents.h"
#include "Spatial3D/SpatialComponents.h"
#include "Universe/UniverseComponents.h"
#include "Galaxy/GalaxyComponents.h"
#include "GalaxyRender/GalaxyRenderComponents.h"
#include "Voxel/VoxelComponents.h"
#include "VoxelRender/VoxelRenderComponents.h"

#include "UniverseSimulation.h"

using namespace voxel_game;

const size_t first = __LINE__ + 1;
template<> template<> const size_t entity::Type::k_type_index<entity::Type::Header> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<entity::CName> =				__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<entity::CParent> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<rendering::CContext> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<rendering::CTransform> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<rendering::CScenario> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<rendering::CInstance> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<rendering::CInstancer> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<rendering::CBase> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CPosition> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CVelocity> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CRotation> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CScale> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CMass> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CBounciness> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CRoughness> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CGravity> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CAABB> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CBox> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CSphere> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<network::CCertificate> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<network::CServer> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<network::CPeer> =				__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<loading::CStreamable> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<loading::CAutosave> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<player::EntityPtr> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<player::AvatarPath> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<player::CPlayer> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<player::CPlayers> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<player::CPlayerAvatar> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<spatial3d::CLoader> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<spatial3d::CRegion> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<spatial3d::CEntity> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<spatial3d::CWorld> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<universe::CUniverse> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<galaxy::CGalaxy> =			__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<galaxy::CStar> =				__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<galaxyrender::CWorld> =		__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<voxel::CWorld> =				__LINE__ - first;
template<> template<> const size_t entity::Type::k_type_index<voxelrender::CWorld> =		__LINE__ - first;

const std::array<PolyTypeInfo, 128> PolyType<entity::Type, 128>::k_type_info =
{
	MakeTypeInfo<entity::Type::Header>(),
	MakeTypeInfo<entity::CName>(),
	MakeTypeInfo<entity::CParent>(),
	MakeTypeInfo<rendering::CContext>(),
	MakeTypeInfo<rendering::CTransform>(),
	MakeTypeInfo<rendering::CScenario>(),
	MakeTypeInfo<rendering::CInstance>(),
	MakeTypeInfo<rendering::CInstancer>(),
	MakeTypeInfo<rendering::CBase>(),
	MakeTypeInfo<physics3d::CPosition>(),
	MakeTypeInfo<physics3d::CVelocity>(),
	MakeTypeInfo<physics3d::CRotation>(),
	MakeTypeInfo<physics3d::CScale>(),
	MakeTypeInfo<physics3d::CMass>(),
	MakeTypeInfo<physics3d::CBounciness>(),
	MakeTypeInfo<physics3d::CRoughness>(),
	MakeTypeInfo<physics3d::CGravity>(),
	MakeTypeInfo<physics3d::CAABB>(),
	MakeTypeInfo<physics3d::CBox>(),
	MakeTypeInfo<physics3d::CSphere>(),
	MakeTypeInfo<network::CCertificate>(),
	MakeTypeInfo<network::CServer>(),
	MakeTypeInfo<network::CPeer>(),
	MakeTypeInfo<loading::CStreamable>(),
	MakeTypeInfo<loading::CAutosave>(),
	MakeTypeInfo<player::EntityPtr>(),
	MakeTypeInfo<player::AvatarPath>(),
	MakeTypeInfo<player::CPlayer>(),
	MakeTypeInfo<player::CPlayers>(),
	MakeTypeInfo<player::CPlayerAvatar>(),
	MakeTypeInfo<spatial3d::CLoader>(),
	MakeTypeInfo<spatial3d::CRegion>(),
	MakeTypeInfo<spatial3d::CEntity>(),
	MakeTypeInfo<spatial3d::CWorld>(),
	MakeTypeInfo<universe::CUniverse>(),
	MakeTypeInfo<galaxy::CGalaxy>(),
	MakeTypeInfo<galaxy::CStar>(),
	MakeTypeInfo<galaxyrender::CWorld>(),
	MakeTypeInfo<voxel::CWorld>(),
	MakeTypeInfo<voxelrender::CWorld>(),
};

namespace voxel_game::entity
{
	Type::Type() : PolyType<Type, 128>() {}

	void Type::InitType(PolyArchetypeRegistry<Type>& factory, ID type_id)
	{
		for (size_t i = 1; i < type_id.size(); i++)
		{
			if (type_id.test(i))
			{
				AddType(i);
			}
		}

		for (auto&& [types, entries] : static_cast<Factory&>(factory).m_callbacks)
		{
			if ((type_id & types) == types)
			{
				for (const Factory::CallbackEntry& entry : entries)
				{
					AddCallback(entry.event, entry.callback);
				}
			}
		}
	}

	void Type::AddCallback(Event event, EventCallback callback)
	{
		m_type_callbacks[to_underlying(event)].push_back(callback);
	}
	
	void Type::DoEvent(Simulation& simulation, Ptr poly, Event event) const
	{
		for (const EventCallback& callback : m_type_callbacks[to_underlying(event)])
		{
			callback(simulation, poly);
		}
	}

	Factory::Factory() {}

	void Factory::AddCallback(TypeID types, Event event, EventCallback callback)
	{
		// Add to future archetypes
		m_callbacks[types].push_back({ event, callback });

		// Add to existing archetypes
		TypeIterate(types, [event, &callback](Type& type)
		{
			type.AddCallback(event, callback);
		});
	}

	void Factory::DoEvent(Simulation& simulation, WeakRef poly, Event event) const
	{
		poly.GetType()->DoEvent(simulation, poly.GetPtr(), event);
	}
}

size_t std::hash<voxel_game::entity::WRef>::operator()(const voxel_game::entity::WRef& wref) const noexcept
{
	return wref.Hash();
}

size_t std::hash<voxel_game::entity::Ref>::operator()(const voxel_game::entity::Ref& ref) const noexcept
{
	return ref.Hash();
}