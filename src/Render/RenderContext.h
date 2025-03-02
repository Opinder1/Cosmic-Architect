#pragma once

#include "RenderAllocator.h"

#include "Commands/TypedCommandBuffer.h"

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

namespace voxel_game::rendering
{
	struct ThreadContext
	{
		// TODO Make into template with <godot::RenderingServer>
		TypedCommandBuffer commands;

		Allocator allocator;
	};

	void SetContext(ThreadContext& context);

	ThreadContext& GetContext();

	template<auto Command, class... Args>
	void AddCommand(Args&&... args)
	{
		GetContext().commands.AddCommand<Command>(args...);
	}

	inline godot::RID AllocRID(RIDType type)
	{
		return GetContext().allocator.GetRID(type);
	}
}

using RD = godot::RenderingDevice;
using RS = godot::RenderingServer;