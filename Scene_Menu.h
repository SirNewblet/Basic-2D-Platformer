#pragma once

#include "Scene.h"

#include <map>
#include <memory>
#include <deque>

class Scene_Menu : public Scene
{

protected:

	std::string						m_title;
	std::vector<std::string>		m_menuStrings;
	std::vector<sf::Text>			m_displayStrings;
	std::vector<std::string>		m_levelPaths;
	std::vector<std::string>		m_musicPaths;
	sf::Text						m_menuText;
	sf::Font						m_menuFont;
	sf::RectangleShape				m_menuTextBackground;
	size_t							m_selectedMenuIndex = 0;

	void init();
	void update();
	void onEnd();
	void sAnimation();
	void sDoAction(const Action& action);

public:

	Scene_Menu(GameEngine* gameEngine = nullptr);
	void sRender();
};