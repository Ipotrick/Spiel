#pragma once
#include "PhysicsTypes.hpp"
#include "JobManager.hpp"
#include "EntityComponentManager.hpp"
#include "Physics.hpp"
#include "collision_detection.hpp"

class PushoutCalcJob : public JobFunctor {
	std::vector<IndexCollisionInfo>& collisionInfos;
	std::vector<Vec2>& velocities;
	std::vector<float>& overlaps;
	std::vector<CollisionResponse>& collisionResponses;
	EntityComponentManager& manager;

	void pushoutCalc(IndexCollisionInfo collInfo);
public:
	PushoutCalcJob(
		std::vector<IndexCollisionInfo>& collisionInfos,
		std::vector<Vec2>& velocities,
		std::vector<float>& overlaps,
		std::vector<CollisionResponse>& collisionResponses,
		EntityComponentManager& manager)
		:collisionInfos{ collisionInfos }, velocities{ velocities }, overlaps{ overlaps }, collisionResponses{ collisionResponses }, manager{ manager }
	{}

	int operator() (int workerId) override {
		for (auto& collInfo : collisionInfos) {
			pushoutCalc(collInfo);
		}
		return 0;
	}
};

void PushoutCalcJob::pushoutCalc(IndexCollisionInfo collInfo) {
	auto& collID = collInfo.indexA;
	if (manager.hasComps<PhysicsBody, Movement>(collID) && manager.hasComp<PhysicsBody>(collInfo.indexB)) {
		auto& otherID = collInfo.indexB;

		bool otherDynamic = false;
		if (manager.hasComps<Movement>(otherID)) otherDynamic = true;

		const float surfaceAreaColl = manager.getComp<Collider>(collID).size.x * manager.getComp<Collider>(collID).size.y;
		const float surfaceAreaOther = manager.getComp<Collider>(otherID).size.x * manager.getComp<Collider>(otherID).size.y;
		float dimColl = (manager.getComp<Collider>(collID).size.x + manager.getComp<Collider>(collID).size.y) / 2;
		float dimother = (manager.getComp<Collider>(otherID).size.x + manager.getComp<Collider>(otherID).size.y) / 2;
		float pushoutPriority = 1.0f;
		if (Physics::pressurebasedPositionCorrection) {
			if (overlaps[collID] > 0) {
				pushoutPriority = 2 * (overlaps[collID]/ dimColl) / ((overlaps[collID]/ dimColl) + (overlaps[otherID]/ dimother));
			}
		}
		Vec2 newPosChange = calcPosChange(
			surfaceAreaColl, velocities[collID],
			surfaceAreaOther, velocities[otherID],
			collInfo.clippingDist, collInfo.collisionNormal, otherDynamic, pushoutPriority);

		Vec2 oldPosChange = collisionResponses.at(collID).posChange;

		float weightOld = norm(oldPosChange);
		float weightNew = norm(newPosChange);
		float normalizer = weightOld + weightNew;
		if (normalizer > Physics::nullDelta) {
			collisionResponses.at(collID).posChange = (oldPosChange * weightOld / normalizer + newPosChange * weightNew / normalizer);
		}
	}
}
