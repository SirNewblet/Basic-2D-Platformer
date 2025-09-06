#pragma once

#include "Vec2.h"

#include <vector>
#include <SFML/Graphics.hpp>

class Animation
{
	sf::Texture		m_texture;
	sf::Sprite		m_sprite;
	size_t			m_frameCount = 1;		// Total number of frames of animation
	size_t			m_currentFrame = 0;		// Current frame of animation being played
	size_t			m_speed = 0;			// Speed to play the animation at
	Vec2			m_size = { 1, 1 };		// Size of the animation frame
	std::string		m_name = "NONE";

public:
	Animation();
	Animation(const std::string& name, const sf::Texture& t);
	Animation(const std::string& name, const sf::Texture& t, size_t frameCount, size_t speed);

	void update();
	bool hasEnded() const;
	const std::string& getName() const;
	std::string getType() const;
	std::string getEntityName();
	const Vec2& getSize() const;
	sf::Sprite& getSprite();
	const sf::Sprite& getSprite() const;
	const size_t getDuration() const;

	static sf::Texture& getDummyTexture();
};