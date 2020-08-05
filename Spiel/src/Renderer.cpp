#include "Renderer.hpp"

#include "windows.h"

Renderer::Renderer(std::shared_ptr<Window> wndw) :
	window{ wndw },
	workerSharedData{ std::make_shared<RenderingSharedData>() },
	workerThread{ RenderingWorker(window, workerSharedData) },
	renderingTime{ 0 },
	syncTime{ 0 },
	frontBuffer{ std::make_shared<RenderBuffer>()}
{
	SetThreadPriority(workerThread.native_handle(), 0);	// windows sceduling function
	workerThread.detach();
}

void Renderer::waitTillFinished() {	
	assert(!wasFushCalled);	// -> did not call render after flush
	assert(!wasWaitCalled);	// -> called wait twice!
	wasWaitCalled = true;
	if (!wasEndCalled) {
		Timer t(syncTime);
		std::unique_lock<std::mutex> switch_lock(workerSharedData->mut);
		workerSharedData->cond.wait(switch_lock, [&]() { return workerSharedData->ready == true; });	// wait for worker to finish
		// reset data
		workerSharedData->ready = false; // reset ready flag
		renderingTime = workerSharedData->new_renderTime;
	}
}

void Renderer::flushSubmissions() {
	assert(wasWaitCalled);	// -> wait was not called!
	assert(!wasFushCalled);	// -> flush was called twice!
	wasFushCalled = true;
	if (!wasEndCalled) {
		workerSharedData->renderBuffer->drawables.clear();
		std::swap(frontBuffer, workerSharedData->renderBuffer);
	}
}

void Renderer::startRendering() {
	assert(wasFushCalled);	// -> did not call flush!
	assert(wasWaitCalled);	// -> did not call wait!
	wasWaitCalled = false;
	wasFushCalled = false;
	if (!wasEndCalled) {
		workerSharedData->cond.notify_one(); // wake up worker
	}
}

void Renderer::end()
{
	if (!wasEndCalled) {
		wasEndCalled = true;
		if (!wasWaitCalled) {	// check if we still need to sync for the renderer
			std::unique_lock<std::mutex> switch_lock(workerSharedData->mut);
			workerSharedData->cond.wait(switch_lock, [&]() { return workerSharedData->ready == true; });	// wait for worker to finish
		}
		workerSharedData->ready = false; // reset ready flag
		workerSharedData->run = false;
		workerSharedData->cond.notify_one(); // wake up worker
	}
}
