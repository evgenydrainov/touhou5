#pragma once

#include "InputManager.h"
#include <SFML/Graphics.hpp>
#include <filesystem>

#define TH_DEBUG _DEBUG

namespace th
{
	constexpr int GAME_W = 640;
	constexpr int GAME_H = 480;

	class Scene;

	class Game
	{
	public:
		Game();
		~Game();

		void Run();
		void Init();
		void Tick(float delta);
		void Update(float delta);
		void Render(sf::RenderTarget& target, float delta);

		std::filesystem::path scripts_path = "Scripts";
		InputManager input;
		sf::RenderWindow window;
		sf::RenderTexture game_surf;
		sf::Font font;
		std::unique_ptr<Scene> scene;
		std::unique_ptr<Scene> next_scene;
		float time = 0.0f;
		int frame = 0;
		float fps = 0.0f;
		float fps_real = 0.0f;
		float update_took = 0.0f;
		float render_took = 0.0f;
		float fps_sum = 0.0f;
		int fps_samples = 0;
		float fps_clock = 0.0f;
		bool frame_advance = false;
		bool do_frame = false;
		bool debug = false;
	};
}
