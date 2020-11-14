#pragma once

#include <chrono>
#include <iostream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "robin_hood.h"

#include "utils.hpp"
#include "Timing.hpp"
#include "Perf.hpp"
#include "BaseTypes.hpp"
#include "RenderTypes.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "World.hpp"
#include "InputManager.hpp"
#include "UIManager.hpp"
#include "Renderer.hpp"
#include "JobManager.hpp"
#include "Physics.hpp"

class Engine {
public:
	Engine(std::string windowName, uint32_t windowWidth, uint32_t windowHeight);
	~Engine();

	/* ends programm after finishing the current frame */
	virtual void quit() final { running = false; }

	/* call run to start the loop */
	virtual void run() final;

	/* specify what happenes once for initialisation */
	virtual void create() = 0;
	/* specify what happenes every update tick */
	virtual void update(float deltaTime) = 0;
	/* specify what happenes once for destruction */
	virtual void destroy() = 0;

					/*-- general statistics utility --*/
	/* returns time difference to last physics dispatch, O(1)*/
	float getDeltaTime(int sampleSize = 1);
	/* returns deltatime or the lowest allowed sim time difference, O(1)*/
	float getDeltaTimeSafe(int sampleSize = 1) { return std::min(getDeltaTime(sampleSize), maxDeltaTime); }
	/* returns the number of past iterations , O(1)*/ 
	uint32_t getIteration() { return iteration; }

					/*-- window utility --*/
	/* returns size of window in pixel of your desktop resolution, O(1)*/
	static Vec2 getWindowSize();
	/* returns aspect ration width/height of the window, O(1)*/
	static float getWindowAspectRatio();

	inline static JobManager jobManager{ std::thread::hardware_concurrency() };

	// core systems
	inline static PerfLogger perfLog;

	inline static World world;

	// window
private: 
	inline static Window window;
public:

	/*
	* singleton class used for rendering in the whole program
	*/
	inline static Renderer renderer;

	// Input
	inline static InputManager in{ window };

	// UI
	inline static UIContext uiContext;
	inline static UIManager ui{ renderer, in };
private:

	// meta
	inline static bool bInstantiated{ false }; // there can only be one active instance of the Engine as it is a singleton

	bool running{ true };

	uint32_t iteration{ 0 };

	std::chrono::microseconds minimunLoopTime;

	std::chrono::microseconds new_deltaTime;
	float deltaTime;
	float maxDeltaTime;
	std::deque<float> deltaTimeQueue;
};
