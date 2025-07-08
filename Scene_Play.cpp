#include "Scene_Play.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"

#include <iostream>
#include <fstream>

Scene_Play::Scene_Play(GameEngine* gameEngine, const std::string& levelPath) :
	Scene(gameEngine),
	m_levelPath(levelPath),
	m_gridText(m_game->assets().getFont("Sooky"))
{
	init(m_levelPath);
}

void Scene_Play::init(const std::string& levelPath)
{
	registerAction(sf::Keyboard::Key::P,			"PAUSE");
	registerAction(sf::Keyboard::Key::Escape,		"QUIT");
	registerAction(sf::Keyboard::Key::T,			"TOGGLE_TEXTURE");				// Toggle drawing (T)extures
	registerAction(sf::Keyboard::Key::C,			"TOGGLE_COLLISION");			// Toggle drawing (C)ollision Boxes
	registerAction(sf::Keyboard::Key::G,			"TOGGLE_GRID");					// Toggle drawing (G)rid

	registerAction(sf::Keyboard::Key::Space,		"JUMP");
	registerAction(sf::Keyboard::Key::A,			"LEFT");
	registerAction(sf::Keyboard::Key::D,			"RIGHT");
	registerAction(sf::Keyboard::Key::S,			"CROUCH");
	//registerAction((sf::Keyboard::Key)sf::Mouse::Button::Left,		"SHOOT");
	//registerAction((sf::Keyboard::Key)sf::Mouse::Button::Right,		"SPECIAL");
	// TODO: Register all other gameplay Actions

	m_gridText.setCharacterSize(24);

	loadLevel(levelPath);
}

// IMPORTANT: Always add the CAnimation component first so that gridToMidPixel can compute correctly
Vec2 Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
	// PRIORITIZE THIS FUNCTION EARLY
	// TODO: This function takes in a grid (x, y) position and an Entity
	//			Return a Vec2 indicating where the CENTER position of the Entity should be
	//			You must use the Entity's Animation size to position it correctly
	//			The size of the grid width and height is stored in m_gridSize.x and m_gridSize.y
	//			The bottom-left corner of the Animation should align with the bottom left of the grid cell

	float x = 0.0f, y = 0.0f;
	x = (gridX * m_gridSize.x) + (entity->getComponent<CAnimation>().animation.getSize().x / 2);
	y = (gridY * m_gridSize.y) + m_game->window().getSize().y - (entity->getComponent<CAnimation>().animation.getSize().y / 2);

	return Vec2(x, y);
}

void Scene_Play::loadLevel(const std::string& filename)
{
	// Reset the entity manager every time we load a level
	m_entityManager = EntityManager();

	// TODO: read in the level file and add the appropriate entities
	//			use the PlayerConfig struct m_playerConfig to store player properties
	//			this struct is defined at the top of Scene_Play.h


	// NOTE: All of the code below is sample code which shows you how to set up and use entities with the new syntax, it should be removed

	std::ifstream fin(filename);
	std::string item = "";

	while (fin >> item)
	{
		if (item == "Tile")
		{
			
			int tileGX = 0, tileGY = 0;
			std::string name;

			fin >> name >> tileGX >> tileGY;

			auto brick = m_entityManager.addEntity("tile");
			brick->addComponent<CAnimation>(m_game->assets().getAnimation(name), true);
			brick->addComponent<CTransform>(gridToMidPixel(tileGX, tileGY, brick));
			brick->addComponent<CBoundingBox>(Vec2(brick->getComponent<CAnimation>().animation.getSize().x, brick->getComponent<CAnimation>().animation.getSize().y));
		}
		else if (item == "Decoration")
		{
			
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
				>> m_playerConfig.jumpSpeed
				>> m_playerConfig.gravity
				>> m_playerConfig.WEAPON;
		}
	}

	spawnPlayer();


	//if (brick->getComponent<CAnimation>().animation.getName() == "Brick")
	//{
	//	std::cout << "This could be a good way of identifying if a tile is a brick!\n";
	//}

	//if (brick->hasComponent<CAnimation>())
	//{
	//	// do whatever here if the brick has a CAnimation component
	//	// can do something like:
	//	brick->removeComponent<CAnimation>();
	//}

	//auto block = m_entityManager.addEntity("tile");
	//block->addComponent<CAnimation>(m_game->assets().getAnimation("PlayerStand"), true);
	//block->addComponent<CTransform>(Vec2(224, 480));
	//// Add a bounding box, this will now show up if we press the "C" key
	//block->addComponent<CBoundingBox>(m_game->assets().getAnimation("PlayerStand").getSize());

	//auto question = m_entityManager.addEntity("tile");
	//question->addComponent<CAnimation>(m_game->assets().getAnimation("PlayerStand"), true);
	//question->addComponent<CTransform>(Vec2(352, 480));

	// NOTE: THIS IS INCREDIBLY IMPORTANT PLEASE READ THIS EXAMPLE
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

void Scene_Play::spawnPlayer()
{
	// Here is a sample player entity which you can use to construct other entities
	m_player = m_entityManager.addEntity("player");
	m_player->addComponent<CAnimation>(m_game->assets().getAnimation("PlayerStand"), true);
	m_player->addComponent<CTransform>(Vec2(100, 100));
	m_player->addComponent<CBoundingBox>(Vec2(m_player->getComponent<CAnimation>().animation.getSize().x, m_player->getComponent<CAnimation>().animation.getSize().y));
	m_player->addComponent<CGravity>(0.2);

	// TODO: Be sure to add the remaining components to the player
}

void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
	// TODO: This should spawn a bullet at the given entity, going in the direction the entity is facing
}

std::shared_ptr<Entity> Scene_Play::player()
{
	return nullptr;
}

void Scene_Play::update()
{
	m_entityManager.update();

	// TODO: Implement pause functionality
	
	sMovement();
	sLifespan();
	sCollision();
	sStatus();
	sAnimation();
	sRender();
}

void Scene_Play::sMovement()
{
	Vec2 playerVelocity(0, m_player->getComponent<CTransform>().velocity.y);

	if (m_player->getComponent<CInput>().right || m_player->getComponent<CInput>().left)
	{
		playerVelocity.x = m_player->getComponent<CInput>().right ? 5 : -5;
		m_player->getComponent<CTransform>().scale.x = m_player->getComponent<CInput>().right ? 1 : -1;
		//m_player->getComponent<CState>().state = "run";
	}

	if (m_player->getComponent<CInput>().canJump && m_player->getComponent<CInput>().up)
	{
		//m_player->getComponent<CState>().state = "jump";
		m_player->getComponent<CInput>().canJump = false;
		playerVelocity.y = -10;
	}

	m_player->getComponent<CTransform>().velocity = playerVelocity;

	for (auto e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CGravity>())
		{
			e->getComponent<CTransform>().velocity.y += e->getComponent<CGravity>().gravity;

			// If the player is moving faster than max speed in any direction
			// set its speed to the max speed in that direction

			// When player lands on the ground, recent Y velocity to 0
			if (e->getComponent<CTransform>().pos.y > m_game->window().getSize().y - 64)
			{
				e->getComponent<CTransform>().pos.y = m_game->window().getSize().y - 64;
				e->getComponent<CTransform>().velocity.y = 0.0f;
				//e->getComponent<CState>().state = "stand";
				e->getComponent<CInput>().canJump = true;
			}
		}

		e->getComponent<CTransform>().pos += e->getComponent<CTransform>().velocity;
	}


	// TODO: Implement player movement / jumping based on its CInput component
	// TODO: Implement gravity's effect on the player
	// TODO: Implement the maximum player speed in both X and Y directions
	// NOTE: Setting an entity's scale.x to -1 / 1 will make it face to the LEFT / RIGHT
}

void Scene_Play::sLifespan()
{
	// TODO: Check lifespan of entities that have them, and destroy them if they go over
}

void Scene_Play::sCamera()
{
	// TODO: Keep Camera on player unless player runs left / falls down a hole / enters a gate
}

void Scene_Play::sStatus()
{
	// We should not modify state outside of this function
	// We should check health here instead
	if (m_player->getComponent<CState>().state != "dead")
	{
		if ((m_player->getComponent<CInput>().left || m_player->getComponent<CInput>().right) && !m_player->getComponent<CInput>().up)
		{
			m_player->getComponent<CState>().state = "run";
		}
		if (m_player->getComponent<CTransform>().velocity.x == 0)
		{
			m_player->getComponent<CState>().state = "stand";
		}
		if (m_player->getComponent<CTransform>().velocity.y != 0)
		{
			m_player->getComponent<CState>().state = "jump";
		}
	}
	else
	{
		// Player has died
		// m_player->getComponent<CState>().state = "dead";
		// Call function "lifelost" here or something to handle when player HP <= 0
	}
}

void Scene_Play::sCollision()
{
	// REMEMBER: SFML's (0,0) position is on the TOP-LEFT corner
	//					This means jumping will have a negative y-component
	//					and gravity will have a positive y-component
	//					Also, something BELOW something else will have a y value GREATER than it
	//					Also, something ABOVE something else will have a y value LESS than it

	// TODO: Implement Physics::GetOverlap() function, use it inside this function
	
	// TODO: Implement bullt / tile collisions
	//			- Destroy the tile if it has a Brick animation
	// TODO: Implement player / tile collisions
	//			- Update the CState component of the player to store whether
	//			- it is currently on the ground or in the air. This will be 
	//			- used by the Animation system
	// TODO: Check to see if the player has fallen down a hole (y > height())
	// TODO: Don't let the player walk off the left side of the map
}

void Scene_Play::sDoAction(const Action& action)
{
	// DO NOT MODIFY PLAYER MOVEMENT HERE - ONLY FOR USING ACTION TO SET CInput COMPONENT
	if (action.type() == "START")
	{
		if (action.name() == "TOGGLE_TEXTURE")		{ m_drawTextures = !m_drawTextures; }
		if (action.name() == "TOGGLE_COLLISION")	{ m_drawCollision = !m_drawCollision; }
		if (action.name() == "TOGGLE_GRID")			{ m_drawGrid = !m_drawGrid; }
		if (action.name() == "PAUSE")				{ setPaused(!m_paused); }
		if (action.name() == "QUIT")				{ onEnd(); }
		if (action.name() == "JUMP")				{ m_player->getComponent<CInput>().up = true; }
		if (action.name() == "CROUCH")				{ m_player->getComponent<CInput>().down = true; }
		if (action.name() == "LEFT")				{ m_player->getComponent<CInput>().left = true; }
		if (action.name() == "RIGHT")				{ m_player->getComponent<CInput>().right = true; }
		if (action.name() == "SHOOT")				{ m_player->getComponent<CInput>().shoot = true; }
		if (action.name() == "SPECIAL")				{ m_player->getComponent<CInput>().special = true; }
	}
	else if (action.type() == "END")
	{
		if (action.name() == "JUMP")				{ m_player->getComponent<CInput>().up = false; }
		if (action.name() == "CROUCH")				{ m_player->getComponent<CInput>().down = false; }
		if (action.name() == "LEFT")				{ m_player->getComponent<CInput>().left = false; }
		if (action.name() == "RIGHT")				{ m_player->getComponent<CInput>().right = false; }
	}
}

void Scene_Play::sAnimation()
{
	// TODO: Complete the Animation class code first

	// TODO: set the animation of the player based on its CState component
	// TODO: For each entity with an animation, call entity->getComponent<CAnimation>().animation.update()
	//			if the animation is not repeated, and it has ended, destroy that entity

	for (auto& e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CAnimation>())
		{
			if (e->getComponent<CState>().state == "run")
			{
				e->addComponent<CAnimation>(m_game->assets().getAnimation("PlayerStand"), true);
			}
			if (e->getComponent<CState>().state == "jump")
			{
				e->removeComponent<CAnimation>();
				e->addComponent<CAnimation>(m_game->assets().getAnimation("PlayerJump"), true);
			}
			if (e->getComponent<CState>().state == "stand")
			{
				e->removeComponent<CAnimation>();
				e->addComponent<CAnimation>(m_game->assets().getAnimation("PlayerStand"), true);
			}
			if (!e->getComponent<CAnimation>().repeat)
			{
				// Call destroy on entity here since it's animation has finished
			}
			e->getComponent<CAnimation>().animation.update();
		}
	}
}

void Scene_Play::sRender()
{
	// Color the background darker so you know that the game is paused
	if (!m_paused) { m_game->window().clear(sf::Color(100, 100, 255)); }
	else { m_game->window().clear(sf::Color(50, 50, 150)); }

	// Set the viewport of the window to be centered on the player if it's far enough right
	auto& pPos = m_player->getComponent<CTransform>().pos;
	float windowCenterX = std::max(m_game->window().getSize().x / 2.0f, pPos.x);
	sf::View view = m_game->window().getView();
	view.setCenter({ windowCenterX, m_game->window().getSize().y - view.getCenter().y });
	m_game->window().setView(view);

	// Draw all Entity textures + animations
	if (m_drawTextures)
	{
		for (auto e : m_entityManager.getEntities())
		{
			auto& transform = e->getComponent<CTransform>();

			if (e->hasComponent<CAnimation>())
			{
				auto& animation = e->getComponent<CAnimation>().animation;
				animation.getSprite().setRotation(sf::degrees(transform.angle));
				animation.getSprite().setPosition({ transform.pos.x, transform.pos.y });
				animation.getSprite().setScale({ transform.scale.x, transform.scale.y });
				m_game->window().draw(animation.getSprite());
			}
		}
	}

	// Draw all Entity collision bounding boxes with a rectangleShape
	if (m_drawCollision)
	{
		for (auto e : m_entityManager.getEntities())
		{
			if (e->hasComponent<CBoundingBox>())
			{
				auto& box = e->getComponent<CBoundingBox>();
				auto& transform = e->getComponent<CTransform>();
				sf::RectangleShape rect;
				rect.setSize(sf::Vector2f(box.size.x - 1, box.size.y - 1));
				rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
				rect.setPosition({ transform.pos.x, transform.pos.y });
				rect.setFillColor(sf::Color(0, 0, 0, 0));
				rect.setOutlineColor(sf::Color(255, 255, 255, 255));
				rect.setOutlineThickness(1);
				m_game->window().draw(rect);
			}
		}
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
}

void Scene_Play::onEnd()
{
	// TODO: When the scene ends, change back to the MENU scene
	//			use m_game->changeScene(correct params);
}
