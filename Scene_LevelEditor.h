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
	bool								m_canSave = true;
	bool								m_drawTextures = true;
	bool								m_drawGrid = false;
	const Vec2							m_gridSize = { 64, 64 };
	sf::Text							m_gridText;

	Vec2								m_camPos;
	Vec2								m_mPos;
	sf::CircleShape						m_mouseShape;
	sf::RectangleShape					m_poolBackground;

	std::vector<std::string>			m_tileSheet;
	std::shared_ptr<std::vector<sf::Sprite>> m_spriteSheet = std::make_shared<std::vector<sf::Sprite>>();;

	Vec2 gridToMidPixel(float gridX, float gridY, Entity e);
	Vec2 mouseToGrid(Vec2 m_mPos, Entity e);
	Vec2 windowToWorld(const Vec2& windowPos) const;

	void init(const std::string& levelPath);
	void loadLevel(const std::string& filename);
	void saveLevel(const std::string& filename);
	void loadTileSheet(const std::string& tilesheet);
	void onEnd();
	void update();

	void spawnPoolBackground(sf::RectangleShape& bg);
	void spawnPlayer();
	void spawnBullet(Entity entity);

	void sDragAndDrop();
	void sMovement();
	void sTilePool();
	void sDoAction(const Action& action);
	void sCamera();

public:
	Scene_LevelEditor(GameEngine* gameEngine, const std::string& levelPath);

	void sRender();
};