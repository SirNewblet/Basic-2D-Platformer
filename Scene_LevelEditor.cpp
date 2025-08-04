#include "Scene_LevelEditor.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"

#include <iostream>
#include <fstream>

Scene_LevelEditor::Scene_LevelEditor(GameEngine* gameEngine, const std::string& levelPath) :
	Scene(gameEngine),
	m_levelPath(levelPath),
	m_gridText(m_game->assets().getFont("Sooky")),
	m_player(m_entityManager.addEntity("Default"))
{
	init(m_levelPath);
}

void Scene_LevelEditor::init(const std::string& levelPath)
{
	registerAction(sf::Keyboard::Key::Escape, "QUIT");
	registerAction(sf::Keyboard::Key::T, "TOGGLE_TEXTURE");					// Toggle drawing (T)extures
	registerAction(sf::Keyboard::Key::G, "TOGGLE_GRID");					// Toggle drawing (G)rid

	registerAction(sf::Keyboard::Key::W, "UP");
	registerAction(sf::Keyboard::Key::A, "LEFT");
	registerAction(sf::Keyboard::Key::S, "DOWN");
	registerAction(sf::Keyboard::Key::D, "RIGHT");

	registerAction(sf::Keyboard::Key::Y, "SAVE");

	m_gridText.setCharacterSize(24);

	loadLevel(levelPath);
}

// IMPORTANT: Always add the CAnimation component first so that gridToMidPixel can compute correctly
Vec2 Scene_LevelEditor::gridToMidPixel(float gridX, float gridY, Entity entity)
{
	//			This function takes in a grid (x, y) position and an Entity
	//			Return a Vec2 indicating where the CENTER position of the Entity should be
	//			You must use the Entity's Animation size to position it correctly
	//			The size of the grid width and height is stored in m_gridSize.x and m_gridSize.y
	//			The bottom-left corner of the Animation should align with the bottom left of the grid cell

	float x = 0.0f, y = 0.0f;
	x = (gridX * m_gridSize.x) + (entity.getComponent<CAnimation>().animation.getSize().x / 2);
	y = m_game->window().getSize().y - (gridY * m_gridSize.y) - (entity.getComponent<CAnimation>().animation.getSize().y / 2 * entity.getComponent<CTransform>().scale.y);

	return Vec2(x, y);
}

Vec2 Scene_LevelEditor::mouseToGrid(Vec2 mousePos, Entity entity)
{
	Vec2 gridLoc;
	gridLoc.x = int(mousePos.x / m_gridSize.x);
	gridLoc.y = int((m_game->window().getSize().y - mousePos.y) / m_gridSize.y);

	return gridLoc;
}

Vec2 Scene_LevelEditor::windowToWorld(const Vec2& windowPos) const
{
	auto view = m_game->window().getView();

	float wx = view.getCenter().x - (m_game->window().getSize().x / 2);
	float wy = view.getCenter().y - (m_game->window().getSize().y / 2);

	return Vec2(windowPos.x + wx, windowPos.y + wy);
}

void Scene_LevelEditor::loadLevel(const std::string& filename)
{
	// store the file name so we can use it again when we save
	m_filename = filename;

	// Reset the entity manager every time we load a level
	m_entityManager = EntityManager();

	std::ifstream fin(filename);
	std::string item = "";

	while (fin >> item)
	{
		if (item == "Tile" || item == "Decoration" || item == "Ladder")
		{

			int tileGX = 0, tileGY = 0;
			std::string name;

			fin >> name >> tileGX >> tileGY;

			auto entity = m_entityManager.addEntity(item);
			entity.addComponent<CAnimation>(m_game->assets().getAnimation(name), true);
			entity.addComponent<CTransform>();
			entity.getComponent<CTransform>().pos = gridToMidPixel(tileGX, tileGY, entity);
			entity.addComponent<CDraggable>();
			entity.addComponent<CGridLocation>(tileGX, tileGY);
		}
		else if (item == "Enemy")
		{
			int tileGX = 0, tileGY = 0, damage = 0;
			std::string name;

			fin >> name >> tileGX >> tileGY >> damage;

			auto entity = m_entityManager.addEntity(item);
			entity.addComponent<CAnimation>(m_game->assets().getAnimation(name), true);
			entity.addComponent<CDamage>(damage);
			entity.addComponent<CTransform>();
			entity.getComponent<CTransform>().pos = gridToMidPixel(tileGX, tileGY, entity);
			entity.addComponent<CDraggable>();
			entity.addComponent<CGridLocation>(tileGX, tileGY);
		}
		else if (item == "Player")
		{
			fin
				>> m_playerConfig.gridX
				>> m_playerConfig.gridY
				>> m_playerConfig.collisionX
				>> m_playerConfig.collisionY
				>> m_playerConfig.speedX
				>> m_playerConfig.speedY
				>> m_playerConfig.maxSpeed
				>> m_playerConfig.gravity
				>> m_playerConfig.WEAPON;
		}
	}

	spawnPlayer();
	spawnPoolBackground(m_poolBackground);
	loadTileSheet("level_tilesheets/level1.txt");


	//      NOTE:
	//------  THIS IS INCREDIBLY IMPORTANT PLEASE READ THIS EXAMPLE  ----------
	//		Components are now returned as references rather than pointers. If you
	//		do not specify a reference variable type, it will COPY the component
	//		Here is an example:
	//		
	//		This will COPY the transform into the variable "transform1" - it is INCORRECT
	//		Any changes you make to transform1 will not be changed inside the entity
	//		auto transform1 = entity->get<CTransform>()
	//
	//		This will REFERENCE the transform with the variable "transform2" - it is CORRECT
	//		Now any changes you make to transform2 will be changed inside the entity
	//		auto& transform2 = entity->get<CTransform>()
}

void Scene_LevelEditor::saveLevel(const std::string& levelPath)
{

	try
	{
		// first, copy the level as a backup incase saving/writing to the file fails, which could potentially
		// corrupt our file and we'd lose X amount of work on the level
		std::cout << "Attempting to create a copy of the pre-existing version...\n";
		std::string copyPath = levelPath;
		char target = '.';
		size_t pos = levelPath.find(target);
		if (pos != std::string::npos) {
			// Character found, proceed with insertion
			copyPath.insert(pos, "_COPY");
		}
		else 
		{
			std::cout << "Character not found." << std::endl;
		}
		std::ifstream source(levelPath, std::ios::binary);
		std::ofstream copy(copyPath, std::ios::binary);
		copy << source.rdbuf();
		source.close();
		copy.close();

		std::cout << "Copy of level successfully saved!\n";

		std::cout << "Attempting to save level...\n";
		std::ofstream fout(levelPath, std::ios::trunc);

		if (fout.is_open())
		{
			for (auto e : m_entityManager.getEntities())
			{
				if (e.tag() == "Player")
				{
					fout <<
						e.tag() << " " <<
						e.getComponent<CGridLocation>().x << " " <<
						e.getComponent<CGridLocation>().y << " " <<
						m_playerConfig.collisionX << " " <<
						m_playerConfig.collisionY << " " <<
						m_playerConfig.speedX << " " <<
						m_playerConfig.speedY << " " <<
						m_playerConfig.maxSpeed << " " <<
						m_playerConfig.gravity << " " <<
						m_playerConfig.WEAPON << " \n";
				}
				if (e.tag() == "Enemy")
				{
					fout << 
						e.tag() << " " << 
						e.getComponent<CAnimation>().animation.getName() << " " << 
						e.getComponent<CGridLocation>().x << " " << 
						e.getComponent<CGridLocation>().y << " " <<
						int(e.getComponent<CDamage>().damage) << " \n";

				}
				if (e.tag() == "Tile" || e.tag() == "Decoration" || e.tag() == "Ladder")
				{
					fout << 
						e.tag() << " " << 
						e.getComponent<CAnimation>().animation.getName() << " " << 
						e.getComponent<CGridLocation>().x << " " << 
						e.getComponent<CGridLocation>().y << " \n";
				}
			}

			fout.close();
			std::cout << "LEVEL SAVED SUCCESSFULLY!\n";
			m_canSave = true;
		}
		else
		{
			std::cout << "File failed to open. No exception was thrown.\n";
		}
	}
	catch (std::exception e)
	{
		std::cout << "File failed to save with ex: " << e.what() << std::endl;
	}
}

void Scene_LevelEditor::loadTileSheet(const std::string& tilesheet)
{
	std::ifstream fin(tilesheet);
	if (fin.is_open())
	{
		std::string item = "";
		while (fin >> item)
		{
			m_tileSheet.push_back(item);
		}

		for (auto e : m_tileSheet)
		{
			m_spriteSheet->push_back(m_game->assets().getAnimation(e).getSprite());
		}
	}
	else
	{
		std::cout << "Failed to open and read list of tiles...\n";
	}
}

void Scene_LevelEditor::spawnPoolBackground(sf::RectangleShape& background)
{
	background = sf::RectangleShape({ 256.0f, float(m_game->window().getView().getSize().y /*- 20*/)});
	background.setFillColor(sf::Color::Red);
	background.setOutlineColor(sf::Color::Black);
	background.setOutlineThickness(3);
	background.setPosition({ 0, 0});
}

void Scene_LevelEditor::spawnPlayer()
{
	m_player = m_entityManager.addEntity("Player");
	m_player.addComponent<CAnimation>(m_game->assets().getAnimation("PlayerIdle"), true);
	m_player.addComponent<CTransform>(Vec2(gridToMidPixel(m_playerConfig.gridX, m_playerConfig.gridY, m_player)));
	m_player.addComponent<CGridLocation>(m_playerConfig.gridX, m_playerConfig.gridY);
	m_player.addComponent<CDraggable>();
	m_camPos = { float(m_game->window().getSize().x / 2), float(m_game->window().getSize().y / 2) };
}

void Scene_LevelEditor::update()
{
	m_entityManager.update();

	// TODO: Implement pause functionality
	sDragAndDrop();
	sMovement();
	sCamera();
	sRender();

	m_currentFrame++;
}

void Scene_LevelEditor::sTilePool()
{
	float relativeX = m_game->window().getView().getCenter().x + (m_game->window().getSize().x / 2) - 256;
	float relativeY = m_game->window().getView().getCenter().y - (m_game->window().getView().getSize().y / 2)/* + 10*/;
	m_poolBackground.setPosition({ relativeX, relativeY });
	m_game->window().draw(m_poolBackground);
	int currentRow = 1;
	int currentCol = 1;
	int tileXCount = 0;
	int tileYCount = 0;
	int tilePerRow = 3;
	int buffer = 16;

	for (sf::Sprite s : *m_spriteSheet)
	{

		float xLoc = relativeX + (currentCol * buffer) + (tileXCount * m_gridSize.x);
		float yLoc = relativeY + (currentRow * buffer) + (tileYCount * m_gridSize.y);
		s.setPosition({ xLoc, yLoc });
		m_game->window().draw(s);

		tileXCount++;
		currentCol++;

		if (tileXCount == tilePerRow)
		{
			tileXCount = 0;
			currentCol = 1;
			currentRow++;
			tileYCount++;
		}


		//tileXCount++;
		//currentCol++;
		// we have 3 tiles on this row, make a new row
		//if (tileXCount >= tilePerRow)
		//{
		//	tileXCount = 0;
		//	currentCol = 1;
		//	currentRow++;
		//	tileYCount++;
		//}
	}
}

void Scene_LevelEditor::sDragAndDrop()
{
	for (auto e : m_entityManager.getEntities())
	{
		if (e.hasComponent<CDraggable>() && e.getComponent<CDraggable>().dragging)
		{
			e.getComponent<CTransform>().pos = windowToWorld(m_mPos);
		}
	}
}

void Scene_LevelEditor::sCamera()
{
	// Camera in the Level Editor should be controllable
	sf::View view = m_game->window().getView();
	view.setCenter({ m_camPos.x, m_camPos.y });
	m_game->window().setView(view);
}

void Scene_LevelEditor::sMovement()
{
	int speed = 10;
	Vec2 vel = { 0, 0 };
	if (m_player.getComponent<CInput>().up)
	{
		vel.y = -speed;
	}
	if (m_player.getComponent<CInput>().down)
	{
		vel.y = speed;
	}
	if (m_player.getComponent<CInput>().right)
	{
		vel.x = speed;
	}
	if (m_player.getComponent<CInput>().left)
	{
		vel.x = -speed;
	}

	m_camPos += vel;
}

void Scene_LevelEditor::sDoAction(const Action& action)
{
	// DO NOT MODIFY PLAYER MOVEMENT HERE - ONLY FOR USING ACTION TO SET CInput COMPONENT
	if (action.type() == "START")
	{
		if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
		if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }
		if (action.name() == "QUIT") { onEnd(); }
		if (action.name() == "SAVE" && m_canSave) { m_canSave = false; saveLevel(m_filename); }
		if (action.name() == "UP") { m_player.getComponent<CInput>().up = true; }
		if (action.name() == "DOWN") { m_player.getComponent<CInput>().down = true; }
		if (action.name() == "LEFT") { m_player.getComponent<CInput>().left = true; }
		if (action.name() == "RIGHT") { m_player.getComponent<CInput>().right = true; }
		if (action.name() == "LEFT_CLICK")
		{
			Vec2 worldPos = windowToWorld(action.pos());
			//std::cout << "Mouse clicked at: " << worldPos.x << ", " << worldPos.y << std::endl;
			for (auto e : m_entityManager.getEntities())
			{
				if (e.hasComponent<CDraggable>() && Physics::IsInside(worldPos, e))
				{
					std::cout << "CLICKED ON ENTITY: " << e.getComponent<CAnimation>().animation.getName() << std::endl;
					e.getComponent<CDraggable>().dragging = !e.getComponent<CDraggable>().dragging;
					if (!e.getComponent<CDraggable>().dragging)
					{
						// put entity into the correct grid location (snapping)
						Vec2 gridLocation = mouseToGrid(worldPos, e);
						e.getComponent<CGridLocation>().x = gridLocation.x;
						e.getComponent<CGridLocation>().y = gridLocation.y;
						e.getComponent<CTransform>().pos = gridToMidPixel(gridLocation.x, gridLocation.y, e);
						 
					}
				}
			}
		}
		if (action.name() == "MOUSE_MOVE")
		{
			m_mPos = action.pos();
			m_mouseShape.setPosition({ m_mPos.x, m_mPos.y });
		}
		if (action.name() == "RIGHT_CLICK")
		{
			Vec2 worldPos = windowToWorld(action.pos());
			for (auto e : m_entityManager.getEntities())
			{
				if (Physics::IsInside(worldPos, e))
				{
					e.destroy();
				}
			}
		}
	}
	else if (action.type() == "END")
	{
		if (action.name() == "UP") { m_player.getComponent<CInput>().up = false; }
		if (action.name() == "DOWN") { m_player.getComponent<CInput>().down = false; }
		if (action.name() == "LEFT") { m_player.getComponent<CInput>().left = false; }
		if (action.name() == "RIGHT") { m_player.getComponent<CInput>().right = false; }
	}
}

void Scene_LevelEditor::sRender()
{
	// Color the background darker so you know that the game is paused
	if (!m_paused) { m_game->window().clear(sf::Color(0xa83e75)); }
	else { m_game->window().clear(sf::Color(0xa83ea8)); }

	// Draw all Entity textures + animations
	if (m_drawTextures)
	{
		for (auto e : m_entityManager.getEntities())
		{
			auto& transform = e.getComponent<CTransform>();

			if (e.hasComponent<CAnimation>())
			{
				auto& animation = e.getComponent<CAnimation>().animation;
				animation.getSprite().setRotation(sf::degrees(transform.angle));
				animation.getSprite().setPosition({ transform.pos.x, transform.pos.y });
				animation.getSprite().setScale({ transform.scale.x, transform.scale.y });
				m_game->window().draw(animation.getSprite());
			}
		}

		sTilePool();
	}

	// Draw the grid so that students can easily debug
	if (m_drawGrid)
	{
		float leftX = m_game->window().getView().getCenter().x - width() / 2;
		float rightX = leftX + width() + m_gridSize.x;
		float nextGridX = leftX - ((int)leftX % (int)m_gridSize.x);

		for (float x = nextGridX; x < rightX; x += m_gridSize.x)
		{
			drawLine(Vec2(x, 0.0f), Vec2(x, (float)height()));
		}

		for (float y = 0; y < height(); y += m_gridSize.y)
		{
			drawLine(Vec2(leftX, height() - y), Vec2(rightX, height() - y));

			for (float x = nextGridX; x < rightX; x += m_gridSize.x)
			{
				std::string xCell = std::to_string((int)x / (int)m_gridSize.x);
				std::string yCell = std::to_string((int)y / (int)m_gridSize.y);
				m_gridText.setString("(" + xCell + "," + yCell + ")");
				m_gridText.setPosition({ x + 3, height() - y - m_gridSize.y + 2 });
				m_game->window().draw(m_gridText);
			}
		}
	}
	m_mouseShape.setFillColor(sf::Color(255, 0, 0));
	m_mouseShape.setRadius(4);
	m_mouseShape.setOrigin({ 2, 2 });
	Vec2 worldPos = windowToWorld(m_mPos);
	m_mouseShape.setPosition({ worldPos.x, worldPos.y });
	m_game->window().draw(m_mouseShape);
	m_currentFrame++;
}

void Scene_LevelEditor::onEnd()
{
	m_hasEnded = true;
	sf::View view = m_game->window().getView();
	view.setCenter({ m_game->window().getSize().x / 2.0f, m_game->window().getSize().y / 2.0f });
	m_game->window().setView(view);
	m_game->changeScene("MENU", nullptr, true);
}