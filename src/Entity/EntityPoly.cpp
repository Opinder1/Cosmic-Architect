#include "EntityPoly.h"

#include "EntityComponents.h"
#include "Render/RenderComponents.h"
#include "Physics3D/PhysicsComponents.h"
#include "Networking/NetworkingComponents.h"
#include "Player/PlayerComponents.h"
#include "Spatial3D/SpatialComponents.h"
#include "Universe/UniverseComponents.h"
#include "Galaxy/GalaxyComponents.h"
#include "GalaxyRender/GalaxyRenderComponents.h"
#include "Voxel/VoxelComponents.h"
#include "VoxelRender/VoxelRenderComponents.h"

using namespace voxel_game;

using Archetype = entity::Factory::Archetype;

const size_t first = __LINE__ + 1;
template<> template<> const size_t Archetype::k_type_index<Archetype::Header> =				__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<entity::CName> =				__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<entity::CRelationship> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<rendering::CContext> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<rendering::CTransform> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<rendering::CScenario> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<rendering::CInstance> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<rendering::CInstancer> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<rendering::CBase> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<physics3d::CPosition> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<physics3d::CVelocity> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<physics3d::CRotation> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<physics3d::CScale> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<physics3d::CMass> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<physics3d::CBounciness> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<physics3d::CRoughness> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<physics3d::CGravity> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<physics3d::CAABB> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<physics3d::CBox> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<physics3d::CSphere> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<network::CCertificate> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<network::CServer> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<network::CPeer> =				__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<player::EntityPtr> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<player::AvatarPath> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<player::CPlayer> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<player::CPlayers> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<player::CPlayerAvatar> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<spatial3d::CLoader> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<spatial3d::CRegion> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<spatial3d::CEntity> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<spatial3d::CWorld> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<universe::CUniverse> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<galaxy::CGalaxy> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<galaxy::CStar> =				__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<galaxyrender::CWorld> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<voxel::CWorld> =				__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<voxelrender::CWorld> =		__LINE__ - first;

const std::array<PolyTypeInfo, 40> Archetype::k_type_info =
{
	MakeTypeInfo<Archetype::Header>(),
	MakeTypeInfo<entity::CName>(),
	MakeTypeInfo<entity::CRelationship>(),
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

size_t std::hash<voxel_game::entity::WRef>::operator()(const voxel_game::entity::WRef& wref) const noexcept
{
	return wref.Hash();
}

size_t std::hash<voxel_game::entity::Ref>::operator()(const voxel_game::entity::Ref& ref) const noexcept
{
	return ref.Hash();
}