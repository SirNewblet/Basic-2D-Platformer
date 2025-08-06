#pragma once

#include "Entity.h"
#include "Components.h"
#include "EntityMemoryPool.h"

#include <vector>
#include <map>

typedef std::vector<Entity> EntityVec;

class EntityManager
{
	EntityVec										m_tagged;					// For returning multiple tags
	EntityVec										m_entities;					// All entities
	EntityVec										m_entitiesToAdd;			// Entities to add next update
	std::map<std::string, EntityVec>				m_entityMap;				// Map from entity tag to vectors
	size_t											m_totalEntities = 0;		// Total entities created

	// Helper function to avoid repeated code
	void removeDeadEntities(EntityVec& vec);

public:
	EntityManager();

	void update();

	Entity addEntity(const std::string& tag);

	const EntityVec& getEntities();
	const EntityVec& getEntities(const std::string& tag);
	const EntityVec& getEntities(const std::vector<std::string>& tags);

};