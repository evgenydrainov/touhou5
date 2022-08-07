#pragma once

#include "Input.h"
#include "Audio.h"
#include <SFML/Graphics.hpp>
#include <filesystem>

#define TH_DEBUG 1

namespace th
{
	constexpr int GAME_W = 640;
	constexpr int GAME_H = 480;

	constexpr int CHARACTER_REIMU = 0;
	constexpr int CHARACTER_MARISA = 1;

	class Scene;

	struct Character
	{
		std::string name;
		float move_spd;
		float focus_spd;
		float radius;
		float graze_radius;
	};

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

		Input input;
		Audio audio;
		std::filesystem::path scripts_path = "Scripts";
		sf::RenderWindow window;
		sf::RenderTexture game_surf;
		sf::Font font;
		std::unique_ptr<Scene> scene;
		std::unique_ptr<Scene> next_scene;
		Character characters[2] = {};
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
		bool show_hitboxes = false;
	};
}
