#include "EntityMemoryPool.h"
#include "Entity.h"
#include <iostream>

EntityMemoryPool::EntityMemoryPool(size_t maxEnts) :
	m_maxEntities(maxEnts)
{
	reserveAll(MAX_ENTITIES);
}

void EntityMemoryPool::reserveAll(size_t maxNum)
{
	// Allocated memory for every vector using MAX_ENTITIES
	m_tags.resize(maxNum);
	m_active.resize(maxNum, false);
	std::apply([maxNum](auto&... vectors) {
		(..., vectors.resize(maxNum));
		}, m_pool);
}

void EntityMemoryPool::destroy(size_t id)
{
	m_active[id] = false;
}

const std::string& EntityMemoryPool::getTag(size_t id) const
{
	return m_tags[id];
}

bool EntityMemoryPool::isActive(size_t id) const
{
	return m_active[id];
}

Entity EntityMemoryPool::addEntity(const std::string& tag)
{
	size_t index = getNextEntityIndex();

	m_tags[index] = tag;
	m_active[index] = true;
	return Entity(index);
}

std::vector<Entity> EntityMemoryPool::getEntities()
{
	std::vector<Entity> taggedEntities;

	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		if (m_active[i])
		{
			taggedEntities.push_back(std::get<std::vector<Entity>>(m_pool)[i]);
		}
	}

	return taggedEntities;
}

std::vector<Entity>& EntityMemoryPool::getEntities(const std::string& tag)
{
	std::vector<Entity> taggedEntities;

	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		if (m_tags[i] == tag && m_active[i])
		{
			taggedEntities.push_back(std::get<std::vector<Entity>>(m_pool)[i]);
		}
	}

	return taggedEntities;
}

size_t EntityMemoryPool::getNextEntityIndex()
{
	if (m_numEntities < MAX_ENTITIES)
	{
		// We have room to add an entity
		for (int i = 0; i < MAX_ENTITIES; i++)
		{
			// Found an index marked as inactive (we can use this slot)
			if (!m_active[i])
			{
				// We will need to decrement when we call destroy() on an entity
				m_numEntities++;

				// return inactive index to be used
				return i;
			}
		}
	}
	else
	{
		// TODO
		// Handle error here for exceeding max num of entities allowed
		// but for now do nothing (don't add entity)
		std::cout << "Max entity count exceeded!!!" << std::endl;
	}

	return 0;
}