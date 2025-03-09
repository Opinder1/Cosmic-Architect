#pragma once

#include "RenderInstancer.h"
#include "RenderContext.h"

namespace voxel_game::rendering
{
	MeshTypeInstancer::MeshTypeInstancer(BaseType type, godot::RID base_id)
	{

	}

	size_t MeshTypeInstancer::EntityCount()
	{
		return 0;
	}

	void MeshTypeInstancer::AddEntity(flecs::entity_t entity)
	{
		
	}

	void MeshTypeInstancer::RemoveEntity(flecs::entity_t entity)
	{

	}

	void MeshTypeInstancer::SetTransform(flecs::entity_t entity, godot::Transform3D transform)
	{

	}

	Instancer::Instancer()
	{

	}

	void Instancer::SetScenario(godot::RID scenario)
	{
		m_scenario = scenario;

		GrowingSmallVector<flecs::id_t, 128> remove_list;

		for (auto&& [id, instancer] : m_instancers)
		{
			if (instancer.EntityCount() == 0)
			{
				remove_list.push_back(id);
			}
		}

		for (flecs::id_t id : remove_list)
		{
			RemoveType(id);
		}
	}

	void Instancer::AddType(flecs::id_t type_id, BaseType type, godot::RID base)
	{
		m_instancers.emplace(type_id, MeshTypeInstancer(type, base));
	}

	void Instancer::RemoveType(flecs::id_t type_id)
	{
		m_instancers.erase(type_id);
	}

	void Instancer::AddEntity(flecs::id_t type_id, flecs::entity_t entity)
	{
		m_instancers.at(type_id).AddEntity(entity);
	}

	void Instancer::RemoveEntity(flecs::id_t type_id, flecs::entity_t entity)
	{
		m_instancers.at(type_id).RemoveEntity(entity);
	}

	void Instancer::SetTransform(flecs::id_t type_id, flecs::entity_t entity, godot::Transform3D transform)
	{
		m_instancers.at(type_id).SetTransform(entity, transform);
	}

	void Instancer::CleanupEmptyTypes()
	{

	}
}