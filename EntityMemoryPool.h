#pragma once

#include "Components.h"
#include "Entity.h"

#include <vector>
#include <string>

static const size_t MAX_ENTITIES = 10000;

typedef std::tuple<
	std::vector<CTransform>,
	std::vector<CLifespan>,
	std::vector<CInput>,
	std::vector<CBoundingBox>,
	std::vector<CAnimation>,
	std::vector<CGravity>,
	std::vector<CState>,
	std::vector<CDraggable>> EntityComponentVectorTuple;

class EntityMemoryPool
{
	size_t							m_numEntities;
	EntityComponentVectorTuple		m_pool;
	std::vector<std::string>		m_tags;
	std::vector<bool>				m_active;

	EntityMemoryPool(size_t maxEntities);

public:
	static EntityMemoryPool& Instance()
	{
		static EntityMemoryPool pool(MAX_ENTITIES);
		return pool;
	}

	template <typename T>
	T& getComponent(size_t entityId)
	{
		return std::get<std::vector<T>>(m_pool)[entityId];
	}

	template <typename T>
	const T& getComponent(size_t entityId) const
	{
		return std::get<std::vector<T>>(m_pool)[entityId];
	}

	template <typename T>
	T& hasComponent(size_t entityId)
	{
		return std::get<std::vector<T>>(m_data)[entityId].has;
	}

	template <typename T, typename... TArgs>
	T& addComponent(TArgs&&... mArgs)
	{
		auto& component = getComponent<T>();
		component = T(std::forward<TArgs>(mArgs)...);
		component.has = true;
		return component;
	}

	template <typename T>
	size_t getNextEntityIndex()
	{
		auto pp = std::get<std::vector<T>>(m_pool);

		return 0;
	}

	template <typename T>
	void removeComponent(size_t entityId)
	{
		getComponent<T>(entityId) = T(entityId);
	}

	Entity addEntity(const std::string& tag)
	{
		size_t index = getNextEntityIndex();

		m_tags[index] = tag;
		m_active[index] = true;
		return Entity(index);
	}

	const std::string& getTag(size_t entityId) const
	{
		return m_tags[entityId];
	}
};