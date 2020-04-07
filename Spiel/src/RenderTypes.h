#pragma once

#include <vector>
#include <string>

#include "BaseTypes.h"
#include "Camera.h"
#include "Window.h"

struct Basis {
	/* x y world coordinates, z depth*/
	vec2 position;
	/* in radiants 2pi = one rotation*/
	float rotation;

	Basis() :
		position{ 0.0f, 0.0f },
		rotation{ 0.0f }
	{
	}

	Basis(vec2 position_, float rotation_) :
		position{ position_ },
		rotation{ rotation_ }
	{
	}

	inline vec2 getPos() const { return position; }
	inline float getRota() const { return rotation; }
};



class Drawable : virtual public Basis {
public:
	vec4 color;
	vec2 scale;
	float drawingPrio;
	uint32_t id;
	Form form;
	bool throwsShadow;

	Drawable(uint32_t id_, vec2 position_, float drawingPrio_, vec2 scale_, vec4 color_, Form form_, float rotation_, bool throwsShadow_ = false) :
		Basis(vec2(position_.x, position_.y), rotation_),
		drawingPrio{ drawingPrio_ },
		scale{ scale_ },
		color{ color_ },
		id{ id_ },
		form{ form_ },
		throwsShadow{ throwsShadow_ }
	{
	}
};


struct Light {
	Light(vec2 pos, float rad, uint32_t id_, vec4 col) : position{ pos }, radius{ rad }, id{ id_ }, color{ col } {}
	vec2 position;
	float radius;
	uint32_t id;
	vec4 color;
};

struct RenderBuffer {
	std::vector<Drawable> worldSpaceDrawables;
	std::vector<Drawable> windowSpaceDrawables;
	Camera camera;
};