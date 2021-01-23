#include "Game.hpp"

#include <iomanip>

#include "../engine/util/Log.hpp"
#include "GameComponents.hpp"
#include "serialization/YAMLSerializer.hpp"

#include "movementScript.hpp"
#include "LayerConstants.hpp"
#include "ParticleScript.hpp"
#include "SuckerScript.hpp"
#include "TesterScript.hpp"
#include "PlayerScript.hpp"
#include "HealthScript.hpp"

#include "BloomRScript.hpp"

#include "LoadBallTestMap.hpp"
#include "LoadRenderTestMap.hpp"
#include "Scripts.hpp"

static int makeID()
{
	static int nextID = 0;
	return nextID++;
}

using namespace util;

Game::Game()
{
	initialize("Balls2", 1600, 900);

	renderer.setLayerCount(LAYER_MAX);

	renderer.getLayer(LAYER_WORLD_BACKGROUND).bClearEveryFrame = true;
	renderer.getLayer(LAYER_WORLD_BACKGROUND).renderMode = RenderSpace::WorldSpace;

	renderer.getLayer(LAYER_WORLD_MIDGROUND).renderMode = RenderSpace::WorldSpace; 
	renderer.getLayer(LAYER_WORLD_MIDGROUND).depthTest = DepthTest::LessOrEqual;

	renderer.getLayer(LAYER_WORLD_PARTICLE).renderMode = RenderSpace::WorldSpace;

	renderer.getLayer(LAYER_WORLD_FOREGROUND).renderMode = RenderSpace::WorldSpace;

	renderer.getLayer(LAYER_WORLD_POSTPROCESS).renderMode = RenderSpace::WorldSpace;
	renderer.getLayer(LAYER_WORLD_POSTPROCESS).attachRenderScript(std::make_unique<BloomRScript>());

	renderer.getLayer(LAYER_DEBUG_UI).renderMode = RenderSpace::WorldSpace;

	renderer.getLayer(LAYER_FIRST_UI).renderMode = RenderSpace::PixelSpace;

	renderer.getLayer(LAYER_SECOND_UI).renderMode = RenderSpace::PixelSpace;

	collisionSystem.disableColliderDetection(Collider::PARTICLE);
}

void Game::create() {
	world.setOnRemCallback<Health>(onHealthRemCallback);

	auto size = getWindowSize();
	renderer.getCamera().frustumBend = (Vec2(1 / getWindowAspectRatio(), 1.0f));
	renderer.getCamera().zoom = 1 / 3.5f;

	const float firstRowWidth = 90.0f;
	const Vec2 textFieldSize{ firstRowWidth , 17.0f };
	const auto font = renderer.makeSmallTexRef(TextureDiscriptor("ConsolasAtlas2.png"));

#ifdef _DEBUG
	loadBallTestMap(*this);
#else
	std::ifstream ifstream("world.yaml");
	if (ifstream.good()) {
		YAMLWorldSerializer s(world);
		std::string str;
		std::getline(ifstream, str, '\0');
		s.deserializeString(str);
	}
#endif

}

void Game::update(float deltaTime) 
{
	if (bLoading) {
		if (JobSystem::finished(loadingWorkerTag)) {
			Monke::log("job with tag {0} finished clientside", (uint32_t)loadingWorkerTag);
			bLoading = false;
			world = loadedWorld;
		}
	}
	else {
		std::cout << "sprites last frame: " << renderer.getSpriteCountLastFrame() << std::endl;

		renderer.submit(makeSprite(0, { 0,0 }, -1.0f, { 2, 2 }, { 0.1, 0.1, 0.1f, 0.1f }, Form::Rectangle, RotaVec2{ 0 }, RenderSpace::WindowSpace), LAYER_WORLD_BACKGROUND);

		collisionSystem.execute(world.submodule<COLLISION_SECM_COMPONENTS>(), deltaTime);
		renderer.submit(collisionSystem.getDebugSprites(), LAYER_WORLD_FOREGROUND);
		physicsSystem2.execute(world.submodule<COLLISION_SECM_COMPONENTS>(), world.physics, deltaTime, collisionSystem);
		renderer.submit(physicsSystem2.getDebugSprites(), LAYER_WORLD_FOREGROUND);
		for (auto [ent, m, t] : world.entityComponentView<Movement, Transform>()) movementScript(*this, ent, t, m, deltaTime);
		gameplayUpdate(deltaTime);
		for (auto [ent, t, d] : world.entityComponentView<Transform, Draw>()) drawScript(*this, ent, t, d);
		world.update();
	}
}

void Game::gameplayUpdate(float deltaTime)
{
	if (mainWindow.keyPressed(Key::NP_6)) {
		world.physics.linearEffectDir = { 1, 0 };
		renderer.getCamera().rotation = 90;
	}
	if (mainWindow.keyPressed(Key::NP_8)) {
		world.physics.linearEffectDir = { 0, 1 };
		renderer.getCamera().rotation = 180;
	}
	if (mainWindow.keyPressed(Key::LEFT_ALT) && mainWindow.keyPressed(Key::F4)) {
		EngineCore::quit();
	}
	if (mainWindow.keyPressed(Key::G)) {
		world.physics.linearEffectAccel += 8 * deltaTime;
	}
	if (mainWindow.keyJustPressed(Key::P) && !mainWindow.keyPressed(Key::LEFT_SHIFT)) {
		renderer.getCamera().position = { 0,0 };
		world = World();
		loadRenderTestMap(*this, 0.5f);
	}
	if (mainWindow.keyJustPressed(Key::P) && mainWindow.keyPressed(Key::LEFT_SHIFT)) {
		renderer.getCamera().position = { 0,0 };
		world = World();
		loadRenderTestMap(*this, 0.3f);
	}
	if (mainWindow.keyPressed(Key::H)) {
		world.physics.linearEffectAccel -= 8 * deltaTime;
	}
	if (mainWindow.keyPressed(Key::UP)) {
		renderer.getCamera().position -= rotate(Vec2(0.0f, -5.0f), renderer.getCamera().rotation) * deltaTime;
	}
	if (mainWindow.keyPressed(Key::LEFT)) {
		renderer.getCamera().position -= rotate(Vec2(5.0f, 0.0f), renderer.getCamera().rotation) * deltaTime;
	}
	if (mainWindow.keyPressed(Key::DOWN)) {
		renderer.getCamera().position -= rotate(Vec2(0.0f, 5.0f), renderer.getCamera().rotation) * deltaTime;
	}
	if (mainWindow.keyPressed(Key::RIGHT)) {
		renderer.getCamera().position -= rotate(Vec2(-5.0f, 0.0f), renderer.getCamera().rotation) * deltaTime;
	}
	if (mainWindow.keyPressed(Key::NP_ADD)) {
		renderer.getCamera().zoom *= 1.0f + (1.0f * deltaTime);
	}
	if (mainWindow.keyPressed(Key::NP_SUBTRACT)) {
		renderer.getCamera().zoom *= 1.0f - (1.0f * deltaTime);
	}
	if (mainWindow.keyPressed(Key::NP_7)) {
		renderer.getCamera().rotation -= 100.0f * deltaTime;
	}
	if (mainWindow.keyPressed(Key::NP_9)) {
		renderer.getCamera().rotation += 100.0f * deltaTime;
	}
	if (mainWindow.keyPressed(Key::NP_0)) {
		renderer.getCamera().rotation = 0.0f;
		renderer.getCamera().position = { 0, 0 };
		renderer.getCamera().zoom = 1 / 5.0f;
		//uiContext.scale = 1.0f;
	}
	if (mainWindow.keyJustPressed(Key::B) && mainWindow.keyReleased(Key::LEFT_SHIFT)) {
		//if (ui.doesFrameExist("Statiscics")) {
		//	ui.getFrame("Statiscics").disable();
		//}
	}
	if (mainWindow.keyJustPressed(Key::B) && mainWindow.keyPressed(Key::LEFT_SHIFT)) {
		//if (ui.doesFrameExist("Statiscics")) {
		//	ui.getFrame("Statiscics").enable();
		//}
	}
	if (mainWindow.keyPressed(Key::PERIOD)) {
		renderer.getLayer(LAYER_WORLD_BACKGROUND).detachRenderScript();
	}
	if (mainWindow.keyPressed(Key::I)) {
		//uiContext.scale = clamp(uiContext.scale - deltaTime, 0.1f, 10.0f);
	}
	if (mainWindow.keyPressed(Key::O)) {
		//uiContext.scale = clamp(uiContext.scale + deltaTime, 0.1f, 10.0f);
	}
	if (mainWindow.keyPressed(Key::J)) {
		world = World();
		loadBallTestMap(*this);
		//ui.update();
	}
	if (mainWindow.keyJustPressed(Key::K)) {

		class SaveJob : public IJob {
		public:
			SaveJob(World w):
				w{ std::move(w) }
			{ }
			virtual void execute(const uint32_t thread) override
			{
				Monke::log("Start saving...");

				std::ofstream of("world.yaml");
				if (of.good()) {
					YAMLWorldSerializer s(w);
					of << s.serializeToString();
					of.close();
				}
				Monke::log("Finished saving!");
			}
		private:
			World w;
		};

		auto tag = JobSystem::submit(SaveJob(world));
		JobSystem::orphan(tag);
	}
	if (mainWindow.keyJustPressed(Key::L)) {
		bLoading = true;

		class LoadJob : public IJob {
		public:
			LoadJob(World& loadedWorld): loadedWorld{ loadedWorld } {}

			virtual void execute(uint32_t const thread) override
			{
				Monke::log("Start loading...");
				loadedWorld = World();
				std::ifstream ifstream("world.yaml");
				if (ifstream.good()) {
					YAMLWorldSerializer s(loadedWorld);
					std::string str;
					std::getline(ifstream, str, '\0');
					s.deserializeString(str);
				}
				Monke::log("Finished loading!");
			}
		private:
			World& loadedWorld;
		};

		loadingWorkerTag = JobSystem::submit(LoadJob(loadedWorld));

		//ui::frame("loadingtext",
		//	UIFrame::Parameters{
		//	.anchor = UIAnchor(UIAnchor::Parameters{
		//		.xmode = UIAnchor::X::LeftRelativeDist,
		//		.x = 0.5,
		//		.ymode = UIAnchor::Y::TopRelativeDist,
		//		.y = 0.5,}),
		//	.size = {1000, 300},
		//	.layer = LAYER_FIRST_UI,
		//	.borders = {10,10}},
		//{
		//	ui::text({
		//		.size = {1000, 300},
		//		.textAnchor = UIAnchor(UIAnchor::Parameters{
		//			.xmode = UIAnchor::X::LeftRelativeDist,
		//			.x = 0.5,
		//			.ymode = UIAnchor::Y::TopRelativeDist,
		//			.y = 0.5,}),
		//		.text = "Loading ...",
		//		.fontTexture = renderer.makeSmallTexRef(TextureDiscriptor("ConsolasAtlas.png")),
		//		.fontSize = {30,100}
		//	})
		//}
		//);
	}

	//execute scripts
	for (auto [ent, comp] : world.entityComponentView<Health>()) healthScript(*this, ent, comp, deltaTime);
	for (auto [ent, comp] : world.entityComponentView<Player>()) playerScript(*this, ent, comp, deltaTime);
	for (auto [ent, comp] : world.entityComponentView<Age>()) ageScript(*this, ent, comp, deltaTime);
	for (auto [ent, comp] : world.entityComponentView<Bullet>()) bulletScript(*this, ent, comp, deltaTime);
	for (auto [ent, comp] : world.entityComponentView<ParticleScriptComp>()) particleScript(*this, ent, comp, deltaTime);
	for (auto [ent, comp] : world.entityComponentView<SuckerComp>()) suckerScript(*this, ent, comp, deltaTime);
	for (auto [ent, comp] : world.entityComponentView<Tester>()) testerScript(*this, ent, comp, deltaTime);

	cursorManipFunc();

	//for (auto ent : world.entityView<SpawnerComp>()) {
	//	const Transform base = world.getComp<Transform>(ent);
	//	int laps = spawnerLapTimer.getLaps(deltaTime);
	//	for (int i = 1; i < laps; i++) {
	//		float rotation = (float)(rand() % 360);
	//		auto particle = world.create();
	//		Vec2 movement = rotate(Vec2(5,0), rotation);
	//		world.addComp<Transform>(particle, Transform(base.position));
	//		auto size = Vec2(0.36, 0.36) * ((rand() % 1000) / 1000.0f);
	//		float gray = (rand() % 1000 / 1000.0f);
	//		world.addComp<Draw>(particle, Draw(Vec4(gray, gray, gray, 0.3), size, rand() % 1000 / 1000.0f, Form::Circle));
	//		world.addComp<Movement>(particle, Movement(movement, rand()%10000/100.0f -50.0f));
	//		//world.addComp<Collider>(particle, Collider(size, Form::Circle, true));
	//		//world.addComp<PhysicsBody>(particle, PhysicsBody(1, 0.01, 10, 0));
	//		world.addComp<Age>(particle, Age(rand()%1000/2000.0f*3));
	//		world.spawn(particle);
	//	}
	//}

	for (auto ent : world.entityView<Movement, Transform>()) {
		auto pos = world.getComp<Transform>(ent).position;
		if (pos.length() > 1000)
			world.destroy(ent);
	}
}

void Game::destroy()
{
	//ui.destroyFrame("Statiscics");
	YAMLWorldSerializer s(world);
	auto str = s.serializeToString();
	std::ofstream ofs("world.yaml");
	ofs << str;
}

void Game::cursorManipFunc()
{
	Vec2 worldCoord = renderer.getCamera().windowToWorld(mainWindow.getCursorPos());
	Vec2 worldVel = (cursorData.oldPos - worldCoord) * getDeltaTimeSafe();
	Transform b = Transform(worldCoord, 0);
	Collider c = Collider({ 0.02,0.02 }, Form::Circle);
	//renderer.submit(
	//	Sprite(0, worldCoord, 2.0f, Vec2(0.02, 0.02) / renderer.getCamera().zoom, Vec4(1, 0, 0, 1), Form::Circle, RotaVec2(0), //RenderSpace::WorldSpace),
	//	LAYER_FIRST_UI
	//);
	if (!cursorData.locked && mainWindow.buttonPressed(MouseButton::MB_LEFT)) {
		std::vector<CollisionInfo> collisions;
		collisionSystem.checkForCollisions(collisions, Collider::DYNAMIC | Collider::SENSOR | Collider::STATIC | Collider::PARTICLE, b, c);
		if (!collisions.empty()) {
			EntityHandleIndex topEntity = collisions.front().indexB;
			EntityHandle id = world.getHandle(topEntity);
			cursorData.relativePos = world.getComp<Transform>(topEntity).position - worldCoord;
			cursorData.lockedID = id;
			cursorData.locked = true;
		}
	}
	else if (mainWindow.buttonPressed(MouseButton::MB_LEFT)) {
		if (world.isHandleValid(cursorData.lockedID)) {
			world.getComp<Transform>(cursorData.lockedID).position = cursorData.relativePos + worldCoord;
			if (world.hasComp<Movement>(cursorData.lockedID)) {
				world.getComp<Movement>(cursorData.lockedID).velocity = worldVel;
				world.getComp<Movement>(cursorData.lockedID).angleVelocity = 0;
			}
		}
		else {
			cursorData.locked = false;
		}
	}
	else {
		cursorData.locked = false;
	}

	cursorData.oldPos = worldCoord;


	//EntityHandleIndex cursor = world.getIndex(cursorID);
	//auto& baseCursor = world.getComp<Base>(cursor);
	//auto& colliderCursor = world.getComp<Collider>(cursor);
	//baseCursor.position = getPosWorldSpace(getCursorPos());
	//
	//baseCursor.rotation = renderer.getCamera().rotation;
	//colliderCursor.size = Vec2(1, 1) / renderer.getCamera().zoom / 100.0f;
	//
	//for (auto ent : world.entity_view<Player>()) {
	//	renderer.getCamera().position = world.getComp<Base>(ent).position;
	//}
	//
	////world.getComp<Draw>(cursorID).scale = vec2(1, 1) / renderer.getCamera().zoom / 100.0f;
	//if (buttonPressed(BUTTON::MB_LEFT)) {
	//	world.setStaticsChanged();
	//	if (cursorManipData.locked) {
	//		
	//		if (world.exists(cursorManipData.lockedID)) {
	//			if (world.hasComp<Movement>(cursorManipData.lockedID)) {
	//				auto& movControlled = world.getComp<Movement>(cursorManipData.lockedID);
	//				world.getComp<Movement>(cursorManipData.lockedID) = baseCursor.position - cursorManipData.oldCursorPos;
	//			}
	//			auto& baseControlled = world.getComp<Base>(cursorManipData.lockedID);
	//			auto& colliderControlled = world.getComp<Collider>(cursorManipData.lockedID);
	//			if (io.keyPressed(KEY::LEFT_SHIFT)) {	//rotate
	//				float cursorOldRot = getRotation(normalize(cursorManipData.oldCursorPos - baseControlled.position));
	//				float cursorNewRot = getRotation(normalize(baseCursor.position - baseControlled.position));
	//				float diff = cursorNewRot - cursorOldRot;
	//				baseControlled.rotation += diff;
	//				cursorManipData.lockedIDDist = baseControlled.position - baseCursor.position;
	//			}
	//			else if (io.keyPressed(KEY::LEFT_CONTROL)) {	//scale
	//				Vec2 ControlledEntRelativeCoordVec = rotate(Vec2(1, 0), baseControlled.rotation);
	//				Vec2 cursormovement = baseCursor.position - cursorManipData.oldCursorPos;
	//				float relativeXMovement = dot(cursormovement, ControlledEntRelativeCoordVec);
	//				if (dot(-cursorManipData.lockedIDDist, ControlledEntRelativeCoordVec) < 0) {
	//					relativeXMovement *= -1;
	//				}
	//				float relativeYMovement = dot(cursormovement, rotate(ControlledEntRelativeCoordVec, 90));
	//				if (dot(-cursorManipData.lockedIDDist, rotate(ControlledEntRelativeCoordVec, 90)) < 0) {
	//					relativeYMovement *= -1;
	//				}
	//				colliderControlled.size = colliderControlled.size + Vec2(relativeXMovement, relativeYMovement) * 2;
	//				world.getComp<Draw>(cursorManipData.lockedID).scale += Vec2(relativeXMovement, relativeYMovement) * 2;
	//				cursorManipData.lockedIDDist = baseControlled.position - baseCursor.position;
	//			}
	//			else {	//move
	//				baseControlled.position = baseCursor.position + cursorManipData.lockedIDDist;
	//			}
	//		}
	//	}
	//	else {
	//		std::optional<CollisionInfo> highestPrioColl;
	//		bool first = true;
	//		for (auto collision : collisionSystem.collisions_view(cursor)) {
	//			if (first) {
	//				highestPrioColl = collision;
	//			}
	//			first = false;
	//			if (world.getComp<Draw>(collision.indexB).drawingPrio > world.getComp<Draw>(highestPrioColl.value().indexB).drawingPrio) {	//higher drawprio found
	//				highestPrioColl = collision;
	//			}
	//		}
	//		if (highestPrioColl.has_value()) {
	//			cursorManipData.lockedID = highestPrioColl.value().indexB;
	//			cursorManipData.lockedIDDist = world.getComp<Base>(highestPrioColl.value().indexB).position - baseCursor.position;
	//			cursorManipData.locked = true;
	//		}
	//	}
	//
	//	if (io.keyPressed(KEY::DELETE) || io.keyPressed(KEY::BACKSPACE)) {
	//		if (cursorManipData.locked == true) {
	//			world.destroy(cursorManipData.lockedID);
	//		}
	//	}
	//}
	//else {
	//	cursorManipData.locked = false;
	//
	//	// spawns:
	//	if (io.keyPressed(KEY::U)) {
	//		Vec2 scale = Vec2(0.3f, 0.3f);
	//		Collider trashCollider = Collider(scale, Form::Circle);
	//		PhysicsBody trashSolidBody(0.0f, 1.0f, calcMomentOfIntertia(1, scale), 1.0f);
	//
	//		Vec2 position = baseCursor.position;
	//		// AFTER THIS LINE ALL REFERENCES TO COMPONENTS ARE GETTING INVALIDATED
	//		for (int i = 0; i < cursorManipData.ballSpawnLap.getLaps(getDeltaTime()); i++) {
	//			Vec4 color = Vec4(rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, rand() % 1000 / 1000.0f, 1);
	//			Draw trashDraw = Draw(color, scale, 0.5f, Form::Circle);
	//			auto trash = world.index_create();
	//			world.addComp<Base>(trash, Base(position, RotaVec2(0)));
	//			world.addComp<Movement>(trash, Movement());
	//			world.addComp<Collider>(trash, trashCollider);
	//			world.addComp<Draw>(trash, trashDraw);
	//			world.addComp<PhysicsBody>(trash, trashSolidBody);
	//			world.addComp<Health>(trash, Health(100));
	//			world.spawn(trash);
	//		}
	//	}
	//
	//	if (io.keyPressed(KEY::I)) {
	//		Vec2 scale = Vec2(0.5f, 0.5f);
	//		Collider trashCollider(scale, Form::Rectangle);
	//		PhysicsBody trashSolidBody(0.00f, 100000000000000000.f, calcMomentOfIntertia(100000000000000000.f, scale), 1.0f);
	//		Draw trashDraw = Draw(Vec4(1, 1, 1, 1), scale, 0.5f, Form::Rectangle);
	//
	//		for (int i = 0; i < cursorManipData.wallSpawnLap.getLaps(getDeltaTime()); i++) {
	//			auto trash = world.index_create();
	//			world.addComp<Base>(trash, Base(cursorManipData.oldCursorPos, 0));
	//			world.addComp<Collider>(trash, trashCollider);
	//			world.addComp<PhysicsBody>(trash, trashSolidBody);
	//			world.addComp<Draw>(trash, trashDraw);
	//			world.addComp<TextureRef>(trash, TextureRef(world.texture.getId("test.png"), Vec2(1.f / 16.f * 3.f, 1.f / 16.f * 15.f), Vec2(1.f / 16.f * 4.f, 1.f / 16.f * 16.f)));
	//			world.spawn(trash);
	//		}
	//	}
	//}
	//cursorManipData.oldCursorPos = getPosWorldSpace(getCursorPos());
}