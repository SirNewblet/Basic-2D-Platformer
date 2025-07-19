#pragma once

#include "Entity.h"

#include <vector>
#include <map>

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::tuple< std::vector<CTransform>, std::vector<CLifespan>, std::vector<CInput>, std::vector<CBoundingBox>, std::vector<CAnimation>, std::vector<CGravity>, std::vector<CState>, std::vector<CDraggable>> EntityComponentVecTuple;

class EntityManager
{
	EntityVec										m_entities;					// All entities
	EntityVec										m_entitiesToAdd;			// Entities to add next update
	std::map<std::string, EntityVec>				m_entityMap;				// Map from entity tag to vectors
	size_t											m_totalEntities = 0;		// Total entities created

	// Helper function to avoid repeated code
	void removeDeadEntities(EntityVec& vec);

public:

	EntityManager();

	void update();

	std::shared_ptr<Entity> addEntity(const std::string& tag);

	const EntityVec& getEntities();
	const EntityVec& getEntities(const std::string& tag);

};