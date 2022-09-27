#pragma once

#include "Sprite.h"

#include "Scenes/TitleScene.h"
#include "Scenes/ScriptSelectScene.h"
#include "Scenes/ErrorScene.h"
#include "Scenes/GameScene.h"
#include "Scenes/OptionsScene.h"

#include <raylib.h>
#include <filesystem>
#include <variant>

#define GAME_VERSION "0.0.001"
#define TH_DEBUG 1

namespace th
{
	constexpr int GAME_W = 640;
	constexpr int GAME_H = 480;

	enum Characters
	{
		CHARACTER_REIMU,
		CHARACTER_MARISA,
		CHARACTER_COUNT
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
		Sprite idle_spr;
		Sprite turn_spr;
	};

	struct Options
	{
		int starting_lives = 3;
		int master_volume = 5;
	};

	class Game
	{
	public:
		Game();
		~Game();

		Game(const Game&) = delete;
		Game& operator=(const Game&) = delete;
		Game(Game&&) = delete;
		Game& operator=(Game&&) = delete;

		void run();

		bool quit = false;
		float time = 0.0f;
		Character characters[CHARACTER_COUNT] = {};
		int character_id = 0;
		int next_scene = TITLE_SCENE;
		Options options = {};
		std::filesystem::path scripts_path = "Scripts";
		std::filesystem::path script_path;
		bool god_mode = false;
		bool show_hitboxes = false;
		int key_pressed = 0;
		int char_pressed = 0;

		Sound sndSelect = {};
		Sound sndOk = {};
		Sound sndCancel = {};
		Font font = {};

		std::string error_msg;

	private:
		void update(float delta);
		void draw(float delta);
		void set_window_mode(int mode);

		std::variant<std::monostate, TitleScene, ScriptSelectScene, ErrorScene, GameScene, OptionsScene> scene;

		RenderTexture2D game_surf = {};
		RenderTexture2D up_surf = {};

		int window_mode = 0;
		bool debug_overlay = false;
		bool frame_advance = false;
		bool skip_frame = false;
	};
}
