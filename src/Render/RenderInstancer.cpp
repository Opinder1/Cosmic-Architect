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

	void MeshTypeInstancer::AddEntity(UUID entity)
	{
		
	}

	void MeshTypeInstancer::RemoveEntity(UUID entity)
	{

	}

	void MeshTypeInstancer::SetTransform(UUID entity, godot::Transform3D transform)
	{

	}

	Instancer::Instancer()
	{

	}

	void Instancer::SetScenario(godot::RID scenario)
	{
		m_scenario = scenario;

		GrowingSmallVector<UUID, 128> remove_list;

		for (auto&& [id, instancer] : m_instancers)
		{
			if (instancer.EntityCount() == 0)
			{
				remove_list.push_back(id);
			}
		}

		for (UUID id : remove_list)
		{
			RemoveType(id);
		}
	}

	void Instancer::AddType(UUID type_id, BaseType type, godot::RID base)
	{
		m_instancers.emplace(type_id, MeshTypeInstancer(type, base));
	}

	void Instancer::RemoveType(UUID type_id)
	{
		m_instancers.erase(type_id);
	}

	void Instancer::AddEntity(UUID type_id, UUID entity)
	{
		m_instancers.at(type_id).AddEntity(entity);
	}

	void Instancer::RemoveEntity(UUID type_id, UUID entity)
	{
		m_instancers.at(type_id).RemoveEntity(entity);
	}

	void Instancer::SetTransform(UUID type_id, UUID entity, godot::Transform3D transform)
	{
		m_instancers.at(type_id).SetTransform(entity, transform);
	}

	void Instancer::CleanupEmptyTypes()
	{

	}
}