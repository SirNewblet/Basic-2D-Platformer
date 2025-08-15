#pragma once

#include "Animation.h"

class Component
{
public:
	bool has = false;
};

class CAnimation : public Component
{
public:
	Animation animation;
	bool repeat = false;
	CAnimation() {};
	CAnimation(const Animation& ani) :
		animation(ani), repeat(false) { }
	CAnimation(const Animation& ani, bool r) :
		animation(ani), repeat(r) { }
};

class CAttacking : public Component
{
public:
	std::string attackType = "";
	float attackRange = 0.0f;
	bool canAttack = true;
	bool isAttacking = false;
	bool isInReach = false;
	int started = 0;
	int duration = 0;
	int coolDown = 0;
	CAttacking() {};
};

class CBoundingBox : public Component
{
public:
	Vec2 size;
	Vec2 halfSize;
	CBoundingBox() {};
	CBoundingBox(const Vec2& s) :
		size(s), halfSize(s.x / 2, s.y / 2) { }
};

class CClimbable : public Component
{
public:
	CClimbable() {};
};

class CDamage : public Component
{
public:
	float damage = 0;
	CDamage() {};
	CDamage(float d) : damage(d) {};
};

class CDestroyable : public Component
{
public:
	CDestroyable() {};
};

class CDraggable : public Component
{
public:
	bool dragging = false;
	CDraggable() {};
};

class CGravity : public Component
{
public:
	float gravity = 0.0f;
	CGravity() {};
	CGravity(float g) : gravity(g) {}
};

class CGridLocation : public Component
{
public:
	int x = 0, y = 0;
	CGridLocation() {};
	CGridLocation(int gridX, int gridY) :
		x(gridX), y(gridY) 
	{ };
};

class CHealth : public Component
{
public:
	float maxHealth = 100;
	float currentHealth = 100;

	CHealth() {};
	CHealth(float mh) :
		maxHealth(mh),
		currentHealth(mh)
	{ };
};

class CInput : public Component
{
public:
	bool up = false;
	bool jump = false;
	bool down = false;
	bool left = false;
	bool right = false;
	bool shoot = false;
	bool special = false;
	bool canShoot = false;
	bool canClimb = false;
	bool canJump = false;

	CInput() {};
};

class CInvulnerable : public Component
{
public:
	// 180 frames = 3 seconds of invulnerability @ 60 fps
	int invulnerableFrames = 180;
	int frameCreated = 0;
	bool isInvulnerable = false;

	CInvulnerable() {};
	CInvulnerable(int duration, int frame) :
		invulnerableFrames(duration), frameCreated(frame) 
	{ }
};

class CLifespan : public Component
{
public:
	int lifespan = 0;
	int frameCreated = 0;
	CLifespan() {};
	CLifespan(int duration, int frame) :
		lifespan(duration), frameCreated(frame) 
	{ }
};

class CRayCaster : public Component
{
public:
	Vec2 source = { 0.0, 0.0 };
	std::vector<Vec2> targets;
	float maxRange = 0;
	float angle = 0;
	bool drawBetween = false;
	bool drawLine = true;
	
	CRayCaster() {};
	CRayCaster(Vec2 origin) : source(origin) {};
	CRayCaster(Vec2 origin, std::vector<Vec2> endPoints) :
		source(origin), targets(endPoints) {}
};

class CState : public Component
{
public:
	std::string state = "";
	CState() {};
	CState(const std::string& s) : state(s) {}
};

class CTransform : public Component
{
public:
	Vec2 pos = { 0.0, 0.0 };
	Vec2 prevPos = { 0.0, 0.0 };
	Vec2 scale = { 1.0, 1.0 };
	Vec2 velocity = { 0.0, 0.0 };
	Vec2 facing = { 0.0, 1.0 };
	float angle = 0;

	CTransform() {};
	CTransform(const Vec2& p) :
		pos(p) {
	}
	CTransform(const Vec2& p, const Vec2& sp, const Vec2& sc, float a) :
		pos(p), prevPos(p), velocity(sp), scale(sc), angle(a) {
	}
};