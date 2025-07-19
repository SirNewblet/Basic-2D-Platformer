#pragma once
constexpr int MAX_ENTITIES = 10000;

#include <vector>
#include <map>
#include "Components.h"

typedef std::tuple< std::vector<CTransform>, std::vector<CLifespan>, std::vector<CInput>, std::vector<CBoundingBox>, std::vector<CAnimation>, std::vector<CGravity>, std::vector<CState>, std::vector<CDraggable>> EntityComponentVecTuple;

class EntityMemoryPool
{
	size_t m_numEntities;
	EntityComponentVecTuple m_pool;
	std::vector<std::string> m_tags;
	std::vector<bool> m_active;
	EntityMemoryPool(size_t maxEntities);

public:
	static EntityMemoryPool& Instance()
	{
		static EntityMemoryPool pool(MAX_ENTITIES);
		return pool;
	}

	template <typename T>
	T& getComponent(size_t entityID)
	{
		return std::get<std::vector<T>>(m_pool)[entityID];
	}
	const std::string& getTag(size_t entityID) const
	{
		return m_tags[entityID];
	}
};