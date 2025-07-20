#pragma once

namespace voxel_game
{
	struct Simulation;

	struct Module
	{
		void(*initialize)(Simulation&);
		void(*uninitialize)(Simulation&);
		bool(*is_unload_done)(Simulation&);
		void(*update)(Simulation&);
		void(*worker_update)(Simulation&, size_t);
	};

	extern const Module simulation_module_schematic;
	extern const Module rendering_module_schematic;
	extern const Module debugrender_module_schematic;
	extern const Module spatial3d_module_schematic;
	extern const Module universe_module_schematic;
	extern const Module galaxy_module_schematic;
}