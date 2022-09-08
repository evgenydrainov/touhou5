#pragma once

#include "Stage/Stage.h"

#include <raylib.h>

namespace th
{
	constexpr int PLAY_AREA_W = 384;
	constexpr int PLAY_AREA_H = 448;
	constexpr int PLAY_AREA_X = 32;
	constexpr int PLAY_AREA_Y = 16;

	constexpr float PLAYER_STARTING_X = float(PLAY_AREA_W) / 2.0f;
	constexpr float PLAYER_STARTING_Y = 384.0f;

	struct Game;

	struct Stats
	{
		int score;
		int lives;
		int bombs;
		int power;
		int graze;
		int points;
	};

	struct GameScene
	{
		enum class State { Playing, Paused, Lost };

		Game& game;

		Stats stats = {};
		int hiscore = 0;
		int continues = 0;

		State state = State::Playing;
		std::optional<Stage> stage;
		std::vector<std::string> menu_labels;
		int menu_cursor = 0;

		// resources
		RenderTexture2D play_area = {};
		RenderTexture2D pause_surf = {};
		Texture2D texReimu = {};
		Texture2D texReimuCard = {};
		Texture2D texBg = {};
		Texture2D texHitbox = {};
		Sound sndGraze = {};
		Sound sndPause = {};
		Sound sndReimuShoot = {};
		Sound sndEnemyHit = {};
		Sound sndPichuun = {};
		Sound sndEnemyShoot = {};
		Sound sndPowerUp = {};
		Sound snd1Up = {};
		Music music = {};

		GameScene(const GameScene&) = delete;
		GameScene& operator=(const GameScene&) = delete;
		GameScene(GameScene&&) = delete;
		GameScene& operator=(GameScene&&) = delete;

		GameScene(Game& game);
		~GameScene();

		void update(float delta);
		void draw(RenderTexture2D target, float delta);

		void reset_stats();
		void pause();
		void resume();
		void game_over();
		void use_continue();
		void quit_to_title_screen();
		void play_music(const std::string& fname);
	};
}
