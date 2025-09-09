#include "Physics.h"
#include <iostream>

bool Physics::IsInside(Vec2& pos, Entity e)
{
	auto ePos = e.getComponent<CTransform>().pos;
	auto eScale = e.getComponent<CTransform>().scale;
	auto size = e.getComponent<CAnimation>().animation.getSize();
	float dx = fabs(pos.x - ePos.x);
	float dy = fabs(pos.y - ePos.y);

	return ((dx <= (eScale.x * size.x / 2)) && (dy <= (eScale.y * size.y / 2)));
}

Vec2 Physics::GetOverlap(Entity a, Entity b)
{
	if (a.hasComponent<CBoundingBox>() && b.hasComponent<CBoundingBox>())
	{
		Vec2 delta(abs(b.getComponent<CTransform>().pos.x - a.getComponent<CTransform>().pos.x),
			abs(b.getComponent<CTransform>().pos.y - a.getComponent<CTransform>().pos.y));
		float XOverlap = a.getComponent<CBoundingBox>().halfSize.x + b.getComponent<CBoundingBox>().halfSize.x - delta.x;
		float YOverlap = a.getComponent<CBoundingBox>().halfSize.y + b.getComponent<CBoundingBox>().halfSize.y - delta.y;
		return Vec2(XOverlap, YOverlap);
	}
	else { return Vec2(0, 0); }
}

Vec2 Physics::GetPreviousOverlap(Entity a, Entity b)
{
	if (a.hasComponent<CBoundingBox>() && b.hasComponent<CBoundingBox>())
	{
		Vec2 delta(abs(b.getComponent<CTransform>().pos.x - a.getComponent<CTransform>().prevPos.x),
			abs(b.getComponent<CTransform>().pos.y - a.getComponent<CTransform>().prevPos.y));
		float XOverlap = a.getComponent<CBoundingBox>().halfSize.x + b.getComponent<CBoundingBox>().halfSize.x - delta.x;
		float YOverlap = a.getComponent<CBoundingBox>().halfSize.y + b.getComponent<CBoundingBox>().halfSize.y - delta.y;
		return Vec2(XOverlap, YOverlap);
	}
	else { return Vec2(0, 0); }
}

bool Physics::IsInside(const Vec2& pos, Entity e)
{
	sf::FloatRect globalBounds = e.getComponent<CAnimation>().animation.getSprite().getGlobalBounds();
	if (pos.x > globalBounds.position.x && pos.x < globalBounds.position.x + globalBounds.size.x &&
		pos.y > globalBounds.position.y && pos.y < globalBounds.position.y + globalBounds.size.y)
	{
		return true;
	}

	return false;
}

Intersect Physics::LineIntersect(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d)
{
	Vec2 r = b - a;
	Vec2 s = d - c;
	float rxs = r.cross(s);
	Vec2 cma = c - a;
	float t = cma.cross(s) / rxs;
	float u = cma.cross(r) / rxs;

	if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
	{
		return { true, Vec2(a.x + t * r.x, a.y + t * r.y) };
	}
	else
	{
		return { false, Vec2(0,0) };
	}
}

bool Physics::EntityIntersect(const Vec2& a, const Vec2& b, Entity e)
{
	sf::FloatRect globalBounds = e.getComponent<CAnimation>().animation.getSprite().getGlobalBounds();
	Vec2 topLeft = Vec2(globalBounds.position.x, globalBounds.position.y);
	Vec2 topRight = Vec2(globalBounds.position.x + globalBounds.size.x, globalBounds.position.y);
	Vec2 bottomLeft = Vec2(globalBounds.position.x, globalBounds.position.y + globalBounds.size.y);
	Vec2 bottomRight = Vec2(globalBounds.position.x + globalBounds.size.x, globalBounds.position.y + globalBounds.size.y);

	if (LineIntersect(a, b, topLeft, topRight).intersected) { return true; }
	else if (LineIntersect(a, b, topRight, bottomRight).intersected) { return true; }
	else if (LineIntersect(a, b, bottomRight, bottomLeft).intersected) { return true; }
	else if (LineIntersect(a, b, bottomLeft, topLeft).intersected) { return true; }

	return false;
}