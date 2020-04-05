#pragma once
#include <algorithm>
#include <fstream>
 
#include <vector>
#include <queue>

#include "robin_hood.h"
#include "json.h"

#include "BaseTypes.h"
#include "RenderTypes.h"
#include "ECS.h"
#include "CoreComponents.h"
#include "GameComponents.h"

#define GENERATE_COMPONENT_ACCESS_FUNCTIONS_INTERN(CompType, CompStorage, storageType) \
template<> inline auto& getAll<CompType>() { return CompStorage; } \
template<> inline CompType& getComp<CompType>(uint32_t id) { return CompStorage.getComponent(id); }\
template<> inline bool hasComp<CompType>(uint32_t id) { return CompStorage.isRegistrated(id); }\
template<> inline void addComp<CompType>(uint32_t id, CompType data) { CompStorage.registrate(id, data); } \
template<> inline void addComp<CompType>(uint32_t id) { CompStorage.registrate(id, CompType()); }

#define generateComponentAccessFunctionsExtern(CompType, CompStorage, storageType) \
template<> inline auto& World::getAll<CompType>() { return CompStorage; } \
template<> inline CompType& World::getComp<CompType>(uint32_t id) { return CompStorage.getComponent(id); }\
template<> inline bool World::hasComp<CompType>(uint32_t id) { return CompStorage.isRegistrated(id); }\
template<> inline void World::addComp<CompType>(uint32_t id, CompType data) { CompStorage.registrate(id, data); } \
template<> inline void World::addComp<CompType>(uint32_t id) { CompStorage.registrate(id, CompType()); }

#define GENERATE_COMPONENT_CODE(CompType, StorageType, Num) \
private: ComponentStorage<CompType, StorageType> compStorage ## Num; \
public: GENERATE_COMPONENT_ACCESS_FUNCTIONS_INTERN(CompType, compStorage ## Num, storageType)

struct Ent {
	Ent(bool valid_ = false) : valid{ valid_ }, despawnQueued{ false } {}
	bool valid;
	bool despawnQueued;
};

template<typename First, typename Second, typename ... CompTypes>
class MultiView;
template<typename CompType>
class SingleView;

class World {
public:

	World() : lastID{ 0 }, despawnList{}
	{
		entities.push_back({ false });
	}
	
	/* returnes if entitiy exists or not, O(1) */
	bool doesEntExist(ent_id_t id);
	/* entity create/destruct utility */
	/* creates blank entity and returns its id, O(1) */
	ent_id_t createEnt();
	/* enslaves the first ent to the second, ~O(1) */
	void enslaveEntTo(ent_id_t slave, ent_id_t owner, vec2 relativePos, float relativeRota);
	/* marks entity for deletion, entities are deleted after each update, O(1) */
	void despawn(ent_id_t id);

	/* world utility */
	/* returnes the id of the most rescently spawned entity.
		if 0 is returnsed, there are no entities spawned yet.
		Try to not use this function and use the return value of create() instead
		, O(1) */
	ent_id_t const getLastEntID();
	/* returns count of entities, O(1) */
	size_t const getEntCount();
	/* returns the size of the vector that holds the entities, O(1) */
	size_t const getEntMemSize();

	/* Component access utility */
	/* returnes reference to a safe virtual container of the given components one can iterate over. the iterator also holds the entity id of the compoenent it points to, O(1) */
	template<typename CompType> auto& getAll();
	/* returnes refference the component data of one entitiy, ~O(1) */
	template<typename CompType> CompType& getComp(ent_id_t id);
	/* returns bool whether or not the given entity has the component added/registered, ~O(1) */
	template<typename CompType> bool hasComp(ent_id_t id);
	template<typename ... CompTypes> bool hasComps(ent_id_t entity);
	/* registeres a new component under the given id, ~O(1) */
	template<typename CompType> void addComp(ent_id_t id, CompType data);
	/*registeres a new component under the given id, ~O(1) */
	template<typename CompType> void addComp(ent_id_t id);
	/* returnes a View, and iterable object that only iterates over the entities with the given Components */
	template<typename First, typename Second, typename ... CompTypes> MultiView<First, Second, CompTypes...> view();
	template<typename CompType> SingleView<CompType> view();

	void loadMap(std::string);
private:
	GENERATE_COMPONENT_CODE(Base, storage_index_t, 0)
	GENERATE_COMPONENT_CODE(Movement, storage_index_t, 1)
	GENERATE_COMPONENT_CODE(Collider, storage_index_t, 2)
	GENERATE_COMPONENT_CODE(SolidBody, storage_index_t, 3)
	GENERATE_COMPONENT_CODE(Draw, storage_index_t, 4)
	GENERATE_COMPONENT_CODE(Slave, storage_index_t, 5)
	GENERATE_COMPONENT_CODE(Composit<4>, storage_hash_t, 6)
	GENERATE_COMPONENT_CODE(CompDataLight, storage_hash_t, 7)
	GENERATE_COMPONENT_CODE(Health, storage_hash_t, 8)
	GENERATE_COMPONENT_CODE(Age, storage_hash_t, 9)
	GENERATE_COMPONENT_CODE(Player, storage_hash_t, 10)
	GENERATE_COMPONENT_CODE(Bullet, storage_hash_t, 11)
	GENERATE_COMPONENT_CODE(Enemy, storage_hash_t, 12)
private:
	
private:
	/* INNER ENGINE FUNCTIONS: */
	friend class Engine;
	void slaveOwnerDespawn(); // slaves with dead owner get despawned, dead slaves cut their refference of themselfes to the owner
	void deregisterDespawnedEntities();	// CALL BEFORE "executeDespawns"
	void executeDespawns();
	Drawable buildDrawable(ent_id_t id, Draw const& draw);
	std::vector<Drawable> getDrawableVec();
private:
	std::vector<Ent> entities;
	std::queue<ent_id_t> emptySlots;
	ent_id_t lastID;
	std::vector<ent_id_t> despawnList;
	bool staticSpawnOrDespawn{ false };
};

// ---------- hasComps implementation --------------------------------------

namespace {
	template<typename... CompTypes>
	struct HasCompsTester {
		HasCompsTester(ent_id_t entity, World& world) {
			result = true;
		}
		bool result;
	};
	template<typename Head, typename... CompTypes>
	struct HasCompsTester<Head, CompTypes...> {
		HasCompsTester(ent_id_t entity, World& world) {
			if (world.hasComp<Head>(entity)) {
				HasCompsTester<CompTypes...> recursiveTester(entity, world);
				result = recursiveTester.result;
			}
			else {
				result = false;
			}
		}
		bool result;
	};
}

template<typename... CompTypes>
inline bool World::hasComps(ent_id_t entity) {
	HasCompsTester<CompTypes...> tester(entity, *this);
	return tester.result;
}

// ------------ view implementation ----------------------------------------

template<typename First, typename Second, typename ... CompTypes>
class MultiView {
public:
	MultiView(World& wrld) : world{ wrld }, endID{ static_cast<ent_id_t>(world.getEntMemSize()) } {}
	template<typename First, typename Second, typename ... CompTypes>
	class iterator {
	public:
		typedef iterator<First, Second, CompTypes...> self_type;
		typedef ent_id_t value_type;
		typedef ent_id_t& reference;
		typedef ent_id_t* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(ent_id_t ent, MultiView& vw) : entity{ ent }, view{ vw } {}
		inline self_type operator++(int junk) {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			entity++;
			while (!(view.world.hasComp<First>(entity) && view.world.hasComp<Second>(entity) && view.world.hasComps<CompTypes...>(entity)) && entity < view.endID) entity++;
			assert(entity <= view.endID);
			return *this;
		}
		inline self_type operator++() {
			auto oldme = *this;
			operator++(0);
			return oldme;
		}
		inline reference operator*() {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			return entity;
		}
		inline pointer operator->() {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			return &entity;
		}
		inline bool operator==(const self_type& rhs) {
			return entity == rhs.entity;
		}
		inline bool operator!=(const self_type& rhs) {
			return entity != rhs.entity;
		}
	private:
		ent_id_t entity;
		MultiView& view;
	};
	inline iterator<First, Second, CompTypes...> begin() {
		ent_id_t entity = 1;
		while (!(world.hasComp<First>(entity) && world.hasComp<Second>(entity) && world.hasComps<CompTypes...>(entity)) && entity < endID) entity++;
		return iterator<First, Second, CompTypes...>(std::min(entity, endID), *this);
	}
	inline iterator<First, Second, CompTypes...> end() { 
		return iterator<First, Second, CompTypes...>(endID, *this);
	}
private:
	World& world;
	ent_id_t endID;
};

template<typename First, typename Second, typename ... CompTypes>
inline MultiView<First, Second, CompTypes...> World::view() {
	return MultiView<First, Second, CompTypes...>(*this);
}

template<typename CompType>
class SingleView {
public:
	SingleView(World& wrld) : world{ wrld }, endID{ static_cast<ent_id_t>(world.getEntMemSize()) } {}
	template<typename CompType>
	class iterator {
	public:
		typedef iterator<CompType> self_type;
		typedef ent_id_t value_type;
		typedef ent_id_t& reference;
		typedef ent_id_t* pointer;
		typedef std::forward_iterator_tag iterator_category;

		iterator(ent_id_t ent, SingleView& vw) : entity{ ent }, view{ vw } {}
		inline self_type operator++(int junk) {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			entity++;
			while (!view.world.hasComp<CompType>(entity) && entity < view.endID) entity++;
			assert(entity <= view.endID);
			return *this;
		}
		inline self_type operator++() {
			auto oldme = *this;
			operator++(0);
			return oldme;
		}
		inline reference operator*() {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			return entity;
		}
		inline pointer operator->() {
			assert(entity < view.endID);
			assert(view.world.doesEntExist(entity));
			return &entity;
		}
		inline bool operator==(const self_type& rhs) {
			return entity == rhs.entity;
		}
		inline bool operator!=(const self_type& rhs) {
			return entity != rhs.entity;
		}
	private:
		ent_id_t entity;
		SingleView& view;
	};
	inline iterator<CompType> begin() {
		ent_id_t entity = 1;
		while (!world.hasComps<CompType>(entity) && entity < endID) entity++;	// TODO: replace getEntMemSize with component specific sizes
		return iterator<CompType>(std::min(entity, endID), *this);
	}
	inline iterator<CompType> end() { return iterator<CompType>(endID, *this); }
private:
	World& world;
	ent_id_t endID;
};

template<typename CompType>
inline SingleView<CompType> World::view() {
	return SingleView<CompType>(*this);
}

// -------------------------------------------------------------------------

inline bool World::doesEntExist(ent_id_t entity) {
	return (entity < entities.size() ? entities[entity].valid : false);
}

inline ent_id_t const World::getLastEntID() {
	return lastID;
}