#include "GroupSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

namespace sim
{
	template<class Tag>
	void OnProcessDeleted(Simulation& simulation, const ProcessDeletedEntitiesEvent& event)
	{
		for (auto&& [entity, vector] : simulation.registry.view<VectorComponent<Tag>, DeletedComponent>().each())
		{
			for (entt::entity child_entity : vector.children)
			{
				simulation.registry.emplace<ParentDeletedComponent>(child_entity);
			}
		}

		auto group_view = simulation.registry.view<GroupComponent<Tag>>();

		for (auto&& [entity, member] : simulation.registry.view<GroupMemberComponent<Tag>, DeletedComponent>().each())
		{
			entt::entity member_entity = member.group;

			if (member_entity != entt::null)
			{
				GroupComponent<Tag>& group = group_view.get(member_entity);

				group.num_members--;

				if (group.num_members == 0)
				{
					simulation.registry.emplace<DeletedComponent>(member_entity);
				}
			}
		}

		for (auto&& [entity, multi_member] : simulation.registry.view<MultiGroupMemberComponent<Tag>, DeletedComponent>().each())
		{
			for (auto&& [id, member_pack] : multi_member.groups)
			{
				entt::entity member_entity = member_pack.first();

				if (member_entity != entt::null)
				{
					GroupComponent<Tag>& group = group_view.get(member_entity);

					group.num_members--;

					if (group.num_members == 0)
					{
						simulation.registry.emplace<DeletedComponent>(member_entity);
					}
				}
			}
		}

		auto heirarchy_view = simulation.registry.view<HierarchyComponent<Tag>>();

		for (auto&& [entity, parent_heirarchy] : simulation.registry.view<HierarchyComponent<Tag>, DeletedComponent>().each())
		{
			if (parent_heirarchy.num_children > 0)
			{
				entt::entity child_entity = parent_heirarchy.first;

				while (child_entity != entt::null)
				{
					HierarchyComponent<Tag>& child_heirarchy = heirarchy_view.get(child_entity);

					simulation.registry.emplace<ParentDeletedComponent>(child_entity);

					child_entity = child_heirarchy.next;
				}
			}
		}
	}

	template<class Tag>
	void OnProcessParentDeleted(Simulation& simulation, const ProcessDeletedParentsEvent& event)
	{
		for (auto&& [entity, vector_child] : simulation.registry.view<VectorChildComponent<Tag>, ParentDeletedComponent>().each())
		{
			simulation.registry.emplace<DeletedComponent>(entity);
		}
	}

	void GroupSystem::OnInitialize(Simulation& simulation)
	{

	}

	void GroupSystem::OnShutdown(Simulation& simulation)
	{

	}

	void GroupSystem::OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event)
	{
		simulation.messager.PostEvent(ProcessDeletedEntitiesEvent());

		simulation.registry.clear<DeletedComponent>();

		if (simulation.registry.storage<ParentDeletedComponent>().size() > 0)
		{
			simulation.messager.PostEvent(ProcessDeletedParentsEvent());

			simulation.registry.clear<ParentDeletedComponent>();
		}
	}
}