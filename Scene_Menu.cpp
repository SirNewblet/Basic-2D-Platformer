#include "Scene_Menu.h"
#include "Scene_Play.h"
#include "GameEngine.h"

#include <iostream>
#include "Scene_LevelEditor.h"

Scene_Menu::Scene_Menu(GameEngine* gameEngine) : 
	Scene(gameEngine),
	m_menuFont(m_game->assets().getFont("Sooky")),
	m_menuText(m_menuFont)
{
	init();
}

void Scene_Menu::init()
{
	// Reset entity manager when entering a new scene
	m_entityManager = EntityManager();

	m_title = "Sad Man";
	m_menuStrings.push_back("Start");
	m_menuStrings.push_back("Quit");
	m_menuStrings.push_back("Options");
	m_menuStrings.push_back("Editor");

	sf::Text upText(m_menuFont, "UP: W", 40);
	sf::Text downText(m_menuFont, "DOWN: S", 40);
	sf::Text playText(m_menuFont, "PLAY: Enter", 40);
	sf::Text backText(m_menuFont, "QUIT: Esc", 40);
	m_displayStrings.push_back(upText);
	m_displayStrings.push_back(downText);
	m_displayStrings.push_back(playText);
	m_displayStrings.push_back(backText);

	m_levelPaths.push_back("levels/level1.txt");
	m_levelPaths.push_back("levels/level2.txt");
	m_levelPaths.push_back("levels/level3.txt");
	m_levelPaths.push_back("levels/level4.txt");
	m_levelPaths.push_back("levels/level5.txt");

	auto menuCharacter = m_entityManager.addEntity("Tile");
	menuCharacter.addComponent<CAnimation>(m_game->assets().getAnimation("PlayerRun"), true);
	menuCharacter.addComponent<CTransform>();
	menuCharacter.getComponent<CTransform>().scale = { 2, 2 };
	menuCharacter.getComponent<CTransform>().pos = { (float)m_game->window().getSize().x / 2.0f, (float)m_game->window().getSize().y / 2.0f };

	registerAction(sf::Keyboard::Key::W,		"UP");
	registerAction(sf::Keyboard::Key::S,		"DOWN");
	registerAction(sf::Keyboard::Key::Enter,	"PLAY");
	registerAction(sf::Keyboard::Key::Escape,	"QUIT");
	registerAction(sf::Keyboard::Key::Up,		"LEVEL_UP");
	registerAction(sf::Keyboard::Key::Down,		"LEVEL_DOWN");
}

void Scene_Menu::update()
{
	m_entityManager.update();

	sAnimation();
	sRender();

	m_currentFrame++;
}

void Scene_Menu::onEnd()
{
	m_game->quit();
}

void Scene_Menu::sDoAction(const Action& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "PLAY")
		{
			if (m_menuStrings[m_selectedMenuIndex] == "Start")
			{
				m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, m_levelPaths[m_selectedLevelIndex]));
			}
			if (m_menuStrings[m_selectedMenuIndex] == "Editor")
			{
				m_game->changeScene("EDIT", std::make_shared<Scene_LevelEditor>(m_game, m_levelPaths[m_selectedLevelIndex]));
			}
		}
		else if (action.name() == "UP")
		{
			m_selectedMenuIndex = (m_selectedMenuIndex > 0) ? --m_selectedMenuIndex : m_menuStrings.size() - 1;
		}
		else if (action.name() == "DOWN")
		{
			m_selectedMenuIndex = (m_selectedMenuIndex + 1) % m_menuStrings.size();
		}
		else if ((action.name() == "QUIT" || action.name() == "PLAY") && m_menuStrings[m_selectedMenuIndex] == "Quit")
		{
			onEnd();
		}
		else if (action.name() == "LEVEL_UP")
		{
			m_selectedLevelIndex = (m_selectedLevelIndex > 0) ? --m_selectedLevelIndex : m_levelPaths.size() - 1;
		}
		else if (action.name() == "LEVEL_DOWN")
		{
			m_selectedLevelIndex = (m_selectedLevelIndex + 1) % m_levelPaths.size();
		}
	}
}

void Scene_Menu::sAnimation()
{
	for (auto e : m_entityManager.getEntities())
	{
		if (e.hasComponent<CAnimation>())
		{
			e.getComponent<CAnimation>().animation.update();
		}
	}
}

void Scene_Menu::sRender()
{
	auto& window = m_game->window();

	// Clear the window with intended background color
	window.clear(sf::Color(67, 0, 0));

	// Display User-Input keys
	for (int i = 0; i < m_displayStrings.size(); i++)
	{
		m_displayStrings[i].setPosition({ window.getSize().x * 0.02f, (window.getSize().y * 0.02f) + (i * m_displayStrings[i].getLocalBounds().size.y * 1.5f) + 10.0f});
		m_displayStrings[i].setOutlineThickness(3);
		m_displayStrings[i].setOutlineColor(sf::Color::Black);
		window.draw(m_displayStrings[i]);
	}

	// Setup the title text
	m_menuText.setString(m_title);
	m_menuText.setOutlineColor(sf::Color::Black);
	m_menuText.setOutlineThickness(8);
	m_menuText.setFillColor(sf::Color::White);
	m_menuText.setCharacterSize(120);

	float posX = window.getSize().x / 2.0f - m_menuText.getLocalBounds().size.x / 2.0f;
	float posY = window.getSize().y * 0.02f;
	m_menuText.setPosition({ posX, posY });
	window.draw(m_menuText);

	// Setup the selectable options
	m_menuText.setCharacterSize(40);
	m_menuTextBackground.setSize(sf::Vector2f(m_menuText.getLocalBounds().size.x + 20, m_menuText.getLocalBounds().size.y + 20));
	m_menuTextBackground.setFillColor(sf::Color::Black);

	for (int i = 0; i < m_menuStrings.size(); i++)
	{
		posX = window.getSize().x - m_menuTextBackground.getSize().x - 20;
		posY = (window.getSize().y * 0.02f) + (i * (m_menuTextBackground.getSize().y * 1.5)) + 10;
		m_menuTextBackground.setPosition({ posX, posY });

		if (m_selectedMenuIndex == i)
		{
			m_menuText.setString(m_menuStrings[i]);
			m_menuText.setFillColor(sf::Color(180, 0, 0));
			m_menuText.setCharacterSize(60);

			// The bounding box's position of a sf:Text object is not (0, 0).
			// To correct this offset, subtract the local bounding rectangle's position.
			posX = m_menuTextBackground.getPosition().x + (m_menuTextBackground.getLocalBounds().size.x / 2.0f) - (m_menuText.getLocalBounds().size.x / 2.0f) - m_menuText.getLocalBounds().position.x;
			posY = m_menuTextBackground.getPosition().y + (m_menuTextBackground.getLocalBounds().size.y / 2.0f) - (m_menuText.getLocalBounds().size.y / 2.0f) - m_menuText.getLocalBounds().position.y;
			m_menuText.setPosition({ posX, posY });

			// Use the outline of the rectangle shape as a highlight to show the option that would be selected.
			m_menuTextBackground.setOutlineColor(sf::Color::Red);
			m_menuTextBackground.setOutlineThickness(4);
		}
		else
		{
			m_menuText.setString(m_menuStrings[i]);
			m_menuText.setFillColor(sf::Color::White);
			m_menuText.setCharacterSize(40);
			posX = m_menuTextBackground.getPosition().x + (m_menuTextBackground.getLocalBounds().size.x / 2.0f) - (m_menuText.getLocalBounds().size.x / 2.0f) - m_menuText.getLocalBounds().position.x;
			posY = m_menuTextBackground.getPosition().y + (m_menuTextBackground.getLocalBounds().size.y / 2.0f) - (m_menuText.getLocalBounds().size.y / 2.0f) - m_menuText.getLocalBounds().position.y;
			m_menuText.setPosition({ posX, posY });

			m_menuTextBackground.setOutlineColor(sf::Color::Black);
			m_menuTextBackground.setOutlineThickness(2);
		}

		for (auto e : m_entityManager.getEntities())
		{
			auto& transform = e.getComponent<CTransform>();

			if (e.hasComponent<CAnimation>())
			{
				auto& animation = e.getComponent<CAnimation>().animation;
				animation.getSprite().setPosition({ transform.pos.x, transform.pos.y });
				animation.getSprite().setScale({ transform.scale.x, transform.scale.y });
				m_game->window().draw(animation.getSprite());
			}
		}

		window.draw(m_menuTextBackground);
		window.draw(m_menuText);
	}

	// Display which level is currently selected
	m_menuText.setCharacterSize(40);
	m_menuText.setFillColor(sf::Color::White);
	m_menuText.setString("Level Selected:   " + m_levelPaths[m_selectedLevelIndex]);
	posX = 10;
	posY = window.getSize().y - m_menuText.getLocalBounds().size.y - 10;
	m_menuText.setPosition({ posX, posY });
	window.draw(m_menuText);
}