#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/classes/texture.hpp>
#include <godot_cpp/classes/mesh.hpp>

namespace voxel_game::voxel
{
	struct BlockColorComponent
	{
		godot::Color color;
	};

	struct BlockTextureComponent
	{
		uint64_t texture_id = 0;
	};

	struct BlockAnimatedComponent
	{
		uint8_t frame_count = 0;
	};

	struct BlockCustomTextureComponent
	{
		godot::Ref<godot::Texture> texture;
	};

	struct BlockModelComponent
	{
		godot::Ref<godot::Mesh> texture;
	};
}