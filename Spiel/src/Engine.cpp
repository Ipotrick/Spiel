#include "Engine.h"


Engine::Engine(World& wrld, std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_) :
	world{ wrld },
	running{ true },
	iteration{ 0 },
	minimunLoopTime{ 100 }, // 10000 microseconds = 10 milliseond => 100 loops per second
	maxDeltaTime{0.02f},
	deltaTime{ 0.0 },
	window{ std::make_shared<Window>(windowName_, windowWidth_, windowHeight_)},
	baseSystem( wrld ),
	physicsSystem{ world, std::thread::hardware_concurrency() - 1 , perfLog},
	renderer{ window }
{
	perfLog.submitTime("maintime");
	perfLog.submitTime("mainwait");
	perfLog.submitTime("updatetime");
	perfLog.submitTime("physicstime");
	perfLog.submitTime("physicsprepare");
	perfLog.submitTime("physicscollide");
	perfLog.submitTime("physicsexecute");
	perfLog.submitTime("rendertime");
	perfLog.submitTime("calcRotaVecTime");
}

Engine::~Engine() {
	renderer.end();
	physicsSystem.end();
}

std::string Engine::getPerfInfo(int detail) {
	std::stringstream ss;
	if (detail >= 4) ss << "Entity Max: " << world.memorySize() << "\n";
	if (detail >= 1) ss << "Entity Count: " << world.entityCount() << "\n";
	if (detail >= 1) {
		ss << "    deltaTime(s): " << perfLog.getTime("maintime") << "\n"
			<< "    Ticks/s: " << 1 / perfLog.getTime("maintime") << "\n"
			<< "    simspeed: " << getDeltaTimeSafe() / getDeltaTime() << '\n';
	}
	if (detail >= 2) {
		ss << "        update(s): " << perfLog.getTime("updatetime") << "\n"

			<< "        physics(s): " << perfLog.getTime("physicstime") << '\n';
	}
	if (detail >= 3) {
		ss << "            physicsPrepare(s): " << perfLog.getTime("physicsprepare") << '(' << floorf(perfLog.getTime("physicsprepare") / perfLog.getTime("physicstime") * 10000.0f) * 0.01f << "%)\n"
			<< "            physicsCollisionTime(s): " << perfLog.getTime("physicscollide") << '(' << floorf(perfLog.getTime("physicscollide") / perfLog.getTime("physicstime") * 10000.0f) * 0.01f << "%)\n"
			<< "            physicsExecuteTime(s): " << perfLog.getTime("physicsexecute") << '(' << floorf(perfLog.getTime("physicsexecute") / perfLog.getTime("physicstime") * 10000.0f) * 0.01f << "%)" << '\n';
	}
	if (detail >= 1) ss << "    renderTime(s): " << perfLog.getTime("rendertime") << '\n';

	return ss.str();
}

InputStatus Engine::getKeyStatus(KEY key_) {
	std::lock_guard<std::mutex> l(window->mut);
	return (InputStatus)glfwGetKey(window->glfwWindow, int(key_));
}

bool Engine::keyPressed(KEY key_) {
	return getKeyStatus(key_) == InputStatus::PRESS;
}

bool Engine::keyReleased(KEY key_) {
	return getKeyStatus(key_) == InputStatus::RELEASE;
}

bool Engine::keyRepeating(KEY key_) {
	return getKeyStatus(key_) == InputStatus::REPEAT;
}

Vec2 Engine::getCursorPos() {
	Vec2 size = getWindowSize();
	std::lock_guard<std::mutex> l(window->mut);
	double xPos, yPos;
	glfwGetCursorPos(window->glfwWindow, &xPos, &yPos);
	return { (float)xPos / size.x * 2.0f - 1.f, -(float)yPos / size.y * 2.0f +1.f };
}

InputStatus Engine::getButtonStatus(BUTTON but_) {
	std::lock_guard<std::mutex> l(window->mut);
	return static_cast<InputStatus>( glfwGetMouseButton(window->glfwWindow, static_cast<int>(but_)));
}

bool Engine::buttonPressed(BUTTON but_) {
	return getButtonStatus(but_) == InputStatus::PRESS;
}

bool Engine::buttonReleased(BUTTON but_) {
	return getButtonStatus(but_) == InputStatus::RELEASE;
}

Vec2 Engine::getWindowSize() {
	std::lock_guard<std::mutex> l(window->mut);
	return { static_cast<float>(window->width), static_cast<float>(window->height) };
}

float Engine::getWindowAspectRatio() {
	std::lock_guard<std::mutex> l(window->mut);
	return static_cast<float>(window->width)/ static_cast<float>(window->height);
}

Vec2 Engine::getPosWorldSpace(Vec2 windowSpacePos_) {
	auto transformedPos = Mat3::translate(camera.position) * Mat3::rotate(camera.rotation) * Mat3::scale(Vec2(1 / camera.frustumBend.x, 1/ camera.frustumBend.y)) * Mat3::scale(1/camera.zoom) * Vec3(windowSpacePos_.x, windowSpacePos_.y, 1);
	return { transformedPos.x, transformedPos.y };
}

std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> Engine::getCollisions(entity_handle entity) {
	return physicsSystem.getCollisions(entity);
}

GridPhysics<bool> const& Engine::getStaticGrid()
{
	return physicsSystem.getStaticGrid();
}

void Engine::run() {
	create();

	while (running) {
		Timer loopTimer(new_deltaTime);
		Waiter<> loopWaiter(minimunLoopTime, Waiter<>::Type::BUSY);
		deltaTime = micsecToFloat(new_deltaTime);
		perfLog.commitTimes();
		//commitTimeMessurements();
		freeDrawableID = 0x80000000;
		{
			Timer mainTimer(perfLog.getInputRef("maintime"));
			{
				Timer t(perfLog.getInputRef("updatetime"));
				update(world, getDeltaTimeSafe());
				world.tick();
			}
			{
				Timer t(perfLog.getInputRef("physicstime"));
				physicsSystem.execute(getDeltaTimeSafe());
				for (auto& d : physicsSystem.debugDrawables) submitDrawable(d);
			}
			{
				Timer t(perfLog.getInputRef("calcRotaVecTime"));
				baseSystem.execute();
			}
			{
				rendererUpdate(world);
			}
		}
		if (glfwWindowShouldClose(window->glfwWindow)) { // if window closes the program ends
			running = false;
			renderer.end();
		}
		// window access begin
		{
			std::lock_guard l(window->mut);
			glfwPollEvents();
		}
		// window access end
		renderer.startRendering();
		// reset flags
		world.setStaticsChanged(false);
		iteration++;
	}
	renderer.end();

	destroy();
}

Drawable buildWorldSpaceDrawable(World& world, entity_handle entity) {
	return std::move(Drawable(entity, world.getComp<Base>(entity).position, world.getComp<Draw>(entity).drawingPrio, world.getComp<Draw>(entity).scale, world.getComp<Draw>(entity).color, world.getComp<Draw>(entity).form, world.getComp<Base>(entity).rotaVec));
}

void Engine::rendererUpdate(World& world)
{
	for (auto ent : world.view<Base,Draw>()) {
		renderer.submit(buildWorldSpaceDrawable(world, ent));
	}
	for (auto ent : world.view<TextureRef>()) {
		renderer.attachTex(ent, world.getComp<TextureRef>(ent));
	}
	renderer.setCamera(camera);

	renderer.waitTillFinished();
	renderer.flushSubmissions();

	perfLog.submitTime("rendertime",renderer.getRenderingTime());
}
