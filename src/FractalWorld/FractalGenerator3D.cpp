#include "FractalGenerator3D.h"

namespace voxel_world
{
	LoadProcessor::LoadProcessor()
	{

	}

	LoadProcessor::~LoadProcessor()
	{

	}
	
	std::string_view LoadProcessor::ProcessFull(std::string_view key, std::string_view value)
	{
		// Build chunk from data

		return REMOVE;
	}

	std::string_view LoadProcessor::ProcessEmpty(std::string_view key)
	{
		// Generate new chunk

		return NOOP;
	}

	void LoadProcessor::ProcessStatus(const tkrzw::Status& status)
	{

	}

	FractalGenerator3D::FractalGenerator3D() :
		m_database_async(&m_database, 12)
	{
		std::map<std::string, std::string> params;

		params.emplace("num_shards", "4");
		params.emplace("dbm", "HashDBM");
		//params.emplace("record_comp_mode", "RECORD_COMP_ZLIB");

		m_database.OpenAdvanced("test", true, tkrzw::File::OPEN_DEFAULT, params);
	}

	void FractalGenerator3D::LoadChunk()
	{
		std::unique_ptr<LoadProcessor> proc;
		m_database_async.Process("test", std::move(proc), false);
	}
}