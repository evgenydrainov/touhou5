#pragma once
#include <SFML/Graphics.hpp>

enum class HAlign { Left, Center, Right };
enum class VAlign { Top, Middle, Bottom };

inline void AlignText(sf::Text& text, HAlign halign, VAlign valign)
{
	sf::FloatRect b = text.getLocalBounds();
	sf::Vector2f o;

	switch (halign) {
		case HAlign::Center: {
			o.x = b.left + 0.5f * b.width;
			break;
		}
		case HAlign::Right: {
			o.x = b.width + 2.0f * b.left;
			break;
		}
	}

	switch (valign) {
		case VAlign::Middle: {
			o.y = b.top + 0.5f * b.width;
			break;
		}
		case VAlign::Bottom: {
			o.y = b.height + 2.0f * b.top;
			break;
		}
	}

	o.x = std::floor(o.x);
	o.y = std::floor(o.y);

	text.setOrigin(o);
}
