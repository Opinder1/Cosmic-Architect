#pragma once

#include "Simulation/UUID.h"

#include "FractalWorld/FractalNode3D.h"
#include "FractalWorld/FractalRegion3D.h"
#include "FractalWorld/FractalWorld3D.h"

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/templates/local_vector.hpp>

#include <robin_hood/robin_hood.h>

namespace voxel_world
{
	class VoxelNode;

	struct Block
	{
		uint16_t id = 0;
		uint16_t data = 0;
	};

	class Entity
	{
		sim::UUID m_uuid;

		sim::UUID m_parent_uuid;
		godot::Vector3 m_parent_rel_pos;
		VoxelNode* m_parent_rel_node;
	};

	class BlockEntity : Entity
	{
		godot::Vector3i m_pos;
	};

	class Faction : Entity
	{

	};

	class EntityRef
	{
		sim::UUID m_uuid;
		Entity* m_entity;
		bool m_keep_loaded;
	};

	class VoxelNode : public FractalNode3D
	{
	public:
		VoxelNode(FractalWorld3D& world, FractalCoord3D coord) : FractalNode3D(world, coord) {}

	private:
		godot::LocalVector<EntityRef> m_entities;

		godot::LocalVector<BlockEntity> m_block_entities;

		godot::LocalVector<EntityRef> m_factions;

		Block m_blocks[16][16][16] = {};
		
		bool m_needs_lodding = false;
		bool m_modified = false;
		bool m_edited = false;
	};

	class VoxelWorld;

	class VoxelRegion : public FractalRegion3D
	{
	public:
		explicit VoxelRegion(VoxelWorld& world);
		~VoxelRegion();

	private:
		FractalNode3D* GenerateNode(FractalCoord3D coord) override;

		void OnNodeUpdate(FractalNode3D& node) override;

		void OnNodeUnload(FractalNode3D& node) override;

	private:
		VoxelWorld& m_world;
	};

	class VoxelWorld : public Entity, public FractalWorld3D
	{
	public:
		explicit VoxelWorld(sim::UUID owning_simulation);
		~VoxelWorld();

	public:
		Block GetBaseBlock(const godot::Vector3i& pos) const;

		Block GetLODBlock(const FractalCoord3D& coord) const;

	private:
		FractalRegion3D* GenerateRegion() override;

	private:
		int m_generator;

		int m_stream;

		godot::Vector3i m_bounds;
	};

	class VoxelWorldNode : public godot::Node3D
	{
		GDCLASS(VoxelWorldNode, godot::Node3D);

	public:
		explicit VoxelWorldNode();
		~VoxelWorldNode();

	private:
		static void _bind_methods();

	private:
		sim::UUID m_root_simulation;

		std::unique_ptr<VoxelWorld> m_root_world;
	};
}