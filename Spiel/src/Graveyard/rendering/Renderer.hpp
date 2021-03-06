#pragma once

#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "../types/Timing.hpp"
#include "../types/BaseTypes.hpp"
#include "Window.hpp"
#include "Sprite.hpp"
#include "RenderBuffer.hpp"
#include "RenderingWorker.hpp"

#include "OpenGLAbstraction/OpenGLTexture.hpp"

#define RENDERER_DEBUG_0

#ifdef RENDERER_DEBUG_0
#define rd_at(x) at(x)
#else 
#define rd_at(x) operator[](x)
#endif

enum class RenderState : int{
	Uninitialized = 0,
	PreWait = 1,
	PreStart = 2,
};

class Renderer {
public:

	void initialize(Window* wndw);
	/*
	* resets all fields to an uninitialised state
	*/
	void reset();

	/*
	* waits for the rendering worker to finish
	*/
	void waitTillFinished();

	bool finished();

	/*
	* returns how many layers got added/ deleted
	*/
	int setLayerCount(size_t lc)
	{
		size_t oldSize = frontBuffer->layers.size();
		frontBuffer->layers.resize(lc);
		return (int)frontBuffer->layers.size() - (int)oldSize;
	}
	size_t getLayerCount() const
	{
		return frontBuffer->layers.size();
	}
	/*
	* sets all existing layers to the default RenderLayer
	*/
	void resetLayers()
	{
		for (auto& layer : frontBuffer->layers) {
			layer = RenderLayer();
		}
	}

	RenderLayer& getLayer(int index) { return frontBuffer->layers.rd_at(index); }

	void setLayerBufferTemporary(int index, bool value = true)
	{
		frontBuffer->layers.rd_at(index).bClearEveryFrame = value;
	}
	bool isLayerBufferTemporary(int index) const
	{
		return frontBuffer->layers.rd_at(index).bClearEveryFrame;
	}

	void submit(Sprite const& d, int layer = 0)
	{
		assert(frontBuffer->layers.size() > layer);
		frontBuffer->layers.rd_at(layer).push(d);
	}
	void submit(Sprite&& d, int layer = 0)
	{
		assert(frontBuffer->layers.size() > layer);
		frontBuffer->layers.rd_at(layer).push(d);
	}
	void submit(std::vector<Sprite> const& in, int layer = 0)
	{
		assert(frontBuffer->layers.size() > layer);
		frontBuffer->layers.rd_at(layer).push(in);
	}

	/*
	* writes frontbuffer data to the backbuffer and starts the rendering worker
	*/
	void render();

	// returns the time spend rendering
	std::chrono::microseconds getRenderingTime() { return renderingTime ; }
	// returns the time spend waiting for the worker to finish
	std::chrono::microseconds getWaitedTime() { return syncTime; }
	/*
	* returns the amount of drawcalls the last rendererd frame
	*/
	size_t getDrawCallsLastFrame() const { return drawCallCount; }

	size_t getSpriteCountLastFrame() const { return spriteCountLastFrame; }

	/**
	 * compile time convertion of coordinate system of vector.
	 * 
	 * \param From RenderSpace the coord is in
	 * \param To RenderSpace the corrd should be converted to
	 * \param vec vector to convert
	 * \return converted vector
	 */
	template<RenderSpace From, RenderSpace To>
	Vec2 convertCoordSys(Vec2 vec) const;

	/**
	 * run time convertion of coordinate system of vector.
	 * 
	 * \param vec vector to convert
	 * \param from RenderSpace the coord is in
	 * \param to RenderSpace the corrd should be converted to
	 * \return converted vector
	 */
	Vec2 convertCoordSys(Vec2 vec, RenderSpace from, RenderSpace to) const;

	Camera& getCamera() { return frontBuffer->camera; }

	TextureManager tex;
private:
	void flushSubmissions();

	void assertIsState(RenderState state)
	{
		if (state != this->state) {
			std::cerr << "ERROR: wait was called in wrong renderer state, state was: " << (int)this->state << ", state should be " << (int)state << std::endl;
			exit(-1);
		}
	}

	void assertIsNotState(RenderState state)
	{
		if (state == this->state) {
			std::cerr << "ERROR: wait was called in wrong renderer state, state was: " << (int)this->state << ", state should be " << (int)state << std::endl;
			exit(-1);
		}
	}

	RenderState state{ RenderState::Uninitialized };
	std::chrono::microseconds renderingTime{ 0 };
	std::chrono::microseconds syncTime{ 0 };
	size_t drawCallCount{ 0 };
	size_t spriteCountLastFrame{ 0 };
	// concurrent data:
	std::unique_ptr<RenderBuffer> frontBuffer;
	SharedRenderData workerSharedData;	
	Window* window{ nullptr };
	//std::thread workerThread;
	RenderingWorker worker{ &workerSharedData, tex.getBackend() };
	u64 jobTag{ 0xFFFFFFFFFFFFFFFF };
};

template<> Vec2 Renderer::convertCoordSys<RenderSpace::Pixel, RenderSpace::Window>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::Camera, RenderSpace::Window>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Window>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::Window, RenderSpace::Pixel>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::Window, RenderSpace::Camera>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::Window, RenderSpace::UniformWindow>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::Camera, RenderSpace::Pixel>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::Pixel, RenderSpace::Camera>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Pixel>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::Pixel, RenderSpace::UniformWindow>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::UniformWindow, RenderSpace::Camera>(Vec2 coord) const;
template<> Vec2 Renderer::convertCoordSys<RenderSpace::Camera, RenderSpace::UniformWindow>(Vec2 coord) const;