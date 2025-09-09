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
	std::string entityType = "";

	while (fin >> entityType)
	{
		if (entityType == "Tile" || entityType == "Decoration" || entityType == "Ladder" || entityType == "Destroyable")
		{
			
			int tileGX = 0, tileGY = 0;
			std::string name;

			fin >> name >> tileGX >> tileGY;

			auto entity = m_entityManager.addEntity(entityType);
			entity.addComponent<CAnimation>(m_game->assets().getAnimation(name), true);
			entity.addComponent<CTransform>();
			entity.getComponent<CTransform>().pos = gridToMidPixel(tileGX, tileGY, entity);
			entity.addComponent<CDraggable>();
			if (entityType == "Decoration")
			{
				// TODO: replace hard-coded values with values from the config (OR - draw decorations in the correct pixel scale)
			}
			if (entityType == "Tile" || entityType == "Destroyable")
			{
				// Decorations should not have a bounding box
				entity.addComponent<CBoundingBox>(Vec2(entity.getComponent<CAnimation>().animation.getSize().x, entity.getComponent<CAnimation>().animation.getSize().y));
				entity.addComponent<CState>("ALIVE");

				if (entityType == "Destroyable")
				{
					entity.addComponent<CDestroyable>();
				}
			}
			if (entityType == "Ladder")
			{
				entity.addComponent<CBoundingBox>(Vec2(entity.getComponent<CAnimation>().animation.getSize().x / 2.0f, entity.getComponent<CAnimation>().animation.getSize().y));
				entity.addComponent<CClimbable>();
			}
		}
		else if (entityType == "Enemy")
		{
			EnemyConfig enemyConfig;

			fin
				>> enemyConfig.enemyType
				>> enemyConfig.animationName
				>> enemyConfig.gridX
				>> enemyConfig.gridY
				>> enemyConfig.collisionX
				>> enemyConfig.collisionY
				>> enemyConfig.speedX
				>> enemyConfig.speedY
				>> enemyConfig.health
				>> enemyConfig.damage
				>> enemyConfig.attackType
				>> enemyConfig.attackDelay
				>> enemyConfig.gravity;

			spawnEnemy(enemyConfig);
		}
		else if (entityType == "Player")
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
				>> m_playerConfig.health
				>> m_playerConfig.WEAPON;
		}
	}

	spawnPlayer();
}

void Scene_Play::spawnPlayer()
{
	m_player = m_entityManager.addEntity("Player");
	m_player.addComponent<CAnimation>(m_game->assets().getAnimation("PlayerJump"), true);
	m_player.addComponent<CTransform>(Vec2(gridToMidPixel(m_playerConfig.gridX, m_playerConfig.gridY, m_player)));
	m_player.addComponent<CState>().state = "JUMPING";
	m_player.addComponent<CBoundingBox>(Vec2(m_playerConfig.collisionX, m_playerConfig.collisionY));
	m_player.addComponent<CGravity>(m_playerConfig.gravity);
	m_player.addComponent<CInvulnerable>();
	m_player.addComponent<CHealth>();
	m_player.getComponent<CHealth>().currentHealth = 5;
	m_player.getComponent<CHealth>().maxHealth = 5;
}

void Scene_Play::spawnEnemy(EnemyConfig& enemy)
{
	auto entity = m_entityManager.addEntity("Enemy");
	entity.addComponent<CState>("ALIVE");
	entity.addComponent<CAnimation>(m_game->assets().getAnimation(enemy.animationName), true);
	entity.addComponent<CTransform>();
	entity.getComponent<CTransform>().pos = gridToMidPixel(enemy.gridX, enemy.gridY, entity);
	entity.getComponent<CTransform>().pos.y += (enemy.collisionY * 0.25f);
	entity.addComponent<CBoundingBox>(Vec2(enemy.collisionX * 0.75f, enemy.collisionY * 0.75f));
	entity.addComponent<CHealth>(enemy.health);
	entity.addComponent<CDamage>(enemy.damage);
	entity.addComponent<CAttacking>();
	entity.getComponent<CAttacking>().attackType = enemy.attackType;
	entity.getComponent<CAttacking>().coolDown = enemy.attackDelay;
	entity.addComponent<CGravity>().gravity = enemy.gravity;

	if (entity.getComponent<CAttacking>().attackType == "HITSCAN")
	{
		entity.addComponent<CRayCaster>(Vec2(entity.getComponent<CTransform>().pos.x, entity.getComponent<CTransform>().pos.y));
		entity.getComponent<CRayCaster>().maxRange = m_game->window().getView().getSize().x / 2;
	}
	else if (entity.getComponent<CAttacking>().attackType == "PROJECTILE")
	{

	}
	else if (entity.getComponent<CAttacking>().attackType == "MELEE")
	{

	}
	else if (entity.getComponent<CAttacking>().attackType == "CUSTOM")
	{
		// Check if enemy is a boss
		// Check what boss it is and attach relevant components
	}
}

void Scene_Play::spawnBullet(Entity entity)
{
	auto bullet = m_entityManager.addEntity("Bullet");
	bullet.addComponent<CTransform>(Vec2(entity.getComponent<CTransform>().pos.x, entity.getComponent<CTransform>().pos.y));
	bullet.getComponent<CTransform>().scale = entity.getComponent<CTransform>().scale;
	bullet.getComponent<CTransform>().velocity.x = bullet.getComponent<CTransform>().scale.x * 15;
	bullet.addComponent<CAnimation>(m_game->assets().getAnimation("BulletIdle"), true);
	bullet.addComponent<CBoundingBox>(bullet.getComponent<CAnimation>().animation.getSize() * 0.90f);
	bullet.addComponent<CDamage>(10);
	bullet.addComponent<CState>("ALIVE");
	bullet.addComponent<CLifespan>(45, m_currentFrame);
}

void Scene_Play::update()
{
	for (auto e : m_entityManager.getEntities("Enemy"))
	{
		if (e.isActive())
		{
			m_gameOver = false;
			break;
		}
	}

	if (!m_gameOver)
	{
		if (!m_paused)
		{
			m_entityManager.update();
			//sDragAndDrop();
			sLifespan();
			sMovement();
			sEnemyLogic();
			sCollision();
			sStatus();
			sAnimation();
			sCamera();
			m_currentFrame++;
		}

		sRender();
		m_gameOver = true;
	}
	else
	{
		// Player has defeated all enemies
		// TODO: Save player progress (next level unlocked)


		std::cout << "Game Over.\n";
	}
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

/// <summary>
/// Method to handle lifespan of entities and events. This includes invulnerable frames, attack delay, acid pools, etc.
/// </summary>
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

		if (e.hasComponent<CInvulnerable>())
		{
			if (m_currentFrame - e.getComponent<CInvulnerable>().frameCreated > e.getComponent<CInvulnerable>().invulnerableFrames)
			{
				e.getComponent<CInvulnerable>().isInvulnerable = false;
			}
		}

		if (e.hasComponent<CAttacking>() && e.getComponent<CState>().state != "DEAD")
		{
			if (m_currentFrame - e.getComponent<CAttacking>().started > e.getComponent<CAttacking>().duration)
			{
				e.getComponent<CAttacking>().isAttacking = false;
				e.getComponent<CTransform>().velocity.x = 0;
				e.getComponent<CState>().state = "IDLE";

				if (m_currentFrame - e.getComponent<CAttacking>().started > e.getComponent<CAttacking>().duration + e.getComponent<CAttacking>().coolDown)
				{
					e.getComponent<CAttacking>().canAttack = true;
				}
			}
		}
	}
}

void Scene_Play::sCamera()
{
	float camYVelocity = 8.0f;
	// TODO: Keep Camera on player unless player runs left / falls down a hole / enters a gate
	// Set the viewport of the window to be centered on the player if it's far enough right
	auto& pPos = m_player.getComponent<CTransform>().pos;
	float windowCenterX = std::max(m_game->window().getSize().x / 2.0f, pPos.x);
	sf::View view = m_game->window().getView();
	float windowCenterY = view.getCenter().y;

	if (pPos.y < view.getCenter().y * 0.8f)
	{
		windowCenterY -= camYVelocity;
	}
	else if (pPos.y > view.getCenter().y)
	{
		windowCenterY += camYVelocity;
	}

	view.setCenter({ windowCenterX, windowCenterY });
	m_game->window().setView(view);
}

void Scene_Play::sEnemyLogic()
{
	for (auto e : m_entityManager.getEntities("Enemy"))
	{
		if (e.hasComponent<CAttacking>())
		{
			e.getComponent<CAttacking>().isInReach = (abs(m_player.getComponent<CTransform>().pos.x - e.getComponent<CTransform>().pos.x) < m_game->window().getView().getSize().x * 0.50f);
			e.getComponent<CTransform>().scale.x = (m_player.getComponent<CTransform>().pos.x < e.getComponent<CTransform>().pos.x) ? 1 : -1;

			if (e.getComponent<CAttacking>().isInReach && e.getComponent<CAttacking>().attackType == "HITSCAN")
			{
				e.getComponent<CRayCaster>().target = m_player.getComponent<CTransform>().pos;
			}
			if (e.getComponent<CAttacking>().isInReach && e.getComponent<CAttacking>().canAttack)
			{
				e.getComponent<CAttacking>().canAttack = false;
				e.getComponent<CAttacking>().started = m_currentFrame;

				if (e.getComponent<CAttacking>().attackType == "HITSCAN")
				{
					// Specify hitscan states here
				}
				else if (e.getComponent<CAttacking>().attackType == "PROJECTILE")
				{

				}
				else if (e.getComponent<CAttacking>().attackType == "MELEE")
				{

				}
				else if (e.getComponent<CAttacking>().attackType == "CUSTOM")
				{
					// Unsure of best way to implement custom enemy logic...
					// Moves need to be timed and coordinated and bosses need "phases"
					e.getComponent<CTransform>().velocity.x = 20 * -(e.getComponent<CTransform>().scale.x);
					e.getComponent<CState>().state = "RUSH";
				}
			}
		}
	}
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
			try
			{
				if (e.getComponent<CState>().state == "RUSH" && e.getComponent<CAnimation>().animation.getType() != "RUSH")
				{
					e.getComponent<CAnimation>().animation = m_game->assets().getAnimation(e.getComponent<CAnimation>().animation.getEntityName() + "Rush");
				}
				else if (e.getComponent<CState>().state == "IDLE" && e.getComponent<CAnimation>().animation.getType() != "IDLE")
				{
					e.getComponent<CAnimation>().animation = m_game->assets().getAnimation(e.getComponent<CAnimation>().animation.getEntityName() + "Idle");
				}
				else if (e.getComponent<CState>().state == "SHOOTING" && e.getComponent<CAnimation>().animation.getType() != "SHOOT")
				{
					// TODO
					//e.getComponent<CAnimation>().animation = m_game->assets().getAnimation(e.getComponent<CAnimation>().animation.getEntityName() + "Shoot");
				}
				else if (e.getComponent<CState>().state == "CROUCHING" && e.getComponent<CAnimation>().animation.getType() != "CROUCH")
				{
					// TODO
					//e.getComponent<CAnimation>().animation = m_game->assets().getAnimation(e.getComponent<CAnimation>().animation.getEntityName() + "Crouch");
				}
				else if (e.getComponent<CState>().state == "CLIMBING" && e.getComponent<CAnimation>().animation.getType() != "CLIMB")
				{
					// TODO
					//e.getComponent<CAnimation>().animation = m_game->assets().getAnimation(e.getComponent<CAnimation>().animation.getEntityName() + "Climb");
				}
				else if (e.getComponent<CState>().state == "JUMPING" && e.getComponent<CAnimation>().animation.getType() != "JUMP")
				{
					e.getComponent<CAnimation>().animation = m_game->assets().getAnimation(e.getComponent<CAnimation>().animation.getEntityName() + "Jump");
				}
				else if (e.getComponent<CState>().state == "RUNNING" && e.getComponent<CAnimation>().animation.getType() != "RUN")
				{
					e.getComponent<CAnimation>().animation = m_game->assets().getAnimation(e.getComponent<CAnimation>().animation.getEntityName() + "Run");
				}
				else if (e.getComponent<CState>().state == "DEAD" && e.getComponent<CAnimation>().animation.getType() != "DEAD")
				{
					// TODO: Create enemy death animation
					e.getComponent<CAnimation>().animation = m_game->assets().getAnimation(e.getComponent<CAnimation>().animation.getEntityName() + "Dead");
					e.getComponent<CAnimation>().repeat = false;
					if (e.hasComponent<CTransform>())
					{
						e.getComponent<CTransform>().velocity = Vec2(0, 0);
					}
					if (e.hasComponent<CBoundingBox>())
					{
						e.getComponent<CBoundingBox>() = Vec2(0, 0);
					}
				}

				if (e.hasComponent<CAttacking>() && e.getComponent<CAnimation>().animation.getType() == "RUSH")
				{
					e.getComponent<CAttacking>().duration = e.getComponent<CAnimation>().animation.getDuration();
				}
			}
			catch (const std::exception& ex)
			{
				e.getComponent<CAnimation>().animation = m_game->assets().getAnimation("BulletDead");
				std::cout << ex.what() << std::endl;
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
					e.getComponent<CTransform>().velocity.x = 0.0f;
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
	std::vector<std::string> tiles = { "Tile", "Destroyable" };
	for (auto e : m_entityManager.getEntities(tiles))
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
			if ((overlap.x > 0 && overlap.y > 0) && e.getComponent<CDestroyable>().has)
			{
				b.getComponent<CState>().state = "DEAD";
				e.getComponent<CState>().state = "DEAD";
			}
			else if (overlap.x > 0 && overlap.y > 0)
			{
				b.getComponent<CState>().state = "DEAD";
			}
		}
	}

	for (auto b : m_entityManager.getEntities("Bullet"))
	{
		for (auto e : m_entityManager.getEntities("Enemy"))
		{
			overlap = Physics::GetOverlap(b, e);
			if ((overlap.x > 0 && overlap.y > 0) && b.getComponent<CState>().state != "DEAD")
			{
				b.getComponent<CState>().state = "DEAD";
				e.getComponent<CHealth>().currentHealth -= b.getComponent<CDamage>().damage;

				if (e.getComponent<CHealth>().currentHealth <= 0)
				{
					e.getComponent<CState>().state = "DEAD";
				}
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

	for (auto e : m_entityManager.getEntities("Enemy"))
	{
		overlap = Physics::GetOverlap(m_player, e);
		if ((overlap.x > 0 && overlap.y > 0) && !m_player.getComponent<CInvulnerable>().isInvulnerable)
		{
			m_player.getComponent<CHealth>().currentHealth -= e.getComponent<CDamage>().damage;
			m_player.getComponent<CInvulnerable>().frameCreated = m_currentFrame;
			m_player.getComponent<CInvulnerable>().isInvulnerable = true;
			break;
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

	if (m_player.getComponent<CHealth>().currentHealth <= 0)
	{
		m_player.getComponent<CHealth>().currentHealth = m_player.getComponent<CHealth>().maxHealth;
		m_player.getComponent<CTransform>().pos = gridToMidPixel(m_playerConfig.gridX, m_playerConfig.gridY, m_player);
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
				if (e.hasComponent<CDraggable>() && Physics::IsInside(worldPos, e))
				{
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

void Scene_Play::sDisplayHealth()
{
	std::vector<std::string> ents = { "Enemy", "Player" };
	for (auto e : m_entityManager.getEntities(ents))
	{
		if (e.tag() == "Enemy" && e.hasComponent<CHealth>())
		{
			if (e.getComponent<CHealth>().currentHealth < e.getComponent<CHealth>().maxHealth 
				&& e.getComponent<CState>().state != "DEAD" 
				&& e.getComponent<CHealth>().currentHealth > 0)
			{
				auto entTrans = e.getComponent<CTransform>();
				auto entSize = e.getComponent<CAnimation>().animation.getSize();
				auto rectBack = sf::RectangleShape({ entSize.x, 16 });
				rectBack.setFillColor(sf::Color::Red);
				// set the position to the middle of the entity (x) and slightly above the entity (-10)
				rectBack.setPosition({ entTrans.pos.x - (entSize.x / 2), entTrans.pos.y - (entSize.y / 2) - 10});

				auto rectFront = sf::RectangleShape({ entSize.x * (e.getComponent<CHealth>().currentHealth / e.getComponent<CHealth>().maxHealth), 16 });
				rectFront.setFillColor(sf::Color::Green);
				rectFront.setPosition({ rectBack.getPosition().x, rectBack.getPosition().y });

				m_game->window().draw(rectBack);
				m_game->window().draw(rectFront);
			}
		}

		if (e.tag() == "Player" && e.hasComponent<CHealth>())
		{
			float offset = 32;
			for (int i = 0; i < e.getComponent<CHealth>().currentHealth; i++)
			{
				auto spr = sf::Sprite(m_game->assets().getAnimation("HeartFull").getSprite());
				auto pos = windowToWorld({ offset, 32 });
				spr.setPosition({ pos.x, pos.y });
				m_game->window().draw(spr);

				offset += spr.getLocalBounds().size.x;
			}
			for (int i = 0; i < e.getComponent<CHealth>().maxHealth - e.getComponent<CHealth>().currentHealth; i++)
			{
				auto spr = sf::Sprite(m_game->assets().getAnimation("HeartEmpty").getSprite());
				auto pos = windowToWorld({ offset, 32 });
				spr.setPosition({ pos.x, pos.y });
				m_game->window().draw(spr);

				offset += spr.getLocalBounds().size.x;
			}
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
		if (e.hasComponent<CInvulnerable>())
		{
			if (e.getComponent<CInvulnerable>().isInvulnerable)
			{
				// do shading of the current animation to show invulnverability
			}
		}
	}
}

void Scene_Play::sRayCast()
{
	for (auto e : m_entityManager.getEntities())
	{
		if (e.hasComponent<CRayCaster>() && e.getComponent<CAttacking>().isInReach)
		{
			for (auto t : e.getComponent<CRayCaster>().targets)
			{
				if (e.getComponent<CRayCaster>().source.dist(t) < e.getComponent<CRayCaster>().maxRange)
				{
					drawLine(e.getComponent<CRayCaster>().source, t);
				}
			}

			if (e.getComponent<CRayCaster>().source.dist(e.getComponent<CRayCaster>().target) < e.getComponent<CRayCaster>().maxRange)
			{
				drawLine(e.getComponent<CRayCaster>().source, e.getComponent<CRayCaster>().target);
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
		sRayCast();
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
		sDisplayHealth();
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
}

void Scene_Play::onEnd()
{
	m_hasEnded = true;
	sf::View view = m_game->window().getView();
	view.setCenter({ m_game->window().getSize().x / 2.0f, m_game->window().getSize().y / 2.0f });
	m_game->window().setView(view);
	m_game->changeScene("MENU", nullptr, true);
}