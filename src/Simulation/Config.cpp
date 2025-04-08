#include "Config.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/os.hpp>

namespace voxel_game::simulation
{
	void LoadJsonConfig(Config& config)
	{
		if (config.path.is_empty())
		{
			return;
		}

		godot::Ref<godot::FileAccess> file = godot::FileAccess::open(config.path, godot::FileAccess::READ);

		if (file.is_null() || file->get_error() != godot::Error::OK)
		{
			DEBUG_PRINT_ERROR(godot::vformat("Failed to open the file %s", config.path));
			return;
		}

		config.values = godot::JSON::parse_string(file->get_as_text());

		file->close();
	}

	void SaveJsonConfig(Config& config)
	{
		if (config.path.is_empty())
		{
			return;
		}

		godot::Ref<godot::FileAccess> file = godot::FileAccess::open(config.path, godot::FileAccess::WRITE);

		if (file.is_null() || file->get_error() != godot::Error::OK)
		{
			DEBUG_PRINT_ERROR(godot::vformat("Failed to open the file %s", config.path));
			return;
		}

		file->store_string(godot::JSON::stringify(config.values, "    "));

		file->close();
	}

	void InitializeConfig(Config& config, const godot::String& path, const ConfigDefaults& defaults)
	{
		config.path = path;

		simulation::LoadJsonConfig(config);

		for (auto&& [key, value] : defaults)
		{
			if (!config.values.has(key))
			{
				config.values[key] = value;
			}
		}

		simulation::SaveJsonConfig(config);
	}
}