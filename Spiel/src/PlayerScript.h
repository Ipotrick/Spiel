#pragma once

#include "Script.h"
#include "Engine.h"


class PlayerScript : public ScriptController<CompDataPlayer, CompController< CompDataPlayer>> {
public:
	PlayerScript(CompController< CompDataPlayer>& cmpCtrl_, Engine& engine_) : ScriptController<CompDataPlayer, CompController< CompDataPlayer>>(cmpCtrl_, engine_) {}

	inline void executeSample(uint32_t id, CompDataPlayer& data, World& world, float deltaTime) override {
		auto* entity = world.getEntityPtr(id);

		if (engine.keyPressed(KEY::W)) {
			//entity->acceleration += rotate(vec2(0.0f, 10.0f), entity->rotation);
			entity->velocity += rotate(vec2(0.0f, 10.0f), entity->rotation) * deltaTime;
		}
		if (engine.keyPressed(KEY::A)) {
			//entity->acceleration += rotate(vec2(-10.0f, 0.0f), entity->rotation);
			entity->velocity += rotate(vec2(-10.0f, 0.0f), entity->rotation) * deltaTime;
		}
		if (engine.keyPressed(KEY::S)) {
			//entity->acceleration += rotate(vec2(0.0f, -10.0f), entity->rotation);
			entity->velocity += rotate(vec2(0.0f, -10.0f), entity->rotation) * deltaTime;
		}
		if (engine.keyPressed(KEY::D)) {
			//entity->acceleration += rotate(vec2(10.0f, 0.0f), entity->rotation);
			entity->velocity += rotate(vec2(10.0f, 0.0f), entity->rotation) * deltaTime;
		}
		if (engine.keyPressed(KEY::Q)) {
			entity->angleVelocity += 700.0f * deltaTime;
		} 
		else if (engine.keyPressed(KEY::E)) {
			entity->angleVelocity -= 700.0f * deltaTime;
		}
		else {

		}
		if (engine.keyPressed(KEY::F)) {
			float scale = rand() % 10 * 0.1f + 0.5f;
			vec2 bulletSize = vec2(0.05f, 0.05f) * scale;
			float bulletVel = 10.0f;
			float velOffsetRota = rand() % 20000 / 1000.0f - 10.0f;
			uint64_t bullets = data.bulletShotLapTimer.getLaps(deltaTime);
			for (uint64_t i = 0; i < bullets; i++) {
				Entity bullC = Entity(entity->getPos() + rotate(vec2(-entity->getSize().y, 0) / 1.9f, entity->rotation + 270), 0, Collidable(bulletSize, Form::CIRCLE, true, true, entity->velocity + bulletVel * rotate(vec2(0, 1), entity->rotation + velOffsetRota)));
				CompDataDrawable bullD = CompDataDrawable(vec4(0.f, 1.f, 0.f, 1), bulletSize, 0.4f, Form::CIRCLE);
				world.spawnSolidEntity(bullC, CompDataSolidBody(1, 0.001f) ,bullD);
				world.bulletCompCtrl.registerEntity(world.getLastID(), CompDataBullet(10 * scale));
			}
		}
	}
	inline void executeMeta(World& world, float deltaTime) {

	}
};