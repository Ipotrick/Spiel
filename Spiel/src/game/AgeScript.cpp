#include "AgeScript.hpp"

void ageScript(EntityHandle id, Age& data, float deltaTime) {

	data.curAge += deltaTime;

	if (data.curAge > data.maxAge) {
		Game::world.destroy(id);
	}
}