#pragma once
#include <SFML/Graphics.hpp>
#include "InputManager.h"

namespace th
{
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
