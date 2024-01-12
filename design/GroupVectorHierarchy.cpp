#include <vector>
#include <string>
#include <tuple>
#include <unordered_map>

using size_t = uint64_t;
struct UUID;
namespace robin_hood { template<class K, class V> using unordered_flat_map = std::unordered_map<K, V>; }
namespace entt { struct entity; entity null; }

struct Simulation { 
	struct registry {
		template<class... T> struct view { std::tuple<entt::entity, T&&...> each(); };
		template<class... T> view<T...>& view(); 
		template<class T> void emplace(entt::entity);
		template<class T> void clear();
		template<class T> size_t count();
	}; 
	registry& GetRegistry();
	template<class Event> void PostEvent(const Event&);
};

template<class Tag>
struct GroupComponent
{
	size_t num_members
};

template<class Tag>
struct GroupMemberComponent
{
	UUID group_id;
	entt::entity group;
};

template<class Tag>
struct MultiGroupMemberComponent
{
	robin_hood::unordered_flat_map<UUID, std::pair<entt::entity, Tag::Data>> groups;
};



template<class Tag>
struct VectorChildComponent
{
	entt::entity parent;
};

template<class Tag>
struct VectorComponent
{
	std::vector<entt::entity> children
};



template<class Tag>
struct HeirarchyComponent
{
	entt::entity parent;
	entt::entity next;
	entt::entity prev;

	size_t num_children;
	entt::entity first;
	entt::entity last;
};



struct DeletedComponent {};

struct ParentDeletedComponent {};

struct ProcessDeletedEvent {};

struct ProcessParentDeletedEvent {};



template<class Tag>
void OnProcessDeleted(Simulation& simulation, const ProcessDeletedEvent&)
{
	for (auto&& [entity, vector] : simulation.GetRegistry().view<VectorComponent<Tag>, DeletedComponent>().each())
	{
		for (entt::entity child_entity : vector.children)
		{
			simulation.GetRegistry().emplace<ParentDeletedComponent>(child_entity);
		}
	}

	auto group_view = simulation.GetRegistry().view<GroupComponent<Tag>>();

	for (auto&& [entity, member] : simulation.GetRegistry().view<GroupMemberComponent<Tag>, DeletedComponent>().each())
	{
		entt::entity member_entity = member.group;

		if (member_entity != entt::null)
		{
			GroupComponent<Tag>& group = group_view.get(member_entity);

			group.num_members--;

			if (group.num_members == 0)
			{
				simulation.GetRegistry().emplace<DeletedComponent>(member_entity);
			}
		}
	}

	for (auto&& [entity, multi_member] : simulation.GetRegistry().view<MultiGroupMemberComponent<Tag>, DeletedComponent>().each())
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
					simulation.GetRegistry().emplace<DeletedComponent>(member_entity);
				}
			}
		}
	}

	auto heirarchy_view = simulation.GetRegistry().view<HeirarchyComponent<Tag>>();

	for (auto&& [entity, parent_heirarchy] : simulation.GetRegistry().view<HeirarchyComponent<Tag>, DeletedComponent>().each())
	{
		if (parent_heirarchy.num_children > 0)
		{
			entt::entity child_entity = parent_heirarchy.first;

			while (child_entity != entt::null)
			{
				HeirarchyComponent<Tag>& child_heirarchy = heirarchy_view.get(child_entity);

				simulation.GetRegistry().emplace<ParentDeletedComponent>(child_entity);

				child_entity = child_heirarchy.next;
			}
		}
	}
}

template<class Tag>
void OnProcessParentDeleted(Simulation& simulation, const ProcessParentDeletedEvent&)
{
	for (auto&& [entity, vector_child] : simulation.GetRegistry().view<VectorChildComponent<Tag>, ParentDeletedComponent>().each())
	{
		simulation.GetRegistry().emplace<DeletedComponent>(entity);
	}
}

void ProcessDeleted(Simulation& simulation)
{
	while (true)
	{
		simulation.PostEvent(ProcessDeletedEvent());

		simulation.GetRegistry().clear<DeletedComponent>();

		if (simulation.GetRegistry().count<ParentDeletedComponent>() > 0)
		{
			simulation.PostEvent(ProcessParentDeletedEvent());

			simulation.GetRegistry().clear<ParentDeletedComponent>();
		}
		else
		{
			break;
		}
	}
}