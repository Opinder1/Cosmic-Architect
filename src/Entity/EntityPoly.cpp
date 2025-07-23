#include "EntityPoly.h"

#include "Components.h"

using namespace voxel_game;

using Archetype = entity::Factory::Archetype;

const size_t first = __LINE__ + 1;
template<> template<> const size_t Archetype::k_type_index<Archetype::Header> =	__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CName> =				__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CRelationship> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CTransform> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CScenario> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CInstance> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CInstancer> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CBase> =				__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CPosition> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CVelocity> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CRotation> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CScale> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CMass> =				__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CBounciness> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CRoughness> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CGravity> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CAABB> =				__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CBox> =				__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CSphere> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CCertificate> =		__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CServer> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CPeer> =				__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CLoader> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CRegion> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CEntity> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CWorld> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CUniverse> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CGalaxy> =			__LINE__ - first;
template<> template<> const size_t Archetype::k_type_index<CStar> =				__LINE__ - first;

const std::array<PolyTypeInfo, 40> Archetype::k_type_info =
{
	MakeTypeInfo<Archetype::Header>(),
	MakeTypeInfo<CName>(),
	MakeTypeInfo<CRelationship>(),
	MakeTypeInfo<CTransform>(),
	MakeTypeInfo<CScenario>(),
	MakeTypeInfo<CInstance>(),
	MakeTypeInfo<CInstancer>(),
	MakeTypeInfo<CBase>(),
	MakeTypeInfo<CPosition>(),
	MakeTypeInfo<CVelocity>(),
	MakeTypeInfo<CRotation>(),
	MakeTypeInfo<CScale>(),
	MakeTypeInfo<CMass>(),
	MakeTypeInfo<CBounciness>(),
	MakeTypeInfo<CRoughness>(),
	MakeTypeInfo<CGravity>(),
	MakeTypeInfo<CAABB>(),
	MakeTypeInfo<CBox>(),
	MakeTypeInfo<CSphere>(),
	MakeTypeInfo<CCertificate>(),
	MakeTypeInfo<CServer>(),
	MakeTypeInfo<CPeer>(),
	MakeTypeInfo<CLoader>(),
	MakeTypeInfo<CRegion>(),
	MakeTypeInfo<CEntity>(),
	MakeTypeInfo<CWorld>(),
	MakeTypeInfo<CUniverse>(),
	MakeTypeInfo<CGalaxy>(),
	MakeTypeInfo<CStar>(),
};

size_t std::hash<voxel_game::entity::WRef>::operator()(const voxel_game::entity::WRef& wref) const noexcept
{
	return wref.Hash();
}

size_t std::hash<voxel_game::entity::Ref>::operator()(const voxel_game::entity::Ref& ref) const noexcept
{
	return ref.Hash();
}