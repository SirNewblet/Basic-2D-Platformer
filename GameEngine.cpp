#include "Scene_Menu.h"
#include "GameEngine.h"

#include <iostream>
#include <fstream>

GameEngine::GameEngine(const std::string& path)
{
	init(path);
}

void GameEngine::init(const std::string& path)
{
	if (m_assets.loadFromFile(path))
	{
		std::cout << "Assets loaded from " << path << " successfully.\n";
	}
	else
	{
		std::cerr << "Assets failed to load.";
	}

	std::cout << "Attempting to read in 'config.txt' now.\n";
	std::ifstream file("config.txt");
	std::string str;
	unsigned int wWidth = 0, wHeight = 0;

	if (!file.is_open())
	{
		std::cerr << "Error encountered when attempting to open/read the config.txt file.\n";
		wWidth = 1080; wHeight = 720;
	}
	else
	{
		std::cout << "Config.txt file opened without error - reading now...\n";
		while (file >> str)
		{
			if (str == "Window")
			{
				file >> wWidth >> wHeight >> m_fps;
			}
		}
	}

	m_window.create(sf::VideoMode({ wWidth, wHeight }), "Definitely NOT Mario");
	m_window.setFramerateLimit(m_fps);

	changeScene("MENU", std::make_shared<Scene_Menu>(this));
}

std::shared_ptr<Scene> GameEngine::currentScene()
{
	return m_sceneMap[m_currentScene];
}

bool GameEngine::isRunning()
{
	return m_running && m_window.isOpen();
}

sf::RenderWindow& GameEngine::window()
{
	return m_window;
}

void GameEngine::run()
{
	while (isRunning())
	{
		update();
	}
}

void GameEngine::sUserInput()
{
	while (const std::optional event = m_window.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
		{
			m_running = false;
			quit();
		}

		if (event->is<sf::Event::KeyPressed>() || event->is<sf::Event::KeyReleased>())
		{

			// Determine start or end action by whether it was key press or release
			const std::string actionType = event->getIf<sf::Event::KeyPressed>() ? "START" : "END";

			if (actionType == "START")
			{
				if (event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::X)
				{
					// Attempt to screenshot the window and save the screenshot
					sf::Texture texture;
					if (texture.resize({ (unsigned int)m_window.getSize().x, (unsigned int)m_window.getSize().y }))
					{
						std::cout << "Texture successfully resized.\n";
						texture.update(m_window);
						if (texture.copyToImage().saveToFile("test.png"))
						{
							std::cout << "Screen shot saved to " << "test.png" << "\n";
						}
					}
					else
					{
						std::cerr << "Texture failed to resize.\n";
					}
				}

				// If the current scene does not have an action associated with this key, skip the event
				if (currentScene()->getActionMap().find(event->getIf<sf::Event::KeyPressed>()->code) == currentScene()->getActionMap().end())
				{
					continue;
				}

				// Look up the action and send the action to the scene
				currentScene()->doAction(Action(currentScene()->getActionMap().at(event->getIf<sf::Event::KeyPressed>()->code), actionType));
			}
			else if (actionType == "END")
			{
				if (currentScene()->getActionMap().find(event->getIf<sf::Event::KeyReleased>()->code) == currentScene()->getActionMap().end())
				{
					continue;
				}

				// Look up the action and send the action to the scene
				currentScene()->doAction(Action(currentScene()->getActionMap().at(event->getIf<sf::Event::KeyReleased>()->code), actionType));
			}
			else
			{

			}
		}
	}
}

void GameEngine::changeScene(const std::string& sceneName, std::shared_ptr<Scene> scene, bool endCurrentScene)
{
	if (scene)
	{
		m_sceneMap[sceneName] = scene;
	}
	else
	{
		if (m_sceneMap.find(sceneName) == m_sceneMap.end())
		{
			std::cerr << "Warning: Scene does not exist: " << sceneName << "\n";
			return;
		}
	}

	if (endCurrentScene)
	{
		m_sceneMap.erase(m_sceneMap.find(m_currentScene));
	}

	m_currentScene = sceneName;
}

void GameEngine::quit()
{
	m_running = false;
}

const Assets& GameEngine::assets() const
{
	return m_assets;
}

void GameEngine::update()
{
	if (!isRunning()) { return; }
	if (m_sceneMap.empty()) { return; }

	sUserInput();
	currentScene()->simulate(m_simulationSpeed);

	// Render system is separated from the scene update so the game engine can
	// simulate a specified number of frames without rendering each frame of simulation
	currentScene()->sRender();
	m_window.display();
}

void GameEngine::playSound(const std::string& soundName)
{
	if (soundName.find("Music") != std::string::npos)
	{
		if (!m_music.openFromFile(this->assets().getMusic(soundName)))
		{
			std::cerr << "Music failed to open - check file name." << "\n";
		}

		m_music.play();
	}
	else
	{
		m_assets.getSound(soundName)->play();
	}
}

void GameEngine::stopSound(const std::string& soundName)
{
	if (soundName.find("Music") != std::string::npos)
	{
		m_music.stop();
	}
	else
	{
		m_assets.getSound(soundName)->stop();
	}
}

const int GameEngine::getFps() const
{
	return m_fps;
}