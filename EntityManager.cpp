#include "EntityManager.h"

#include <iostream>

EntityManager::EntityManager() {}

void EntityManager::update()
{
	// TODO: Add entities from m_entitiesToAdd to the proper location(s)
	//			- add them to the vector of all entities
	//			- add them to the vector inside the map, with the tag as a key
	for (auto& e : m_entitiesToAdd)
	{
		m_entities.push_back(e);
		m_entityMap[e.tag()].push_back(e);
	}

	m_entitiesToAdd.clear();

	// Remove dead entities faaom each vector in the entity map
	// C++17 way of iterating through [key,value] pairs in a map
	for (auto& [tag, entityVec] : m_entityMap)
	{
		removeDeadEntities(entityVec);
	}
	removeDeadEntities(m_entities);
}

void EntityManager::removeDeadEntities(EntityVec& vec)
{
	// TODO: remove all dead entities from the input vector
	//			this is called by the update() function
	auto ne = std::remove_if(vec.begin(), vec.end(),
		[](auto e)
		{
			//std::cout << "Removing dead entity: " << e->id() << "\n";
			return !e.isActive();
		});

	vec.erase(ne, vec.end());
}

Entity EntityManager::addEntity(const std::string& tag)
{
	auto e = EntityMemoryPool::Instance().addEntity(tag);
	m_entitiesToAdd.push_back(e);
	return e;
}

const EntityVec& EntityManager::getEntities()
{
	return m_entities;
}

const EntityVec& EntityManager::getEntities(const std::string& tag)
{
	return m_entityMap[tag];
}