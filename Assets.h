#pragma once

#include "Animation.h"
#include <SFML/Audio.hpp>
#include <map>

class Assets
{
private:
	std::map<std::string, sf::Texture>						m_textureMap;
	std::map<std::string, Animation>						m_animationMap;
	std::map<std::string, sf::Font>							m_fontMap;
	std::map<std::string, sf::SoundBuffer>					m_soundBufferMap;
	std::map<std::string, std::unique_ptr<sf::Sound>>       m_soundMap;
	std::map<std::string, std::string>						m_musicMap;
	Vec2													m_tileSize = { 64, 64 };

	void addTexture(const std::string& textureName, const std::string& path, bool smooth = true);
	bool isTileEmpty(const sf::Image& tileImage, Vec2& tileSize);
	void processTilesheet(const std::string& tilesheetName, const std::string& path, std::vector<std::string>& tileNames);
	void addAnimation(const std::string& animationName, const std::string& textureName, size_t frameCount, size_t speed);
	void addFont(const std::string& fontName, const std::string& path);
	void addSound(const std::string& soundName, const std::string& path);
	void addMusic(const std::string& musicName, const std::string& path);

public:
    Assets();

    bool loadFromFile(const std::string& path);

	const std::map<std::string, sf::Texture>& getTextures() const;
	const std::map<std::string, Animation>& getAnimations() const;
	const std::map<std::string, std::unique_ptr<sf::Sound>>& getSounds() const;
	const std::map<std::string, std::string>& getMusic() const;

	const sf::Texture& getTexture(const std::string& textureName) const;
	const Animation& getAnimation(const std::string& animationName) const;
	const sf::Font& getFont(const std::string& fontName) const;
	std::unique_ptr<sf::Sound>& getSound(const std::string& soundName);
	const std::string& getMusic(const std::string& musicName) const;
};