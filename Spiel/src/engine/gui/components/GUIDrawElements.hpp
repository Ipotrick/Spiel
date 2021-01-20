#pragma once

#include "../GUIManager.hpp"
#include "GUIUpdateElements.hpp"

namespace gui {

	template<typename T>
	void onDraw(Manager& manager, T& element, DrawContext const& context, std::vector<Sprite>& out) {}

	template<> inline void onDraw<Root>(Manager& manager, Root& self, DrawContext const& context, std::vector<Sprite>& out);
	template<> inline void onDraw<Box>(Manager& manager, Box& self, DrawContext const& context, std::vector<Sprite>& out);
	template<> inline void onDraw<FillBox>(Manager& manager, FillBox& self, DrawContext const& context, std::vector<Sprite>& out);
	template<> inline void onDraw<Column>(Manager& manager, Column& self, DrawContext const& context, std::vector<Sprite>& out);
	template<> inline void onDraw<GUIButton>(Manager& manager, GUIButton& self, DrawContext const& context, std::vector<Sprite>& out);

	void draw(Manager& manager, ElementVariant& var, DrawContext const& context, std::vector<Sprite>& out)
	{
		std::visit([&](auto&& element) { onDraw(manager, element, context, out); }, var);
	}

	/**
	 * \return the UNSCALED minimum size the element needs
	 */
	Vec2 minsizeOf(Manager& manager, u32 id)
	{
		ElementVariant& var = manager.elements[id];
		if (Box* box = std::get_if<Box>(&var)) {
			return box->minsize;
		}
		else if (Column* col = std::get_if<Column>(&var)) {
			if (manager.minsizes[id].has_value()) {
				return manager.minsizes[id].value();
			}
			else {
				Vec2 minsize{ 0,0 };
				for (u32 child : col->children) {
					auto childMinSize = minsizeOf(manager, child);
					minsize.x = std::max(minsize.x, childMinSize.x);
					minsize.y += childMinSize.y;
				}
				manager.minsizes[id] = minsize;
				return minsize;
			}
		}
		else if (GUIButton* button = std::get_if<GUIButton>(&var)) {
			return button->size;
		}
		assert(false);
		return { 0,0 };
	}

	  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	 ////////////////////////////////////////////////// TYPE SPECIFIC IMPLEMENTATIONS: ////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<> inline void onDraw<Root>(Manager& manager, Root& self, DrawContext const& context, std::vector<Sprite>& out)
	{
		auto scaledSize = getSize(self.sizeing, context);
		auto place = getPlace(scaledSize, self.placeing, context);
		out.push_back(
			Sprite{
				.color = self.color,
				.position = Vec3{place, context.renderDepth},
				.scale = scaledSize,
				.form = Form::Rectangle,
				.drawMode = RenderSpace::PixelSpace
			}
		);
		if (self.child != INVALID_ELEMENT_ID) {
			const auto childDrawContext = fit(context, scaledSize, place);
			draw(manager, manager.elements[self.child], childDrawContext, out);
		}
	}

	template<> inline void onDraw<Box>(Manager& manager, Box& self, DrawContext const& context, std::vector<Sprite>& out)
	{
		auto scaledSize = self.minsize * context.scale;
		auto place = getPlace(scaledSize, context);
		out.push_back(
			Sprite{
				.color = self.color,
				.position = Vec3{place, context.renderDepth},
				.scale = scaledSize,
				.form = Form::Rectangle,
				.drawMode = RenderSpace::PixelSpace
			}
		);
	}

	template<> inline void onDraw<FillBox>(Manager& manager, FillBox& self, DrawContext const& context, std::vector<Sprite>& out)
	{
		auto scaledSize = context.size();
		auto place = context.centerpos();
		out.push_back(
			Sprite{
				.color = self.color,
				.position = Vec3{place, context.renderDepth},
				.scale = scaledSize,
				.form = Form::Rectangle,
				.drawMode = RenderSpace::PixelSpace
			}
		);
	}

	template<> inline void onDraw<Column>(Manager& manager, Column& self, DrawContext const& context, std::vector<Sprite>& out)
	{
		const float scaledSpaceing = self.spaceing * context.scale;

		float childrenScaledY{ 0.0f };
		for (auto& child : self.children) {
			childrenScaledY += minsizeOf(manager, child).y;
		}
		childrenScaledY *= context.scale;
		const float childCount = static_cast<float>(self.children.size());
		float allSpacingY{ (childCount - 1) * scaledSpaceing };

		const float leftYSpace = context.size().y - scaledSpaceing * (childCount-1.0f) - childrenScaledY;
		const float uniformPerChildSpace = leftYSpace / childCount;
		
		float offset{ 0.0f };
		if (self.listing == Listing::Packed) {
			if (self.yalignment == YAlign::Center)
				offset += (context.size().y - childrenScaledY - allSpacingY) * 0.5f;
			if (self.yalignment == YAlign::Bottom)
				offset += context.size().y - childrenScaledY - allSpacingY;
		}

		for (auto& childid : self.children) {
			ElementVariant& childvar = manager.elements[childid];
			const auto scaledSize = minsizeOf(manager, childid) * context.scale;

			DrawContext childContext = context;
			childContext.xalign = self.xalignment;
			childContext.yalign = self.yalignment;

			if (self.listing == Listing::Packed) {
				childContext.cutTop(offset);
				childContext.cutBottom(context.size().y - scaledSize.y - offset);
				offset += scaledSize.y + scaledSpaceing;
			}
			else {
				childContext.cutTop(offset);
				childContext.cutBottom(context.size().y - scaledSize.y - uniformPerChildSpace - offset);
				offset += scaledSize.y + scaledSpaceing + uniformPerChildSpace;
			}
			draw(manager, childvar, childContext, out);
		}
	}

	template<> inline void onDraw<GUIButton>(Manager& manager, GUIButton& self, DrawContext const& context, std::vector<Sprite>& out)
	{
		auto scaledSize = self.size * context.scale;
		auto place = getPlace(scaledSize, context);

		onUpdate(manager, self, context.bCursorFocus, place, scaledSize);

		Vec4 color = self._bHold ? Vec4::From256(38, 241, 244) : Vec4::From256(10, 57, 58);

		out.push_back(
			Sprite{
				.color = color,
				.position = Vec3{place, context.renderDepth},
				.scale = scaledSize,
				.form = Form::Rectangle,
				.drawMode = RenderSpace::PixelSpace
			}
		);
	}
}