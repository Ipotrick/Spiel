#pragma once

#include "RenderTypes.hpp"

struct UIContext {
	Vec2 ulCorner{ 0.0f, 0.0f };
	Vec2 drCorner{ 0.0f, 0.0f };
	float scale{ 1.0f };
	float drawingPrio{ 1.0f };
	static constexpr float incrementSize{ 0.0001f };
	RenderSpace drawMode{ RenderSpace::PixelSpace };

	void increaseDrawPrio()
	{
		drawingPrio += incrementSize;
	}

	void debugDraw(std::vector<Drawable>& buffer)
	{
		Vec2 position = (ulCorner + drCorner) * 0.5f;
		Vec2 size = abs(ulCorner - drCorner);
		Vec4 color{ 1,0,1,0.5 };
		buffer.push_back(Drawable(0, position, drawingPrio, size, color, Form::Rectangle, RotaVec2(0), drawMode));
		increaseDrawPrio();
	}

	Vec2 getScaledSize() const
	{
		return { drCorner.x - ulCorner.x, ulCorner.y - drCorner.y };
	}

	Vec2 getUnscaledSize() const
	{
		return { (drCorner.x - ulCorner.x) / scale, (ulCorner.y - drCorner.y) / scale };
	}

	/*
	* shrinks the contexts size by the given border.
	* so for example: border.x = 2 as parameter means the context shrinks by 4 units in x direction
	*/
	void cutOffBorder(const Vec2 border)
	{
		ulCorner.x += border.x;
		ulCorner.y -= border.y;
		drCorner.x -= border.x;
		drCorner.y += border.y;
	}

	void cutOffBottom(const float f)
	{
		this->drCorner.y += f;
	}
	void cutOffTop(const float f)
	{
		this->ulCorner.y -= f;
	}
	void cutOffLeft(const float f)
	{
		this->ulCorner.x += f;
	}
	void cutOffRight(const float f)
	{
		this->drCorner.x -= f;
	}
};