#include "UIText.hpp"

void UIText::draw(std::vector<Sprite>& buffer, UIContext context)
{
	calculateTextSize();
	if (bAutoSize) {
		setSize(textSize);
	}
	// create new context inside the UIElement size window of the text element:
	context = anchor.shrinkContextToMe(this->size, context);

	// now apply letterSize and textAnchor to the context:
	context = textAnchor.shrinkContextToMe(this->textSize, context);
	context.recursionDepth++;

	int column = 0;
	int row = 0;
	Vec2 letterSize = context.scale * fontSize;
	auto letter = makeSprite(0, {}, 0, letterSize, color, Form::Rectangle, RotaVec2(0), context.drawMode);
	for (auto c : text) {
		if (c == '\n') {
			row += 1;
			column = 0;
		}
		else {
			letter.position = context.ulCorner + (Vec2(0.5f + column, -0.5f - row) * letterSize);
			if ((letter.position + Vec2(0.5, 0) * letterSize).x > context.drCorner.x) {	// overrun right border
				row += 1;
				column = 0;
				continue;
			}
			else {
				if ((letter.position - Vec2(0.5, 0) * letterSize).y < context.drCorner.y) {	// overrun lower border
					break;
				}
			}

			letter.texRef = makeAsciiRef(fontTexture.id, c);

			column += 1;
			buffer.push_back(letter);
		}
	}
}

void UIText::calculateTextSize()
{
	float maxCol{ 0.0f };
	float maxRow{ 0.0f };
	float col{ 0.0f };
	float row{ 1.0f };
	for (auto& c : text) {
		if (c == '\n') {
			row += 1.0f;
			col = 1.0f;
		}
		else {
			col += 1.0f;
			maxCol = std::max(maxCol, col);
			maxRow = std::max(maxRow, row);
		}
	}
	textSize = { maxCol * fontSize.x, maxRow * fontSize.y } ;
}
