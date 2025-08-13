#pragma once

#include "Scene.h"
#include "EntityManager.h"

#include <map>
#include <memory>

class Scene_Play : public Scene
{
	struct PlayerConfig
	{
		float gridX = 0, gridY = 0, collisionX = 0, collisionY = 0, speedX = 0, speedY = 0, maxSpeed = 0,  gravity = 0;
		std::string WEAPON;
	};

protected:
	Entity								m_player;
	std::string							m_levelPath;
	std::string							m_lastAction;
	PlayerConfig						m_playerConfig;
	bool								m_gameOver = false;
	bool								m_pIsOnGround = false;
	bool								m_drawTextures = true;
	bool								m_drawCollision = false;
	bool								m_drawGrid = false;
	const Vec2							m_gridSize = { 64, 64 };
	sf::Text							m_gridText;

	Vec2								m_mPos;
	sf::CircleShape						m_mouseShape;

	Vec2 gridToMidPixel(float gridX, float gridY, Entity);
	Vec2 windowToWorld(const Vec2& windowPos) const;

	void init(const std::string& levelPath);
	void loadLevel(const std::string& filename);
	void onEnd();
	void update();

	void spawnPlayer();
	void spawnBullet(Entity entity);

	void sDragAndDrop();
	void sDoAction(const Action& action);
	void sMovement();
	void sStatus();
	void sEnemyLogic();
	void sAnimation();
	void sDisplayHealth();
	void sCollision();
	void sLifespan();
	void sCamera();

public:
	Scene_Play(GameEngine* gameEngine, const std::string& levelPath);

	void sRender();
};