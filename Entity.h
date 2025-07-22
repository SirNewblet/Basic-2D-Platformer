#pragma once

#include "EntityMemoryPool.h"

#include <tuple>
#include <string>

class EntityMemoryPool;

class Entity
{
	friend EntityMemoryPool;
	size_t		m_id;

	Entity(const size_t& id);
public:
	template<typename T>
	T& getComponent()
	{
		return EntityMemoryPool::Instance().getComponent<T>(m_id);
	}

	template <typename T>
	bool hasComponent() const
	{
		return EntityMemoryPool::Instance().hasComponent<T>(m_id);
	}

	template <typename T>
	T& addComponent()
	{
		return EntityMemoryPool::Instance.addComponent<T>(m_id);
	}

	template <typename T>
	T& removeComponent()
	{
		return EntityMemoryPool::Instance().removeComponent<T>(m_id);
	}
};
