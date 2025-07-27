#include "Scene_Play.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"

#include <iostream>
#include <fstream>

bool isInside(Vec2 pos, Entity e)
{
	auto ePos = e.getComponent<CTransform>().pos;
	auto size = e.getComponent<CAnimation>().animation.getSize();
	float dx = fabs(pos.x - ePos.x);
	float dy = fabs(pos.y - ePos.y);

	return ((dx <= size.x / 2) && (dy <= size.y / 2));
}

Scene_Play::Scene_Play(GameEngine* gameEngine, const std::string& levelPath) :
	Scene(gameEngine),
	m_levelPath(levelPath),
	m_gridText(m_game->assets().getFont("Sooky")),
	m_player(m_entityManager.addEntity("Default"))
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
	registerAction(sf::Keyboard::Key::Enter,		"SHOOT");
	registerAction(sf::Keyboard::Key::W,			"CLIMB");
	registerAction(sf::Keyboard::Key::A,			"LEFT");
	registerAction(sf::Keyboard::Key::D,			"RIGHT");
	registerAction(sf::Keyboard::Key::S,			"CROUCH");
	//registerAction((sf::Keyboard::Key)sf::Mouse::Button::Right,		"SPECIAL");

	m_gridText.setCharacterSize(24);

	loadLevel(levelPath);
}

// IMPORTANT: Always add the CAnimation component first so that gridToMidPixel can compute correctly
Vec2 Scene_Play::gridToMidPixel(float gridX, float gridY, Entity entity)
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

Vec2 Scene_Play::windowToWorld(const Vec2& windowPos) const
{
	auto view = m_game->window().getView();

	float wx = view.getCenter().x - (m_game->window().getSize().x / 2);
	float wy = view.getCenter().y - (m_game->window().getSize().y / 2);

	return Vec2(windowPos.x + wx, windowPos.y + wy);
}

void Scene_Play::loadLevel(const std::string& filename)
{
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

			auto tile = m_entityManager.addEntity(item);
			tile.addComponent<CAnimation>(m_game->assets().getAnimation(name), true);
			tile.addComponent<CTransform>();
			tile.addComponent<CDraggable>();
			if (item == "Decoration")
			{
				tile.getComponent<CTransform>().scale = { 5.0, 7.0 };
			}
			tile.getComponent<CTransform>().pos = gridToMidPixel(tileGX, tileGY, tile);
			if (item == "Tile")
			{
				// Decorations should not have a bounding box
				tile.addComponent<CBoundingBox>(Vec2(tile.getComponent<CAnimation>().animation.getSize().x, tile.getComponent<CAnimation>().animation.getSize().y));
				tile.addComponent<CState>("ALIVE");
			}
			if (item == "Ladder")
			{
				tile.addComponent<CBoundingBox>(Vec2(tile.getComponent<CAnimation>().animation.getSize().x / 2.0f, tile.getComponent<CAnimation>().animation.getSize().y));
				tile.addComponent<CClimbable>();
			}
		}
		//else if (item == "Ladder")
		//{
		//	// Create a CClimbable component to be used here to allow the player to climb up the ladder
		//}
		else if (item == "Enemy")
		{
			int tileGX = 0, tileGY = 0, damage = 0;
			std::string name;

			fin >> name >> tileGX >> tileGY >> damage;

			auto enemy = m_entityManager.addEntity("Enemy");
			enemy.addComponent<CAnimation>(m_game->assets().getAnimation(name), true);
			enemy.addComponent<CTransform>();
			enemy.addComponent<CBoundingBox>(Vec2(enemy.getComponent<CAnimation>().animation.getSize().x * 0.85f, enemy.getComponent<CAnimation>().animation.getSize().y * 0.85f));
			enemy.getComponent<CTransform>().pos = gridToMidPixel(tileGX, tileGY, enemy);
			enemy.getComponent<CTransform>().pos.y += enemy.getComponent<CAnimation>().animation.getSize().y * 0.15f;
			enemy.addComponent<CState>("ALIVE");
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

void Scene_Play::spawnPlayer()
{
	m_player = m_entityManager.addEntity("Player");
	m_player.addComponent<CAnimation>(m_game->assets().getAnimation("PlayerJump"), true);
	m_player.addComponent<CTransform>(Vec2(gridToMidPixel(m_playerConfig.gridX, m_playerConfig.gridY, m_player)));
	m_player.addComponent<CState>().state = "JUMPING";
	m_player.addComponent<CBoundingBox>(Vec2(m_playerConfig.collisionX, m_playerConfig.collisionY));
	m_player.addComponent<CGravity>(m_playerConfig.gravity);
}

void Scene_Play::spawnBullet(Entity entity)
{
	auto bullet = m_entityManager.addEntity("Bullet");
	bullet.addComponent<CTransform>(Vec2(entity.getComponent<CTransform>().pos.x, entity.getComponent<CTransform>().pos.y));
	bullet.getComponent<CTransform>().scale = entity.getComponent<CTransform>().scale;
	bullet.getComponent<CTransform>().velocity.x = bullet.getComponent<CTransform>().scale.x * 15;
	bullet.addComponent<CAnimation>(m_game->assets().getAnimation("BulletAlive"), true);
	bullet.addComponent<CBoundingBox>(bullet.getComponent<CAnimation>().animation.getSize() * 0.90f);
	bullet.addComponent<CState>("ALIVE");
	bullet.addComponent<CLifespan>(600, m_currentFrame);
}

void Scene_Play::update()
{
	m_entityManager.update();

	// TODO: Implement pause functionality
	sDragAndDrop();
	sLifespan();
	sMovement();
	sCollision();
	sStatus();
	sAnimation();
	sCamera();
	sRender();

	m_currentFrame++;
}

void Scene_Play::sDragAndDrop()
{
	for (auto e : m_entityManager.getEntities())
	{
		if (e.hasComponent<CDraggable>() && e.getComponent<CDraggable>().dragging)
		{
			e.getComponent<CTransform>().pos = windowToWorld(m_mPos);
		}
	}
}

void Scene_Play::sLifespan()
{
	for (auto e : m_entityManager.getEntities())
	{
		if (e.hasComponent<CLifespan>())
		{
			if (m_currentFrame - e.getComponent<CLifespan>().frameCreated > e.getComponent<CLifespan>().lifespan)
			{
				e.destroy();
			}
		}
	}
}

void Scene_Play::sCamera()
{
	// TODO: Keep Camera on player unless player runs left / falls down a hole / enters a gate
	// Set the viewport of the window to be centered on the player if it's far enough right
	auto& pPos = m_player.getComponent<CTransform>().pos;
	float windowCenterX = std::max(m_game->window().getSize().x / 2.0f, pPos.x);
	sf::View view = m_game->window().getView();
	view.setCenter({ windowCenterX, m_game->window().getSize().y - view.getCenter().y });
	m_game->window().setView(view);
}

void Scene_Play::sStatus()
{
	// PLAYER STATES: Will need to conver this to an enum later
	// - STANDING
	// - JUMPING
	// - RUNNING
	// - SHOOTING
	// - CROUCHING

	// BULLET STATES:
	// - LIVING
	// - DYING // Dying takes time, set animation depending on frames for the animation, once the animation ends, sAnimation() calls destroy() on the entity

	// BLOCK STATES:
	// - LIVING
	// - DYING // Dying takes time, set animation depending on frames for the animation, once the animation ends, sAnimation() calls destroy() on the entity

	for (auto e : m_entityManager.getEntities())
	{
		if (e.hasComponent<CState>())
		{
			if (e.tag() == "Player")
			{
				if (e.getComponent<CState>().state == "IDLE" && e.getComponent<CAnimation>().animation.getName() != "PlayerIdle")
				{
					e.getComponent<CAnimation>().animation = m_game->assets().getAnimation("PlayerIdle");
				}
				else if (e.getComponent<CState>().state == "JUMPING" && e.getComponent<CAnimation>().animation.getName() != "PlayerJump")
				{
					e.getComponent<CAnimation>().animation = m_game->assets().getAnimation("PlayerJump");
				}
				else if (e.getComponent<CState>().state == "RUNNING" && e.getComponent<CAnimation>().animation.getName() != "PlayerRun")
				{
					e.getComponent<CAnimation>().animation = m_game->assets().getAnimation("PlayerRun");
				}
				else if (e.getComponent<CState>().state == "SHOOTING")
				{
					// TODO
				}
				else if (e.getComponent<CState>().state == "CROUCHING")
				{
					// TODO
				}
				else if (e.getComponent<CState>().state == "CLIMBING")
				{
					// TODO
					//e.getComponent<CAnimation>().animation = m_game->assets().getAnimation("PlayerClimb");
					std::cout << "TEST WORKING" << std::endl;
				}
			}
			else if (e.tag() == "Bullet")
			{
				if (e.getComponent<CState>().state == "DEAD" && e.getComponent<CAnimation>().animation.getName() != "BulletDead")
				{
					e.getComponent<CTransform>().velocity = Vec2(0, 0);
					e.getComponent<CAnimation>().animation = m_game->assets().getAnimation("BulletDead");
					e.getComponent<CAnimation>().repeat = false;
				}
			}
			else if (e.tag() == "Tile")
			{
				if (e.getComponent<CState>().state == "DEAD" && e.getComponent<CAnimation>().animation.getName() != "GroundDead")
				{
					e.getComponent<CAnimation>().animation = m_game->assets().getAnimation("GroundDead");
					e.getComponent<CAnimation>().repeat = false;
				}
			}
		}
	}
}

void Scene_Play::sMovement()
{
	if (m_pIsOnGround && !m_player.getComponent<CInput>().jump)
	{
		m_player.getComponent<CInput>().canJump = true;
	}

	if (m_player.getComponent<CInput>().shoot && m_player.getComponent<CInput>().canShoot)
	{
		spawnBullet(m_player);
		m_player.getComponent<CInput>().canShoot = false;
	}

	for (auto e : m_entityManager.getEntities())
	{
		// Before we do anything, make a copy of the entity's position
		e.getComponent<CTransform>().prevPos = e.getComponent<CTransform>().pos;

		if (e.hasComponent<CGravity>())
		{
			if (e.tag() == "Player")
			{
				// Player is colliding with a climbable object and is holding the "W" key to climb
				if (e.getComponent<CInput>().canClimb && e.getComponent<CInput>().up)
				{
					e.getComponent<CState>().state = "CLIMBING";
					e.getComponent<CTransform>().velocity.y = -5.0f;
				}
				else
				{
					// SET PLAYER X-VELOCITY
					if (e.getComponent<CInput>().right || e.getComponent<CInput>().left)
					{
						e.getComponent<CTransform>().velocity.x = e.getComponent<CInput>().right ? m_playerConfig.speedX : -m_playerConfig.speedX;
						e.getComponent<CTransform>().scale.x = e.getComponent<CInput>().right ? 1 : -1;
						if (m_pIsOnGround)
						{
							e.getComponent<CState>().state = "RUNNING";
						}
					}
					else if (!e.getComponent<CInput>().right && !e.getComponent<CInput>().left)
					{
						e.getComponent<CTransform>().velocity.x = 0.0f;
						if (m_pIsOnGround)
						{
							e.getComponent<CState>().state = "IDLE";
						}
					}

					// SET PLAYER Y-VELOCITY
					if (e.getComponent<CInput>().jump && e.getComponent<CInput>().canJump && m_pIsOnGround)
					{
						e.getComponent<CTransform>().velocity.y = m_playerConfig.speedY;
						e.getComponent<CState>().state = "JUMPING";
						e.getComponent<CInput>().canJump = false;
						m_pIsOnGround = false;
					}
					else if (!e.getComponent<CInput>().canJump && !m_pIsOnGround && e.getComponent<CInput>().jump)
					{
						e.getComponent<CTransform>().velocity.y += e.getComponent<CTransform>().velocity.y + e.getComponent<CGravity>().gravity;
					}
					else if (!e.getComponent<CInput>().canJump && !m_pIsOnGround && !e.getComponent<CInput>().jump)
					{
						e.getComponent<CTransform>().velocity.y += e.getComponent<CGravity>().gravity;
					}
					else
					{
						e.getComponent<CTransform>().velocity.y += e.getComponent<CGravity>().gravity;
					}

					e.getComponent<CGravity>().gravity *= 1.1;
				}
			}
			else
			{
				e.getComponent<CTransform>().velocity.y += e.getComponent<CGravity>().gravity;
			}
		}

		// Cap entities speed in all directions using player's max speed (ideally entities should have their own max speed)
		if (e.getComponent<CTransform>().velocity.x > m_playerConfig.maxSpeed)
		{
			e.getComponent<CTransform>().velocity.x = m_playerConfig.maxSpeed;
		}
		if (e.getComponent<CTransform>().velocity.x < -m_playerConfig.maxSpeed)
		{
			e.getComponent<CTransform>().velocity.x = -m_playerConfig.maxSpeed;
		}
		if (e.getComponent<CTransform>().velocity.y > m_playerConfig.maxSpeed)
		{
			e.getComponent<CTransform>().velocity.y = m_playerConfig.maxSpeed;
		}
		if (e.getComponent<CTransform>().velocity.y < -m_playerConfig.maxSpeed)
		{
			e.getComponent<CTransform>().velocity.y = -m_playerConfig.maxSpeed;
		}


		// Velocity has been managed, now update entity position using the updated volocity
		e.getComponent<CTransform>().pos += e.getComponent<CTransform>().velocity;
	}
}

void Scene_Play::sCollision()
{
	// Player collision with tiles
	Vec2 overlap(0, 0);
	auto& pPos = m_player.getComponent<CTransform>();

	for (auto e : m_entityManager.getEntities("Tile"))
	{
		overlap = Physics::GetOverlap(e, m_player);
		// Collision detected
		if (overlap.x > 0 && overlap.y > 0)
		{
			if (overlap.x > overlap.y)
			{
				// Vertical collision since the overlap of x is greater than the overlap of y
				// Resolve y-direction since this is the primary overlap
				if (pPos.pos.y > pPos.prevPos.y)
				{
					// Falling into a block
					pPos.pos.y -= overlap.y;
					pPos.velocity.y = 0.0f;
					m_player.getComponent<CGravity>().gravity = m_playerConfig.gravity;
					m_pIsOnGround = true;
				}
				else
				{
					// Jumping into a block
					pPos.pos.y += overlap.y;
					pPos.velocity.y = 0.0f;
					m_pIsOnGround = false;
				}
			}
			overlap = Physics::GetOverlap(e, m_player);
			if (overlap.x > 0 && overlap.y > 0)
			{
				// Horizontal overlap since the overlap of y is greater than overlap of x
				// Resolve x-direction since this is the primary overlap
				if (pPos.pos.x > pPos.prevPos.x)
				{
					// Running to the right
					pPos.pos.x -= overlap.x;
				}
				else if (pPos.pos.x < pPos.prevPos.x)
				{
					// Running to the left
					pPos.pos.x += overlap.x;
				}
			}
		}

		for (auto b : m_entityManager.getEntities("Bullet"))
		{
			overlap = Physics::GetOverlap(e, b);
			if (overlap.x > 0 && overlap.y > 0)
			{
				b.getComponent<CState>().state = "DEAD";
				e.getComponent<CState>().state = "DEAD";
			}
		}
	}

	for (auto e : m_entityManager.getEntities("Ladder"))
	{
		overlap = Physics::GetOverlap(m_player, e);
		if (overlap.x > 0 && overlap.y > 0)
		{
			// If our overlap in the x-direction is > 0 for any "Ladder", we can climb - don't loop over the rest of the "Ladders"
			m_player.getComponent<CInput>().canClimb = true;
			break;
		}
		else
		{
			m_player.getComponent<CInput>().canClimb = false;
		}
	}

	// Not using bounding box here since we want the player to be off screen before we respawn them
	if (m_player.getComponent<CTransform>().pos.y > m_game->window().getSize().y + m_player.getComponent<CAnimation>().animation.getSize().y)
	{
		m_player.getComponent<CTransform>().pos = gridToMidPixel(m_playerConfig.gridX, m_playerConfig.gridY, m_player);
	}
	// Player can not walk off the left side of the screen
	if (m_player.getComponent<CTransform>().pos.x - m_player.getComponent<CBoundingBox>().halfSize.x < 0)
	{
		m_player.getComponent<CTransform>().pos.x = m_player.getComponent<CBoundingBox>().halfSize.x;
	}
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
		if (action.name() == "CLIMB")				{ m_player.getComponent<CInput>().up = true; }
		if (action.name() == "JUMP")				{ m_player.getComponent<CInput>().jump = true; }
		if (action.name() == "CROUCH")				{ m_player.getComponent<CInput>().down = true; }
		if (action.name() == "LEFT")				{ m_player.getComponent<CInput>().left = true; }
		if (action.name() == "RIGHT")				{ m_player.getComponent<CInput>().right = true; }
		if (action.name() == "SHOOT")				{ m_player.getComponent<CInput>().shoot = true; }
		if (action.name() == "SPECIAL")				{ m_player.getComponent<CInput>().special = true; }
		if (action.name() == "LEFT_CLICK")			
		{ 
			Vec2 worldPos = windowToWorld(action.pos());
			//std::cout << "Mouse clicked at: " << worldPos.x << ", " << worldPos.y << std::endl;
			for (auto e : m_entityManager.getEntities())
			{
				if (e.hasComponent<CDraggable>() && isInside(worldPos, e))
				{
					std::cout << "CLICKED ON ENTITY: " << e.getComponent<CAnimation>().animation.getName() << std::endl;
					e.getComponent<CDraggable>().dragging = !e.getComponent<CDraggable>().dragging;
				}
			}
		}
		if (action.name() == "MOUSE_MOVE")
		{ 
			m_mPos = action.pos();
			m_mouseShape.setPosition({ m_mPos.x, m_mPos.y });
		}
	}
	else if (action.type() == "END")
	{
		if (action.name() == "CLIMB")				{ m_player.getComponent<CInput>().up = false; }
		if (action.name() == "JUMP")				{ m_player.getComponent<CInput>().jump = false; }
		if (action.name() == "CROUCH")				{ m_player.getComponent<CInput>().down = false; }
		if (action.name() == "LEFT")				{ m_player.getComponent<CInput>().left = false; }
		if (action.name() == "RIGHT")				{ m_player.getComponent<CInput>().right = false; }
		if (action.name() == "SHOOT") 
		{
			m_player.getComponent<CInput>().canShoot = true;
			m_player.getComponent<CInput>().shoot = false;
		}
	}
}

void Scene_Play::sAnimation()
{
	for (auto e : m_entityManager.getEntities())
	{
		if (e.hasComponent<CAnimation>())
		{
			if (e.getComponent<CAnimation>().repeat)
			{
				e.getComponent<CAnimation>().animation.update();
			}
			else
			{
				if (e.getComponent<CAnimation>().animation.hasEnded())
				{
					e.destroy();
				}
				else
				{
					e.getComponent<CAnimation>().animation.update();
				}
			}
		}
	}
}

void Scene_Play::sRender()
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
	}

	// Draw all Entity collision bounding boxes with a rectangleShape
	if (m_drawCollision)
	{
		for (auto e : m_entityManager.getEntities())
		{
			if (e.hasComponent<CBoundingBox>())
			{
				auto& box = e.getComponent<CBoundingBox>();
				auto& transform = e.getComponent<CTransform>();
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
	m_mouseShape.setFillColor(sf::Color(255, 0, 0));
	m_mouseShape.setRadius(4);
	m_mouseShape.setOrigin({ 2, 2 });
	Vec2 worldPos = windowToWorld(m_mPos);
	m_mouseShape.setPosition({ worldPos.x, worldPos.y });
	m_game->window().draw(m_mouseShape);
	m_currentFrame++;
}

void Scene_Play::onEnd()
{
	m_hasEnded = true;
	sf::View view = m_game->window().getView();
	view.setCenter({ m_game->window().getSize().x / 2.0f, m_game->window().getSize().y / 2.0f });
	m_game->window().setView(view);
	m_game->changeScene("MENU", nullptr, true);
}