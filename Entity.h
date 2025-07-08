#pragma once

#include "Components.h"

#include <tuple>
#include <string>

class EntityManager;

typedef std::tuple<CTransform, CLifespan, CInput, CBoundingBox, CAnimation, CGravity, CState> ComponentTuple;

class Entity
{
	friend class EntityManager;

	bool						m_active = true;
	std::string					m_tag = "default";
	size_t						m_id = 0;
	ComponentTuple				m_components;

	// How to access items in a tuple
	//std::tuple<int, double, char> m_myTuple;
	//std::get<int>(m_myTuple);
	// so you can do:
	//std::get<CTransform>(m_components); - gets transform component from that container m_components

	// Constructor is private so we can never create entities outside
	// the EntityManager which has friend access
	Entity(const size_t& id, const std::string& tag);

public:
	void						destroy();
	const size_t				id()			const;
	bool						isActive()		const;
	const std::string&			tag()			const;

	template <typename T>
	bool hasComponent() const
	{
		return getComponent<T>().has;
	}

	template <typename T, typename... TArgs>
	T& addComponent(TArgs&&... mArgs)
	{
		auto& component = getComponent<T>();
		component = T(std::forward<TArgs>(mArgs)...);
		component.has = true;
		return component;
	}

	template<typename T>
	T& getComponent()
	{
		return std::get<T>(m_components);
	}

	template<typename T>
	const T& getComponent() const
	{
		return std::get<T>(m_components);
	}

	template<typename T>
	void removeComponent()
	{
		getComponent<T>() = T();
	}
};
