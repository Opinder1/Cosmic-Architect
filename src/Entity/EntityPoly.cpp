#include "EntityPoly.h"

#include "EntityComponents.h"
#include "Render/RenderComponents.h"
#include "Physics3D/PhysicsComponents.h"
#include "Simulation/SimulationComponents.h"
#include "Networking/NetworkingComponents.h"
#include "Loading/LoadingComponents.h"
#include "Player/PlayerComponents.h"
#include "Spatial3D/SpatialComponents.h"
#include "Universe/UniverseComponents.h"
#include "Galaxy/GalaxyComponents.h"
#include "GalaxyRender/GalaxyRenderComponents.h"
#include "Voxel/VoxelComponents.h"
#include "VoxelRender/VoxelRenderComponents.h"

using namespace voxel_game;

template<> template<> const size_t entity::Type::k_type_index<entity::Type::Header> = 0;
template<> template<> const size_t entity::Type::k_type_index<entity::CName> = 1;
template<> template<> const size_t entity::Type::k_type_index<entity::CParent> = 2;
template<> template<> const size_t entity::Type::k_type_index<rendering::CContext> = 3;
template<> template<> const size_t entity::Type::k_type_index<rendering::CTransform> = 4;
template<> template<> const size_t entity::Type::k_type_index<rendering::CScenario> = 5;
template<> template<> const size_t entity::Type::k_type_index<rendering::CInstance> = 6;
template<> template<> const size_t entity::Type::k_type_index<rendering::CInstancer> = 7;
template<> template<> const size_t entity::Type::k_type_index<rendering::CBase> = 8;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CPosition> = 9;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CVelocity> = 10;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CRotation> = 11;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CScale> = 12;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CMass> = 13;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CBounciness> = 14;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CRoughness> = 15;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CGravity> = 16;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CAABB> = 17;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CBox> = 18;
template<> template<> const size_t entity::Type::k_type_index<physics3d::CSphere> = 19;
template<> template<> const size_t entity::Type::k_type_index<simulation::CFrame> = 20;
template<> template<> const size_t entity::Type::k_type_index<simulation::CLocalTime> = 21;
template<> template<> const size_t entity::Type::k_type_index<simulation::CPath> = 22;
template<> template<> const size_t entity::Type::k_type_index<simulation::CConfig> = 23;
template<> template<> const size_t entity::Type::k_type_index<simulation::CThreadWorker> = 24;
template<> template<> const size_t entity::Type::k_type_index<simulation::CUUID> = 25;
template<> template<> const size_t entity::Type::k_type_index<network::CCertificate> = 26;
template<> template<> const size_t entity::Type::k_type_index<network::CServer> = 27;
template<> template<> const size_t entity::Type::k_type_index<network::CPeer> = 28;
template<> template<> const size_t entity::Type::k_type_index<loading::CIdentifier> = 29;
template<> template<> const size_t entity::Type::k_type_index<loading::CSaveable> = 30;
template<> template<> const size_t entity::Type::k_type_index<loading::CEntityDatabase> = 31;
template<> template<> const size_t entity::Type::k_type_index<player::EntityPtr> = 32;
template<> template<> const size_t entity::Type::k_type_index<player::AvatarPath> = 33;
template<> template<> const size_t entity::Type::k_type_index<player::CPlayer> = 34;
template<> template<> const size_t entity::Type::k_type_index<player::CPlayers> = 35;
template<> template<> const size_t entity::Type::k_type_index<player::CPlayerAvatar> = 36;
template<> template<> const size_t entity::Type::k_type_index<spatial3d::CLoader> = 37;
template<> template<> const size_t entity::Type::k_type_index<spatial3d::CRegion> = 38;
template<> template<> const size_t entity::Type::k_type_index<spatial3d::CEntity> = 39;
template<> template<> const size_t entity::Type::k_type_index<spatial3d::CScale> = 40;
template<> template<> const size_t entity::Type::k_type_index<spatial3d::CWorld> = 41;
template<> template<> const size_t entity::Type::k_type_index<universe::CWorld> = 42;
template<> template<> const size_t entity::Type::k_type_index<galaxy::CWorld> = 43;
template<> template<> const size_t entity::Type::k_type_index<galaxy::CStar> = 44;
template<> template<> const size_t entity::Type::k_type_index<galaxyrender::CWorld> = 45;
template<> template<> const size_t entity::Type::k_type_index<voxel::CWorld> = 46;
template<> template<> const size_t entity::Type::k_type_index<voxelrender::CWorld> = 47;

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
	MakeTypeInfo<simulation::CFrame>(),
	MakeTypeInfo<simulation::CLocalTime>(),
	MakeTypeInfo<simulation::CPath>(),
	MakeTypeInfo<simulation::CConfig>(),
	MakeTypeInfo<simulation::CThreadWorker>(),
	MakeTypeInfo<simulation::CUUID>(),
	MakeTypeInfo<network::CCertificate>(),
	MakeTypeInfo<network::CServer>(),
	MakeTypeInfo<network::CPeer>(),
	MakeTypeInfo<loading::CIdentifier>(),
	MakeTypeInfo<loading::CSaveable>(),
	MakeTypeInfo<loading::CEntityDatabase>(),
	MakeTypeInfo<player::EntityPtr>(),
	MakeTypeInfo<player::AvatarPath>(),
	MakeTypeInfo<player::CPlayer>(),
	MakeTypeInfo<player::CPlayers>(),
	MakeTypeInfo<player::CPlayerAvatar>(),
	MakeTypeInfo<spatial3d::CLoader>(),
	MakeTypeInfo<spatial3d::CRegion>(),
	MakeTypeInfo<spatial3d::CEntity>(),
	MakeTypeInfo<spatial3d::CScale>(),
	MakeTypeInfo<spatial3d::CWorld>(),
	MakeTypeInfo<universe::CWorld>(),
	MakeTypeInfo<galaxy::CWorld>(),
	MakeTypeInfo<galaxy::CStar>(),
	MakeTypeInfo<galaxyrender::CWorld>(),
	MakeTypeInfo<voxel::CWorld>(),
	MakeTypeInfo<voxelrender::CWorld>(),
};