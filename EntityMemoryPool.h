#pragma once

#include "Components.h"
//#include "Entity.h"

#include <vector>
#include <string>

static const size_t MAX_ENTITIES = 1000000;

typedef std::tuple<
	std::vector<CTransform>,
	std::vector<CLifespan>,
	std::vector<CInput>,
	std::vector<CBoundingBox>,
	std::vector<CAnimation>,
	std::vector<CGravity>,
	std::vector<CState>,
	std::vector<CDraggable>> EntityComponentVectorTuple;

class Entity;

class EntityMemoryPool
{
	size_t							m_numEntities = 0;
	const size_t					m_maxEntities;
	EntityComponentVectorTuple		m_pool;
	std::vector<std::string>		m_tags;
	std::vector<bool>				m_active;

	EntityMemoryPool(size_t maxEntities);
	void reserveAll(size_t maxEntities);
	void removeAllComponents(size_t entityId);

public:

	const std::string& getTag(size_t entityId) const;
	bool isActive(size_t entityId) const;
	void destroy(size_t entityId);
	size_t getNextEntityIndex();
	Entity addEntity(const std::string& tag);

	static EntityMemoryPool& Instance()
	{
		static EntityMemoryPool pool(MAX_ENTITIES);
		return pool;
	}

	template <typename T>
	T& getComponent(size_t id)
	{
		return std::get<std::vector<T>>(m_pool)[id];
	}

	//template <typename T>
	//const T& getComponent(size_t id) const
	//{
	//	return std::get<std::vector<T>>(m_pool)[id];
	//}

	template <typename T>
	bool hasComponent(size_t id)
	{
		return std::get<std::vector<T>>(m_pool)[id].has;
	}

	template <typename T, typename... TArgs>
	T& addComponent(size_t id, TArgs&&... mArgs)
	{
		auto& component = getComponent<T>(id);
		component = T(std::forward<TArgs>(mArgs)...);
		component.has = true;
		return component;
	}

	template <typename T>
	void removeComponent(size_t id)
	{
		getComponent<T>(id) = T(id);
	}
};