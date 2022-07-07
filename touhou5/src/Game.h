#pragma once
#include <SFML/Graphics.hpp>
#include "InputManager.h"

namespace th
{
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

	class Scene;

	class Game
	{
	public:
		constexpr static int GAME_W = 640;
		constexpr static int GAME_H = 480;

		Game();
		~Game();

		void Run();
		void Update(float delta);
		void Render(sf::RenderTarget& target, float delta);

		InputManager input;

		sf::RenderWindow window;
		sf::RenderTexture game_surf;
		std::unique_ptr<Scene> scene;
		std::unique_ptr<Scene> next_scene;

		sf::Font font;

		float time = 0.0f;
		int frame = 0;

		float mean_fps = 0.0f;
		float mean_fps_sum = 0.0f;
		int mean_fps_samples = 0;
		float update_took = 0.0f;
		float render_took = 0.0f;
	};
}
