#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <mutex>

#include "BaseTypes.hpp"
#include "RenderTypes.hpp"
#include "World.hpp"
#include "collision_detection.hpp"
#include "JobManager.hpp"

struct QuadtreeNode {
	QuadtreeNode() :
		firstSubTree{ 0 }
	{}

	inline bool hasSubTrees() const 
	{
		return (firstSubTree > 0);
	}

	std::vector<uint32_t> collidables;
	uint32_t firstSubTree;
};

class NodeStorage {
public:

	inline QuadtreeNode& get(uint32_t index)  
	{
		if (page(index) > pages.size() || pages[page(index)] == nullptr)
			throw new std::exception();
		return pages[page(index)]->nodes[offset(index)];
	}

	inline const QuadtreeNode& get(uint32_t index) const
	{
		if (page(index) > pages.size() || pages[page(index)] == nullptr)
			throw new std::exception();
		return pages[page(index)]->nodes[offset(index)];
	}

	/*
		allocates 4 new Quadtree nodes
		this is threadsave with the get function
		it returnes the index to the first new Node
		the odther 3 nodes are allways the following 3 indices
	*/
	uint32_t make4Children() 
	{
		std::lock_guard lock(mut);
		if (freeStack.empty()) {
			if (pages.at(page(nextIndex)) == nullptr) {
				pages.at(page(nextIndex)) = new Page();
			}
			pages.at(page(nextIndex))->nodeCount += 4;
			auto index = nextIndex;
			nextIndex += 4;
			return index;
		}
		else {
			int index = freeStack.back();
			freeStack.pop_back();
			if (pages.at(page(index)) == nullptr) {
				pages.at(page(index)) = new Page();
			}
			pages.at(page(index))->nodeCount += 4;
			return index;
		}
	}
	/*
		deallocates 4 Quadtreenodes of a given group of 4 quadtrees
		the last 2 bits are ignored.
	*/
	void kill4Children (uint32_t index)
	{
		std::lock_guard lock(mut);
		index &= ~(3);	// clip off the last 2 bits
		if (pages.at(page(index)) == nullptr) throw new std::exception("error: tried to delete not allocated children");
		pages.at(page(index))->nodeCount -= 4;
		for (int i = 0; i < 4; ++i) {
			get(index + i).firstSubTree = 0;
			get(index + i).collidables.clear();
		}
		if constexpr (DELETE_EMPTY_PAGES) {
			if (pages.at(page(index))->nodeCount == 0) {
				delete pages.at(page(index));
				pages.at(page(index)) = nullptr;
			}
		}
		freeStack.push_back(index);
	}

	/*
		deallocates all allocated storage
	*/
	void reset() 
	{
		for (auto& page : pages) {
			if (page != nullptr) {
				delete page;
				page = nullptr;
			}
		}
	}

	size_t freeCapacity() 
	{
		return freeStack.size() * 4;
	}

private:
	static const uint32_t PAGE_BITS = 6;
	static const uint32_t PAGE_SIZE = 1 << PAGE_BITS;
	static const uint32_t OFFSET_MASK = ~(-1 << PAGE_BITS);
	static const uint32_t PAGE_COUNT = 10000;
	static const bool DELETE_EMPTY_PAGES = true;
	static int offset(uint32_t index) {
		return OFFSET_MASK & index;
	}
	static int page(uint32_t index) {
		return index >> PAGE_BITS;
	}
	struct Page {
		std::array<QuadtreeNode, PAGE_SIZE> nodes;
		uint32_t nodeCount{ 0 };
	};
	std::array<Page*, PAGE_COUNT> pages{ nullptr };
	std::mutex mut{};
	uint32_t nextIndex{ 0 };
	std::vector<uint32_t> freeStack;
}; 


class Quadtree3 {
public:
	Quadtree3(const Vec2 minPos_, const Vec2 maxPos_, const size_t capacity_, World& wrld, JobManager& jobManager, uint8_t TAG = 0);

	~Quadtree3()
	{
		nodes.reset();	// deallocates all heap memory
	}

	void insert(Entity ent, const std::vector<Vec2>& aabbs);

	void broadInsert(const std::vector<Entity>& entities, const std::vector<Vec2>& aabbs);

	void querry(std::vector<Entity>& rVec, const Vec2 qryPos, const Vec2 qrySize) const;

	void querryDebug(const Vec2 qryPos, const Vec2 qrySize, std::vector<Drawable>& draw) const {
		querryDebug(qryPos, qrySize, 0, m_pos, m_size, draw, 0);
	}
	void querryDebugAll(std::vector<Drawable>& draw, const Vec4 color) const {
		querryDebugAll(0, m_pos, m_size, draw, color, 0);
	}

	void clear(const uint32_t thisID);
	void clear() {
		root.collidables.clear();
		clear(0);
		clear(1);
		clear(2);
		clear(3);
	}

	void resetPerPosSize(const Vec2 pos, const Vec2 size);

	void resetPerMinMax(const Vec2 minPos, const  Vec2 maxPos);

	void removeEmptyLeafes(const uint32_t thisID);
	void removeEmptyLeafes() {
		removeEmptyLeafes(0);
		removeEmptyLeafes(1);
		removeEmptyLeafes(2);
		removeEmptyLeafes(3);
	}

	inline void setPosSize(const Vec2 pos, const Vec2 size) {
		m_pos = pos;
		m_size = size;
	}

	inline Vec2 getPosition() const { return m_pos; }
	inline Vec2 getSize() const { return m_size; }

	const uint8_t IGNORE_TAG;
private:
	void insert(const uint32_t ent, const std::vector<Vec2>& aabbs, const uint32_t thisID, const Vec2 thisPos, const Vec2 thisSize, const int depth);
	void broadInsert(const std::vector<Entity>& entities, const std::vector<Vec2>& aabbs, const uint32_t thisID, const Vec2 thisPos, const Vec2 thisSize, const int depth);
	void querry(std::vector<Entity>& rVec, const Vec2 qryPos, const Vec2 qrySize, const uint32_t thisID, const Vec2 thisPos, const Vec2 thisSize) const;
	void querryDebug(const Vec2 qryPos, const Vec2 qrySize, const uint32_t thisID, const Vec2 thisPos, const Vec2 thisSize, std::vector<Drawable>& draw, int depth) const;
	void querryDebugAll(const uint32_t thisID, const Vec2 thisPos, const Vec2 thisSize, std::vector<Drawable>& draw, const Vec4 color, const int depth) const;

	static std::tuple<bool, bool, bool, bool> isInSubtrees(const Vec2 treePos, const Vec2 treeSize, const Vec2 pos, const Vec2 size) {
		const bool u = pos.y + size.y * 0.5f > treePos.y;
		const auto r = pos.x + size.x * 0.5f > treePos.x;
		const bool d = pos.y - size.y * 0.5f < treePos.y;
		const auto l = pos.x - size.x * 0.5f < treePos.x;
		return {
			d & l,
			d & r,
			u & l,
			u & r
		};
	}
public:
	World& world;
	JobManager& jobManager;
private:
	static const int STABILITY = 2;	// 1 = high stability lower speed 2 = lower stability higher speed
	static const int MAX_DEPTH = 15;
	static const int MAX_ENTITIES_PER_JOB = 2000;
	static const int MAX_JOBS = 100;
	std::vector<LambdaJob> jobs;
	std::vector<Tag> tags;

	Vec2 m_pos;
	Vec2 m_size;
	size_t m_capacity;
	NodeStorage nodes;
	QuadtreeNode root;
};

using Quadtree = Quadtree3;