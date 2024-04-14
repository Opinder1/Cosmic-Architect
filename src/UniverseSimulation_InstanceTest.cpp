#include "UniverseSimulation.h"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/rd_texture_format.hpp>
#include <godot_cpp/classes/rd_texture_view.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

namespace voxel_game
{
    struct Position
    {
        godot::Vector3i position;
    };

    struct Velocity
    {
        godot::Vector3i velocity;
    };

    struct Instance
    {
        godot::RID instance;
    };

    struct Mesh
    {
        godot::RID mesh;
    };

    struct Scenario
    {
        godot::RID scenario;
    };

    struct GodotRenderTestModule
    {
        GodotRenderTestModule(flecs::world& world)
        {
            auto* rendering_server = godot::RenderingServer::get_singleton();

            world.module<GodotRenderTestModule>();

            world.component<Position>();
            world.component<Velocity>();
            world.component<Instance>();
            world.component<Scenario>();
            world.component<Mesh>();

            world.observer<Instance>()
                .event(flecs::OnAdd)
                .iter([rendering_server](flecs::iter& it, Instance* instances)
            {
                for (auto i : it)
                {
                    instances[i].instance = rendering_server->instance_create();
                }
            });

            world.observer<const Instance>()
                .event(flecs::OnRemove)
                .iter([rendering_server](flecs::iter& it, const Instance* instances)
            {
                for (auto i : it)
                {
                    rendering_server->free_rid(instances[i].instance);
                }
            });

            world.system<Position, const Velocity>()
                .iter([](flecs::iter& it, Position* positions, const Velocity* velocities)
            {
                for (auto i : it)
                {
                    positions[i].position.x += velocities[i].velocity.x * it.delta_time();
                    positions[i].position.y += velocities[i].velocity.y * it.delta_time();
                }
            });

            world.observer<const Scenario, const Instance>()
                .event(flecs::OnSet)
                .iter([rendering_server](flecs::iter& it, const Scenario* scenarios, const Instance* instances)
            {
                for (auto i : it)
                {
                    rendering_server->instance_set_scenario(instances[i].instance, scenarios[i].scenario);
                }
            });

            world.observer<const Mesh, const Instance>()
                .event(flecs::OnSet)
                .iter([rendering_server](flecs::iter& it, const Mesh* meshes, const Instance* instances)
            {
                for (auto i : it)
                {
                    rendering_server->instance_set_base(instances[i].instance, meshes[i].mesh);
                }
            });

            world.system<const Position, const Instance>()
                .iter([rendering_server](flecs::iter& it, const Position* positions, const Instance* instances)
            {
                for (auto i : it)
                {
                    godot::Transform3D transform;
                    transform.origin = positions[i].position;
                    rendering_server->instance_set_transform(instances[i].instance, transform);
                }
            });

            godot::UtilityFunctions::print("Added render module");
        }
    };

    void print_limits()
    {
        auto* rendering_server = godot::RenderingServer::get_singleton();
        auto* rendering_device = rendering_server->get_rendering_device();

        auto limits =
        {
            "LIMIT_MAX_BOUND_UNIFORM_SETS",
            "LIMIT_MAX_FRAMEBUFFER_COLOR_ATTACHMENTS",
            "LIMIT_MAX_TEXTURES_PER_UNIFORM_SET",
            "LIMIT_MAX_SAMPLERS_PER_UNIFORM_SET",
            "LIMIT_MAX_STORAGE_BUFFERS_PER_UNIFORM_SET",
            "LIMIT_MAX_STORAGE_IMAGES_PER_UNIFORM_SET",
            "LIMIT_MAX_UNIFORM_BUFFERS_PER_UNIFORM_SET",
            "LIMIT_MAX_DRAW_INDEXED_INDEX",
            "LIMIT_MAX_FRAMEBUFFER_HEIGHT",
            "LIMIT_MAX_FRAMEBUFFER_WIDTH",
            "LIMIT_MAX_TEXTURE_ARRAY_LAYERS",
            "LIMIT_MAX_TEXTURE_SIZE_1D",
            "LIMIT_MAX_TEXTURE_SIZE_2D",
            "LIMIT_MAX_TEXTURE_SIZE_3D",
            "LIMIT_MAX_TEXTURE_SIZE_CUBE",
            "LIMIT_MAX_TEXTURES_PER_SHADER_STAGE",
            "LIMIT_MAX_SAMPLERS_PER_SHADER_STAGE",
            "LIMIT_MAX_STORAGE_BUFFERS_PER_SHADER_STAGE",
            "LIMIT_MAX_STORAGE_IMAGES_PER_SHADER_STAGE",
            "LIMIT_MAX_UNIFORM_BUFFERS_PER_SHADER_STAGE",
            "LIMIT_MAX_PUSH_CONSTANT_SIZE",
            "LIMIT_MAX_UNIFORM_BUFFER_SIZE",
            "LIMIT_MAX_VERTEX_INPUT_ATTRIBUTE_OFFSET",
            "LIMIT_MAX_VERTEX_INPUT_ATTRIBUTES",
            "LIMIT_MAX_VERTEX_INPUT_BINDINGS",
            "LIMIT_MAX_VERTEX_INPUT_BINDING_STRIDE",
            "LIMIT_MIN_UNIFORM_BUFFER_OFFSET_ALIGNMENT",
            "LIMIT_MAX_COMPUTE_SHARED_MEMORY_SIZE",
            "LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_X",
            "LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Y",
            "LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Z",
            "LIMIT_MAX_COMPUTE_WORKGROUP_INVOCATIONS",
            "LIMIT_MAX_COMPUTE_WORKGROUP_SIZE_X",
            "LIMIT_MAX_COMPUTE_WORKGROUP_SIZE_Y",
            "LIMIT_MAX_COMPUTE_WORKGROUP_SIZE_Z",
            "LIMIT_MAX_VIEWPORT_DIMENSIONS_X",
            "LIMIT_MAX_VIEWPORT_DIMENSIONS_Y"
        };

        size_t i = 0;
        for (const char* limit : limits)
        {
            godot::UtilityFunctions::print(limit, ": ", rendering_device->limit_get(static_cast<godot::RenderingDevice::Limit>(i++)));
        }
    }

    godot::RID create_texture()
    {
        auto* rendering_server = godot::RenderingServer::get_singleton();
        auto* rendering_device = rendering_server->get_rendering_device();

        godot::Ref<godot::RDTextureFormat> format;
        godot::Ref<godot::RDTextureView> view;
        godot::PackedByteArray data;
        godot::TypedArray<godot::PackedByteArray> data_array;
        godot::PackedByteArray update_data;

        size_t width = 16, height = 16, layers = 2048;

        format.instantiate();
        format->set_format(godot::RenderingDevice::DATA_FORMAT_R8G8B8A8_UNORM);
        format->set_texture_type(godot::RenderingDevice::TEXTURE_TYPE_2D_ARRAY);
        format->set_usage_bits(godot::RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT | godot::RenderingDevice::TEXTURE_USAGE_CAN_UPDATE_BIT);
        format->set_width(width);
        format->set_height(height);
        format->set_array_layers(layers);

        view.instantiate();

        data.resize(width * height * 4);
        data.fill(0);

        for (size_t i = 0; i < layers; i++) data_array.push_back(data);

        auto texture_buffer = rendering_device->texture_create(format, view, data_array);
        godot::UtilityFunctions::print("texture_buffer: ", texture_buffer);

        update_data.resize(width * height * 4);
        update_data.fill(0);

        auto texture_layer = rendering_device->texture_create_shared_from_slice(view, texture_buffer, 2047, 0, 1, godot::RenderingDevice::TEXTURE_SLICE_2D);

        rendering_device->texture_update(texture_layer, 0, update_data);
        rendering_device->texture_update(texture_buffer, 2047, update_data);

        //auto texture = rendering_server->texture_rd_create(texture_buffer);
        godot::UtilityFunctions::print("texture: ", texture_buffer);

        return texture_buffer;
    }

    void test_material_uniforms()
    {
        auto* rendering_server = godot::RenderingServer::get_singleton();
        auto* rendering_device = rendering_server->get_rendering_device();

        auto shader = rendering_server->shader_create();

        auto material1 = rendering_server->material_create();
        auto material2 = rendering_server->material_create();

        rendering_server->material_set_shader(material1, shader);
        rendering_server->material_set_shader(material2, shader);

        auto texture1 = create_texture();
        auto texture2 = create_texture();

        godot::UtilityFunctions::print(godot::vformat("Param1: %d (%s)", texture1.get_id(), godot::Variant::get_type_name(godot::Variant(texture1).get_type())));
        godot::UtilityFunctions::print(godot::vformat("Param2: %d (%s)", texture2.get_id(), godot::Variant::get_type_name(godot::Variant(texture2).get_type())));

        rendering_server->material_set_param(material1, "test", texture1);

        godot::Variant texture3 = rendering_server->material_get_param(material1, "test");

        godot::UtilityFunctions::print(godot::vformat("Param2: %d (%s)", texture3.operator godot::RID().get_id(), godot::Variant::get_type_name(texture3.get_type())));

        rendering_server->material_set_param(material2, "test", texture2);

        godot::Variant texture4 = rendering_server->material_get_param(material2, "test");

        godot::UtilityFunctions::print(godot::vformat("Param4: %d (%s)", texture4.operator godot::RID().get_id(), godot::Variant::get_type_name(texture4.get_type())));
    }

	uint64_t UniverseSimulation::CreateInstance(godot::RID mesh, godot::RID scenario)
	{
        SIM_DEFER_COMMAND_V(k_commands->create_instance, 0, mesh, scenario);

        if (static bool once = true; once)
        {
            print_limits();

            //test_material_uniforms();

            m_world.import<GodotRenderTestModule>();

            once = false;
        }

        godot::String name = godot::vformat("Instance %d", rand());

		auto entity = m_world.entity(name.utf8());

		entity.add<Instance>();
		entity.emplace<Scenario>(scenario);
		entity.emplace<Mesh>(mesh);
		entity.add<Position>();
		entity.add<Velocity>();

        QueueSignal(k_signals->levelup_available);

		return entity.id();
	}

	void UniverseSimulation::SetInstancePos(uint64_t instance_id, const godot::Vector3& pos)
	{
        SIM_DEFER_COMMAND(k_commands->set_instance_pos, instance_id, pos);

		auto entity = m_world.entity(instance_id);

		entity.get_mut<Position>()->position = pos;
		entity.modified<Position>();
	}

	bool UniverseSimulation::DeleteInstance(uint64_t instance_id)
	{
        SIM_DEFER_COMMAND_V(k_commands->delete_instance, false, instance_id);

		auto entity = m_world.entity(instance_id);

		entity.destruct();

		return entity.is_alive();
	}
}