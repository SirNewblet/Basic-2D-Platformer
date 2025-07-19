#pragma once

#include "Components.h"

#include <tuple>
#include <string>

class EntityManager;

//typedef std::tuple<CTransform, CLifespan, CInput, CBoundingBox, CAnimation, CGravity, CState, CDraggable> ComponentTuple;

class Entity
{
	size_t m_id = 0;

	// How to access items in a tuple
	//std::tuple<int, double, char> m_myTuple;
	//std::get<int>(m_myTuple);
	// so you can do:
	//std::get<CTransform>(m_components); - gets transform component from that container m_components

	// Constructor is private so we can never create entities outside
	// the EntityManager which has friend access
	Entity(const size_t& id);

public:
	template<typename T>
	T& getComponent()
	{
		return EntityMemoryPool::Instance().getComponent<T>(m_id);
	}
	template <typename T> 
	bool hasComponent()
	{
		return EntityMemoryPool::Instance().hasComponent<T>(m_id);
	}
};
