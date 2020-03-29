#include "PhysicsWorker.h"
#include <algorithm>
#include <random>

void PhysicsWorker::operator()()
{
	auto& world = *physicsPoolData->world;
	auto& beginDyn = physicsData->beginDyn;
	auto& endDyn = physicsData->endDyn;
	auto& dynCollidables = physicsPoolData->dynCollidables;
	auto& beginStat = physicsData->beginStat;
	auto& endStat = physicsData->endStat;
	auto& statCollidables = physicsPoolData->statCollidables;
	auto& collisionResponses = physicsPoolData->collisionResponses;
	auto& collisionInfos = physicsData->collisionInfos;
	auto& qtreesDynamic = physicsPoolData->qtreesDynamic;
	auto& qtreesStatic = physicsPoolData->qtreesStatic;

	while (true) {
		{
			std::unique_lock<std::mutex> switch_lock(syncData->mut);
			syncData->go.at(physicsData->id) = false;
			syncData->cond.notify_all();
			syncData->cond.wait(switch_lock, [&]() { return syncData->go.at(physicsData->id) == true; });	//wait for engine to give go sign
			if (syncData->run == false) break;
		}

		// rebuild dyn qtree
		if (physicsPoolData->rebuildDynQuadTrees) {
			for (int i = beginDyn; i < endDyn; i++) {
				if (!(*dynCollidables)[i].second->isParticle()) {	//never check for collisions against particles
					qtreesDynamic->at(physicsData->id).insert((*dynCollidables)[i]);
				}
			}
		}
		// rebuild stat qtree
		if (physicsPoolData->rebuildStatQuadTrees) {
			for (int i = beginStat; i < endStat; i++) {
				if (!(*statCollidables)[i].second->isParticle()) {	//never check for collisions against particles
					qtreesStatic->at(physicsData->id).insert((*statCollidables)[i]);
				}
			}
		}
		
		// re sync with others after inserting (building trees)
		{
			std::unique_lock<std::mutex> switch_lock(syncData->mut2);
			syncData->insertReady++;
			if (syncData->insertReady == physicsThreadCount) {
				syncData->insertReady = 0;
				syncData->cond2.notify_all();
			}
			else {
				syncData->cond2.wait(switch_lock, [&]() { return syncData->insertReady == 0; });
			}
		}

		collisionInfos->reserve(dynCollidables->size() / 10.f);	//try to avoid reallocations

		std::vector<std::pair<uint32_t, Collidable*>> nearCollidables;	//reuse heap memory for all dyn collidable collisions
		for (int i = beginDyn; i < endDyn; i++) {
			auto& coll = dynCollidables->at(i);
			(*collisionResponses)[coll.first].posChange = vec2(0, 0);
			nearCollidables.clear();

			// querry dynamic entities
			for (int i = 0; i < physicsThreadCount; i++) {
				qtreesDynamic->at(i).querry(nearCollidables, coll.second->getPos(), coll.second->getBoundsSize());
			}

			// querry static entities
			for (int i = 0; i < physicsThreadCount; i++) {
				qtreesStatic->at(i).querry(nearCollidables, coll.second->getPos(), coll.second->getBoundsSize());
			}

			//check for collisions and save the changes in velocity and position these cause
			for (auto& other : nearCollidables) {
				//do not check against self or slave/owner owner/slave
				if (coll.first != other.first && ((!coll.second->isSlave() && !other.second->isSlave()) || (coll.second->getOwnerID() != other.first && other.second->getOwnerID() != coll.first))) {

					auto newTestResult = checkForCollision(coll.second, other.second, coll.second->isSolid() && other.second->isSolid());

					if (newTestResult.collided) {
						collisionInfos->push_back(CollisionInfo(coll.first, other.first, newTestResult.clippingDist, newTestResult.collisionNormal, newTestResult.collisionPos));
						//take average of pushouts with weights
						float weightOld = norm((*collisionResponses)[coll.first].posChange);
						float weightNew = norm(newTestResult.posChange);
						float normalizer = weightOld + weightNew;
						if (normalizer > Physics::nullDelta) {
							(*collisionResponses)[coll.first].posChange = ((*collisionResponses)[coll.first].posChange * weightOld / normalizer + newTestResult.posChange * weightNew / normalizer);
						}
					}
				}
			}
		}
	}
}
