#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/classes/texture.hpp>
#include <godot_cpp/classes/mesh.hpp>

namespace voxel_game::voxel
{
	struct CBlockColor
	{
		godot::Color color;
	};

	struct CBlockTexture
	{
		uint64_t texture_id = 0;
	};

	struct CBlockAnimated
	{
		uint8_t frame_count = 0;
	};

	struct CBlockCustomTexture
	{
		godot::Ref<godot::Texture> texture;
	};

	struct CBlockModel
	{
		godot::Ref<godot::Mesh> texture;
	};
}