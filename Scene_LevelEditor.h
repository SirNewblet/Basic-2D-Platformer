#pragma once

#include "Scene.h"
#include "EntityManager.h"

#include <map>
#include <memory>

class Scene_LevelEditor : public Scene
{

	struct PlayerConfig
	{
		float gridX = 0, gridY = 0, collisionX = 0, collisionY = 0, speedX = 0, speedY = 0, maxSpeed = 0, gravity = 0;
		std::string WEAPON;
	};

protected:
	Entity								m_player;
	PlayerConfig						m_playerConfig;
	std::string							m_levelPath;
	std::string							m_lastAction;
	std::string							m_filename;
	bool								m_drawTextures = true;
	bool								m_drawGrid = false;
	const Vec2							m_gridSize = { 64, 64 };
	sf::Text							m_gridText;

	Vec2								m_mPos;
	sf::CircleShape						m_mouseShape;

	Vec2 gridToMidPixel(float gridX, float gridY, Entity e);
	Vec2 mouseToGrid(Vec2 m_mPos, Entity e);
	Vec2 windowToWorld(const Vec2& windowPos) const;

	void init(const std::string& levelPath);
	void loadLevel(const std::string& filename);
	bool saveLevel(const std::string& filename);
	void onEnd();
	void update();

	void spawnPlayer();
	void spawnBullet(Entity entity);

	void sDragAndDrop();
	void sMovement();
	void sDoAction(const Action& action);
	void sCamera();

public:
	Scene_LevelEditor(GameEngine* gameEngine, const std::string& levelPath);

	void sRender();
};