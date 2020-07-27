#pragma once

#include <chrono>
#include <iostream>
#include <sstream>
#include "std_extra.hpp"

#include "GL/glew.h"
#include "GLFW/glfw3.h"

// makro definitions:
//#define DEBUG_STATIC_GRID
//#define DEBUG_QUADTREE
//#define DEBUG_QUADTREE2
//#define DEBUG_PATHFINDING

// ------------------

#include "robin_hood.h"

#include "Timing.hpp"
#include "Perf.hpp"
#include "BaseTypes.hpp"
#include "RenderTypes.hpp"
#include "QuadTree.hpp"
#include "Physics.hpp"
#include "input.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "EventHandler.hpp"
#include "World.hpp"

// Core Systems
#include "MovementSystem.hpp"
#include "CollisionSystem.hpp"
#include "PhysicsSystem.hpp"
#include "BaseSystem.hpp"
#include "Renderer.hpp"


class Engine
{
public:
	Engine(World& wrld, std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_);
	~Engine();

	/* ends programm after finisheing the current tick */
	inline void quit() { running = false; }

	/* call run to start the rpogramm */
	virtual void run() final;

	/* specify what happenes once for initialisation */
	virtual void create() = 0;
	/* specify what happenes every update tick */
	virtual void update(World& world, float deltaTime) = 0;
	/* specify what happenes once for destruction */
	virtual void destroy() = 0;

					/*-- general statistics utility --*/
	/* returns time difference to last physics dispatch, O(1)*/
	inline float getDeltaTime() { return deltaTime; }
	/* returns deltatime or the lowest allowed sim time difference, O(1)*/
	inline float getDeltaTimeSafe() { return std::min(deltaTime, maxDeltaTime); }
	/* returns the number of past iterations , O(1)*/ 
	inline uint32_t getIteration() { return iteration; }
	/* returnes a string wtih formated performance info. The detail level changes how much information is shown, O(1) (os call) */
	std::string getPerfInfo(int detail);

					/*-- input utility --*/
	/* returns the status(KEYSTATUS) of a given key_(KEY), O(1) (mutex locking) */
	InputStatus getKeyStatus(KEY key);
	/* returns if a given key_ is pressed, O(1) (mutex locking) */
	bool keyPressed(KEY key);
	/* returns if a given key_ is released, O(1) (mutex locking) */
	bool keyReleased(KEY key);
	/* returns if a given key_ is repeating, O(1) (mutex locking) */
	bool keyRepeating(KEY key);
	/* returns mouse position in window relative coordinates, O(1) (mutex locking) */
	Vec2 getCursorPos();
	/* returns the keystatus of mouse buttons, O(1) (mutex locking) */
	InputStatus getButtonStatus(BUTTON but);
	/* returns true when a button is pressed, O(1) (mutex locking) */
	bool buttonPressed(BUTTON but);
	/* returns true when a button is NOT pressed, O(1) (mutex locking) */
	bool buttonReleased(BUTTON but);

					/*-- window utility --*/
	/* returns size of window in pixel of your desktop resolution, O(1)*/
	Vec2 getWindowSize();
	/* returns aspect ration width/height of the window, O(1)*/
	float getWindowAspectRatio();
	/* transformes world space coordinates into relative window space coordinates */
	Vec2 getPosWorldSpace(Vec2 windowSpacePos);

					/* graphics utility */
	/*  submit a Drawable to be rendered the next frame, O(1)  */
	void submitDrawable(Drawable && d);
	void submitDrawable(Drawable const& d);
	/* attach a texture to a submited Drawable */
	void attachTexture(uint32_t drawableID, std::string_view name, Vec2 min = { 0,0 }, Vec2 max = { 1,1 });

					/* physics utility */
	/* returns a range (iterator to begin and end) of the collision list for the ent with the id, O(1) */
	std::tuple<std::vector<IndexCollisionInfo>::iterator, std::vector<IndexCollisionInfo>::iterator> getCollisions(entity_index_type index);
	std::tuple<std::vector<IndexCollisionInfo>::iterator, std::vector<IndexCollisionInfo>::iterator> getCollisions(entity_id id);

	friend class CollisionsView;
	template<typename entityReference>
	class CollisionsView {
		using iterator = std::vector<IndexCollisionInfo>::iterator;
		Engine& engine; iterator beginIter; iterator endIter;
	public:
		CollisionsView(Engine& engine, entityReference ent) : engine{ engine } {
			auto [begin, end] = engine.getCollisions(ent); this->beginIter = begin; this->endIter = end;
		}
		inline iterator begin() { return beginIter; }
		inline iterator end() { return endIter; }
		inline size_t size() { return std::distance(beginIter, endIter); }
	};

	/*
		returns a view to the collisions of an entity id or entity index this is only slightly slower then getCollisions(entRef entity)
	*/
	template<typename entityReference>
	CollisionsView<entityReference> collisions_view(entityReference entity) {
		return CollisionsView<entityReference>(*this, entity);
	}

	/* returns a Grid that with bools, if a cell is "true" there is a solid object, if it is "false" there is no solid object
		the position of the cells can be calculated using the minPos and the cellSize member variables, O(1) */
	GridPhysics<bool> const& getStaticGrid();

public:
	World& world;
	EventHandler events;
	Camera camera;

	uint32_t freeDrawableID{ 0x80000000 };

	JobManager jobManager;

	// core systems
	BaseSystem baseSystem;
	MovementSystem movementSystem;
	CollisionSystem collisionSystem;
	PhysicsSystem physicsSystem;

	PerfLogger perfLog;

private:
	void rendererUpdate(World& world);
private:
	// meta
	std::chrono::microseconds minimunLoopTime;
	bool running;
	uint32_t iteration;
	float maxDeltaTime;

	std::chrono::microseconds new_deltaTime;
	float deltaTime;

	// window
	std::shared_ptr<Window> window;

	// render
	Renderer renderer;
};

__forceinline void Engine::submitDrawable(Drawable && d) {
	renderer.submit(d);
}

__forceinline void Engine::submitDrawable(Drawable const& d) {
	renderer.submit(d);
}

__forceinline void Engine::attachTexture(uint32_t drawableID, std::string_view name, Vec2 min, Vec2 max)
{
	renderer.attachTex(drawableID, name, min, max);
}

__forceinline std::tuple<std::vector<IndexCollisionInfo>::iterator, std::vector<IndexCollisionInfo>::iterator> Engine::getCollisions(entity_id id)
{
	return getCollisions(world.getIndex(id));
}