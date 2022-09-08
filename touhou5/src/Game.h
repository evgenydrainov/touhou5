#pragma once

#include "Scenes/TitleScene.h"
#include "Scenes/ScriptSelectScene.h"
#include "Scenes/ErrorScene.h"
#include "Scenes/GameScene.h"
#include "Scenes/OptionsScene.h"

#include <raylibx.h>
#include <filesystem>
#include <variant>

#define GAME_VERSION "0.0.1"
#define TH_DEBUG 1

namespace th
{
	constexpr int GAME_W = 640;
	constexpr int GAME_H = 480;

	enum Characters
	{
		CHARACTER_REIMU,
		CHARACTER_MARISA
	};

	enum Scenes
	{
		TITLE_SCENE = 1,
		SCRIPT_SELECT_SCENE,
		ERROR_SCENE,
		GAME_SCENE,
		OPTIONS_SCENE
	};

	struct Character
	{
		std::string name;
		float move_spd;
		float focus_spd;
		float radius;
		float graze_radius;
		float deathbomb_time;
		int starting_bombs;
		void (Stage::*shot_type)(float);
		void (Stage::*bomb)();
	};

	struct Options
	{
		int starting_lives = 3;
		float master_volume = 0.5f;
	};

	struct Game
	{
		Character characters[2] = {};
		std::filesystem::path scripts_path = "Scripts";

		// resources
		RenderTexture2D game_surf = {};
		RenderTexture2D up_surf = {};
		Sound sndSelect = {};
		Sound sndOk = {};
		Sound sndCancel = {};

		bool quit = false;
		Options options = {};
		float time = 0;
		int window_mode = 0;
		bool show_hitboxes = false;
		bool god_mode = false;
		bool debug_overlay = false;
		bool frame_advance = false;
		bool skip_frame = false;

		std::filesystem::path script_path;
		std::string error_msg;
		int character_id = 0;

		std::variant<std::monostate, TitleScene, ScriptSelectScene, ErrorScene, GameScene, OptionsScene> scene;
		int next_scene = TITLE_SCENE;

		Game(const Game&) = delete;
		Game& operator=(const Game&) = delete;
		Game(Game&&) = delete;
		Game& operator=(Game&&) = delete;

		Game();
		~Game();

		void run();
		void update(float delta);
		void draw(float delta);
		void set_window_mode(int mode);
		void set_master_volume(float vol);
	};
}
