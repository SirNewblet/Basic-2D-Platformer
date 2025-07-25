#pragma once

#include "Entity.h"

struct Intersect
{
	bool intersected = false;
	Vec2 point;
};

class Physics
{
public:

	Physics() {}

	Vec2 static GetOverlap(Entity a, Entity b);
	Vec2 static GetPreviousOverlap(Entity a, Entity b);
	bool static IsInside(const Vec2& pos, Entity e);
	Intersect LineIntersect(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d);
	bool EntityIntersect(const Vec2& a, const Vec2& b, Entity e);
};