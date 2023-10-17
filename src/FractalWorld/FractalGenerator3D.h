#pragma once

#include <TKRZW/tkrzw_dbm_shard.h>
#include <TKRZW/tkrzw_dbm_async.h>

#include <string_view>

namespace voxel_world
{
	struct LoadProcessor : public tkrzw::AsyncDBM::RecordProcessor
	{
		LoadProcessor();
		~LoadProcessor();
		
		std::string_view ProcessFull(std::string_view key, std::string_view value) override;

		std::string_view ProcessEmpty(std::string_view key) override;

		void ProcessStatus(const tkrzw::Status& status) override;
	};

	struct SaveProcessor {};

	class FractalGenerator3D
	{
	public:
		FractalGenerator3D();

		void LoadChunk();

	private:
		tkrzw::ShardDBM m_database;

		tkrzw::AsyncDBM m_database_async;
	};
}