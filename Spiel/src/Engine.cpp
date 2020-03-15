#include "Engine.h"

#include <windows.h>

Engine::Engine(std::string windowName_, uint32_t windowWidth_, uint32_t windowHeight_) :
	running{ true },
	iteration{ 0 },
	minimunLoopTime{ 40 },//10000 microseconds = 10 milliseond => 100 loops per second
	deltaTime{ 0.0 },
	mainTime{ 0.0 },
	updateTime{ 0.0 },
	physicsTime{ 0.0 },
	physicsPrepareTime{ 0.0 },
	physicsCollisionTime{ 0.0 },
	physicsExecuteTime{ 0.0 },
	renderBufferPushTime{ 0.0 },
	mainSyncTime{ 0.0 },
	mainWaitTime{ 0.0 },
	renderTime{ 0.0 },
	new_deltaTime{ 0 },
	new_mainTime{ 0 },
	new_updateTime{ 0 },
	new_physicsTime{ 0 },
	new_physicsPrepareTime{ 0 },
	new_physicsCollisionTime{ 0 },
	new_physicsExecuteTime{ 0 },
	new_renderTime{ 0 },
	new_renderBufferPushTime{ 0 },
	new_mainSyncTime{ 0 },
	new_mainWaitTime{ 0 },
	collInfos{},
	window{ std::make_shared<Window>(windowName_, windowWidth_, windowHeight_)},
	sharedRenderData{ std::make_shared<RendererSharedData>() },
	renderBufferA{},
	windowSpaceDrawables{},
	physicsThreadCount{ std::thread::hardware_concurrency() - 1},
	qtreeCapacity{ 10 }
{
	window->initialize();
	renderThread = std::thread(Renderer(sharedRenderData, window));
	renderThread.detach();
	SetThreadPriority(renderThread.native_handle(), -15);
	windowSpaceDrawables.reserve(50);

	sharedPhysicsData = std::vector<std::shared_ptr<PhysicsSharedData>>(physicsThreadCount);
	int n = 0;
	for (auto& el : sharedPhysicsData) {
		el = std::make_shared<PhysicsSharedData>();
		el->id = n++;
	}
	sharedPhysicsSyncData = std::make_shared<PhysicsSyncData>();
	sharedPhysicsSyncData->go = std::vector<bool>(physicsThreadCount);
	for (unsigned i = 0; i < physicsThreadCount; i++) {
		physicsThreads.push_back(std::thread(PhysicsWorker(sharedPhysicsData.at(i), sharedPhysicsSyncData, physicsThreadCount)));
		physicsThreads.at(i).detach();
		SetThreadPriority(physicsThreads.at(i).native_handle(), 15);
	}
}

Engine::~Engine() {
	{
		std::lock_guard<std::mutex> l(sharedRenderData->mut);
		sharedRenderData->run = false;
	}
	{
		std::lock_guard<std::mutex> l(sharedPhysicsSyncData->mut);
		sharedPhysicsSyncData->run = false;
	}
}

std::string Engine::getPerfInfo(int detail)
{
	std::stringstream ss;
	if (detail >= 4) ss << "Entities: " << world.entities.size() << "\n";
	ss << "deltaTime(s): " << deltaTime << " ticks/s: " << (1 / deltaTime) << " simspeed: " << getDeltaTimeSafe()/ deltaTime << '\n';
	if (detail >= 1) ss << "    mainTime(s): "   << mainTime << " mainSyncTime(s): " << mainSyncTime << " mainWaitTime(s): " << mainWaitTime <<'\n';
	if (detail >= 2) ss << "        update(s): " << updateTime    << " physics(s): " << physicsTime << " renderBufferPush(s): " << renderBufferPushTime << '\n';
	if (detail >= 3) ss << "            physicsPrepare(s): " << physicsPrepareTime << " physicsCollisionTime(s): " << physicsCollisionTime << " physicsExecuteTime(s): " << physicsExecuteTime << '\n';
	if (detail >= 1) ss << "    renderTime(s): " << renderTime << " renderSyncTime(s): " << renderSyncTime << '\n';

	return ss.str();
}

InputStatus Engine::getKeyStatus(KEY key_)
{
	std::lock_guard<std::mutex> l(window->mut);
	return (InputStatus)glfwGetKey(window->glfwWindow, int(key_));
}

bool Engine::keyPressed(KEY key_)
{
	return getKeyStatus(key_) == InputStatus::PRESS;
}

bool Engine::keyReleased(KEY key_)
{
	return getKeyStatus(key_) == InputStatus::RELEASE;
}

bool Engine::keyRepeating(KEY key_)
{
	return getKeyStatus(key_) == InputStatus::REPEAT;
}

vec2 Engine::getCursorPos()
{
	vec2 size = getWindowSize();
	std::lock_guard<std::mutex> l(window->mut);
	double xPos, yPos;
	glfwGetCursorPos(window->glfwWindow, &xPos, &yPos);
	return { (float)xPos / size.x * 2.0f - 1.f, -(float)yPos / size.y * 2.0f +1.f };
}

InputStatus Engine::getButtonStatus(BUTTON but_)
{
	std::lock_guard<std::mutex> l(window->mut);
	return static_cast<InputStatus>( glfwGetMouseButton(window->glfwWindow, static_cast<int>(but_)));
}

bool Engine::buttonPressed(BUTTON but_)
{
	return getButtonStatus(but_) == InputStatus::PRESS;
}

bool Engine::buttonReleased(BUTTON but_)
{
	return getButtonStatus(but_) == InputStatus::RELEASE;
}

vec2 Engine::getWindowSize()
{
	std::lock_guard<std::mutex> l(window->mut);
	return { static_cast<float>(window->width), static_cast<float>(window->height) };
}

float Engine::getWindowAspectRatio()
{
	std::lock_guard<std::mutex> l(window->mut);
	return static_cast<float>(window->width)/ static_cast<float>(window->height);
}

vec2 Engine::getPosWorldSpace(vec2 windowSpacePos_)
{
	auto transformedPos = mat4::translate(camera.position) * mat4::rotate_z(camera.rotation) * mat4::scale(vec2(1 / camera.frustumBend.x, 1/ camera.frustumBend.y)) * mat4::scale(1/camera.zoom) * vec4(windowSpacePos_.x, windowSpacePos_.y, 0, 1);
	return { transformedPos.x, transformedPos.y };
}

std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> Engine::getCollisionInfos(uint32_t id_)
{
	auto begin = collInfoBegins.find(id_);
	auto end = collInfoEnds.find(id_);
	if (begin != collInfoBegins.end() && end != collInfoEnds.end()) {	//is there even collisionInfo for the id?
		return { begin->second, end->second };
	}
	else {
		return { collInfos.end(), collInfos.end() };
	}
}

void Engine::commitTimeMessurements() {
	deltaTime = micsecToFloat(new_deltaTime);
	mainTime = micsecToFloat(new_mainTime);
	updateTime = micsecToFloat(new_updateTime);
	physicsTime = micsecToFloat(new_physicsTime);
	physicsPrepareTime = micsecToFloat(new_physicsPrepareTime);
	physicsCollisionTime = micsecToFloat(new_physicsCollisionTime);
	physicsExecuteTime = micsecToFloat(new_physicsExecuteTime);
	renderTime = micsecToFloat(new_renderTime);
	mainSyncTime = micsecToFloat(new_mainSyncTime);
	mainWaitTime = micsecToFloat(new_mainWaitTime);
	renderBufferPushTime = micsecToFloat(new_renderBufferPushTime);
	renderSyncTime = micsecToFloat(new_renderSyncTime);
}

void Engine::run() {
	create();

	while (running) {
		Timer<> loopTimer(new_deltaTime);
		Waiter<> loopWaiter(minimunLoopTime, Waiter<>::Type::BUSY, &new_mainWaitTime);
		commitTimeMessurements();
		glfwPollEvents();
		sharedRenderData->cond.notify_one();	//wake up rendering thread

		{
			Timer<> mainTimer(new_mainTime);
			{
				Timer<> t(new_updateTime);
				update(world, getDeltaTimeSafe());
				world.deregisterDespawnedEntities();
				world.executeDespawns();
			}
			{
				Timer<> t(new_physicsTime);
				physicsUpdate(world, getDeltaTimeSafe());
			}
			{
				Timer<> t(new_renderBufferPushTime);
				renderBufferA.camera = Camera();
				renderBufferA.windowSpaceDrawables.clear();
				renderBufferA.worldSpaceDrawables.clear();
				
				for (auto& d : windowSpaceDrawables) renderBufferA.windowSpaceDrawables.push_back(d);
				auto puffer = world.getDrawableVec();
				renderBufferA.worldSpaceDrawables.insert(renderBufferA.worldSpaceDrawables.end(), puffer.begin(), puffer.end());
				for (auto& d : worldSpaceDrawables) renderBufferA.worldSpaceDrawables.push_back(d);
				renderBufferA.camera = camera;
			
				windowSpaceDrawables.clear();
				worldSpaceDrawables.clear();
			}
		}
		
		{	
			Timer<> t(new_mainSyncTime);
			std::unique_lock<std::mutex> switch_lock(sharedRenderData->mut);
			sharedRenderData->cond.wait(switch_lock, [&]() { return sharedRenderData->ready == true; });	//wait for rendering thread to finish
			sharedRenderData->ready = false;																//reset renderers ready flag
			sharedRenderData->renderBufferB = renderBufferA;												//push Drawables and camera
			new_renderTime = sharedRenderData->new_renderTime;	//save render time
			new_renderSyncTime = sharedRenderData->new_renderSyncTime;
			// light data
			sharedRenderData->lightCollisions.clear();
			for (auto& light : world.lightCompCtrl.componentData) {
				auto [begin, end] = getCollisionInfos(light.first);
				for (auto iter = begin; iter != end; ++iter) {
					sharedRenderData->lightCollisions.push_back(*iter);
				}
			}
			sharedRenderData->lights = world.getLightVec();

			if (sharedRenderData->run == false) {
				running = false;
			}
		}

		iteration++;
	}

	destroy();
}

void Engine::physicsUpdate(World& world_, float deltaTime_)
{
	Timer<> t1(new_physicsPrepareTime);
	collInfos.clear();
	collInfos.reserve(world_.entities.size()); //~one collisioninfo per entity minumum capacity

	std::vector<std::pair<uint32_t, Collidable *>> dynCollidables;
	dynCollidables.reserve(world.entities.size());
	std::vector<std::pair<uint32_t, Collidable*>> statCollidables;
	statCollidables.reserve(world.entities.size());
	vec2 maxPos, minPos;
	if (world_.entities.size() > 0) {
		maxPos = world_.entities.at(0).getPos();
		minPos = maxPos;
	}
	for (auto& elHash : world_.entities) {
		auto & el = elHash.second;
		if (el.isDynamic()) {
			dynCollidables.push_back({ elHash.first, (Collidable*)&(elHash.second) });	//build dynamic collidable vector
		}
		else {
			statCollidables.push_back({ elHash.first, (Collidable*)&(elHash.second) });	//build static collidable vector
		}
		//look if the quadtree has to take uop largera area
		if (el.position.x < minPos.x) minPos.x = el.position.x;
		if (el.position.y < minPos.y) minPos.y = el.position.y;
		if (el.position.x > maxPos.x) maxPos.x = el.position.x;
		if (el.position.y > maxPos.y) maxPos.y = el.position.y;
	}
	
	std::vector<Quadtree> qtrees;
	qtrees.reserve(physicsThreadCount);
	for (unsigned i = 0; i < physicsThreadCount; i++) {
		vec2 randOffsetMin = { +(rand() % 2000 / 1000.0f) + 1, +(rand() % 2000 / 1000.0f) + 1 };
		vec2 randOffsetMax = { +(rand() % 2000 / 1000.0f) + 1, +(rand() % 2000 / 1000.0f) + 1 };
		qtrees.emplace_back(Quadtree(minPos - randOffsetMin, maxPos + randOffsetMax, qtreeCapacity));
	}
	/*
	//the random offset makes quadtree atrifacts go away
	vec2 randOffsetMin = { +(rand() % 2000 / 1000.0f) + 1, +(rand() % 2000 / 1000.0f) + 1 };
	vec2 randOffsetMax = { +(rand() % 2000 / 1000.0f) + 1, +(rand() % 2000 / 1000.0f) + 1 };
	//generate quadtree for fast lookup of nearby collidables
	Quadtree qtree(minPos - randOffsetMin, maxPos + randOffsetMax, qtreeCapacity);
	for (auto& el : world_.entities) {
		qtree.insert({ el.first, (Collidable*) &(el.second) });
	}*/
	std::vector<CollisionResponse> collisionResponses(dynCollidables.size());

	std::vector< robin_hood::unordered_map<uint32_t, CollisionResponse>> collisionResponsesOthers;
	for (unsigned i = 0; i < physicsThreadCount; i++) {
		collisionResponsesOthers.emplace_back(robin_hood::unordered_map<uint32_t, CollisionResponse>(dynCollidables.size()));
	}
	t1.stop();

	/* check for collisions */
	Timer<> t2(new_physicsCollisionTime);
	/* split the entities between threads */
	float splitStepDyn = (float)dynCollidables.size() / (float)(physicsThreadCount);
	std::vector<std::array<int ,2>> rangesDyn(physicsThreadCount);
	for (unsigned i = 0; i < physicsThreadCount; i++) {
		rangesDyn[i][0] = static_cast<int>(floorf(i * splitStepDyn));
		rangesDyn[i][1] = static_cast<int>(floorf((i + 1) * splitStepDyn));
	}
	float splitStepStat = (float)statCollidables.size() / (float)(physicsThreadCount);
	std::vector<std::array<int, 2>> rangesStat(physicsThreadCount);
	for (unsigned i = 0; i < physicsThreadCount; i++) {
		rangesStat[i][0] = static_cast<int>(floorf(i * splitStepStat));
		rangesStat[i][1] = static_cast<int>(floorf((i + 1) * splitStepStat));
	}

	//give physics workers their info
	std::vector<std::vector<CollisionInfo>> collisionInfosSplit(physicsThreadCount);
	for (unsigned i = 0; i < physicsThreadCount; i++) {
		auto& pData = sharedPhysicsData[i];
		pData->dynCollidables = &dynCollidables;
		pData->beginDyn = rangesDyn[i][0];
		pData->endDyn = rangesDyn[i][1];
		pData->statCollidables = &statCollidables;
		pData->beginStat = rangesStat[i][0];
		pData->endStat = rangesStat[i][1];
		pData->collisionInfos = &collisionInfosSplit[i];
		pData->collisionResponses = &collisionResponses;
		pData->collisionResponsesOthers = &collisionResponsesOthers;
		pData->qtrees = &qtrees;
		pData->deltaTime = deltaTime_;
	}

	{	// start physics threads
		std::unique_lock switch_lock(sharedPhysicsSyncData->mut);
		for (unsigned i = 0; i < physicsThreadCount; i++) {
			sharedPhysicsSyncData->go.at(i) = true;
		}
		sharedPhysicsSyncData->cond.notify_all();
		//wait for physics threads to finish
		sharedPhysicsSyncData->cond.wait(switch_lock, [&]() { 
			/* wenn alle false sind wird true returned */
			for (unsigned i = 0; i < physicsThreadCount; i++) {
				if (sharedPhysicsSyncData->go.at(i) == true) {
					return false;
				}
			}
			return true; 
			}
		);
	}
	t2.stop();

	Timer<> t3(new_physicsExecuteTime);
	//store all collisioninfos in one vector
	for (auto collInfoSplit : collisionInfosSplit) {
		collInfos.insert(collInfos.end(), collInfoSplit.begin(), collInfoSplit.end());
	}

	//build hastable for first and last iterator element of collisioninfo
	collInfoBegins.clear();
	collInfoEnds.clear();
	uint32_t lastIDA{};
	for (auto iter = collInfos.begin(); iter != collInfos.end(); ++iter) {
		if (iter == collInfos.begin()) {	//initialize values from first element
			lastIDA = iter->idA;
			collInfoBegins.insert({iter->idA, iter});
		}
		if (lastIDA != iter->idA) {	//new idA found
			collInfoEnds.insert({lastIDA, iter});
			collInfoBegins.insert({iter->idA, iter});
			lastIDA = iter->idA;	//set lastId to new id
		}
	}
	collInfoEnds.insert({ lastIDA, collInfos.end() });

	//execute physics changes in pos, vel, accel
	int i = 0;
	for (auto& coll : dynCollidables) {
		coll.second->velocity += collisionResponses.at(i).velChange;
		coll.second->position += collisionResponses.at(i).posChange + coll.second->velocity * deltaTime_;
		coll.second->collided |= collisionResponses.at(i).collided;
		coll.second->position += coll.second->velocity * deltaTime_;
		coll.second->acceleration = 0;
		i++;
	}
	t3.stop();

	/* submit debug drawables for physics */
	for (auto& el : Physics::debugDrawables) {
		submitDrawableWorldSpace(el);
	}
	Physics::debugDrawables.clear();
}