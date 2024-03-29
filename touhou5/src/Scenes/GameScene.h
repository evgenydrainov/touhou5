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

	constexpr float BOSS_STARTING_X = float(PLAY_AREA_W) / 2.0f;
	constexpr float BOSS_STARTING_Y = 96.0f;

	class Game;

	struct Stats
	{
		int score;
		int lives;
		int bombs;
		int power;
		int graze;
		int points;
	};

	class GameScene
	{
	public:
		GameScene(Game& game);
		~GameScene();

		GameScene(const GameScene&) = delete;
		GameScene& operator=(const GameScene&) = delete;
		GameScene(GameScene&&) = delete;
		GameScene& operator=(GameScene&&) = delete;

		void update(float delta);
		void draw(RenderTexture2D target, float delta);

		void get_power(int power);
		void get_lives(int lives);
		void get_bombs(int bombs);
		void get_points(int points);
		void get_score(int score);

		void game_over();
		void win();
		void play_music(const std::string& fname);

		std::optional<Stage> stage;
		Stats stats = {};
		int hiscore = 0;

		Texture2D texReimuCard = {};
		Texture2D texReimuOrb = {};
		Texture2D texReimuOrbShot = {};
		Texture2D texBg = {};
		Texture2D texHitbox = {};
		Texture2D texSidebar = {};
		Texture2D texUiLife = {};
		Texture2D texUiBomb = {};
		Texture2D texEnemyLabel = {};
		Texture2D texPickupPower = {};
		Texture2D texPickupPoint = {};
		Texture2D texPickupBigPower = {};
		Texture2D texPickupBomb = {};
		Texture2D texPickupFullPower = {};
		Texture2D texPickup1Up = {};
		Texture2D texPickupScore = {};
		Sound sndGraze = {};
		Sound sndPause = {};
		Sound sndReimuShoot = {};
		Sound sndEnemyHit = {};
		Sound sndPichuun = {};
		Sound sndEnemyShoot = {};
		Sound sndPowerUp = {};
		Sound snd1Up = {};
		Sound sndPickup = {};
		Sound sndLazer = {};
		Sound sndEnemyDie = {};
		Sound sndBossDie = {};
		Sound sndSpellCard = {};

	private:
		enum class State
		{
			Playing,
			Paused,
			Lost,
			Won
		};

		void reset_stats();
		void pause();
		void resume();
		void use_continue();
		void quit_to_title_screen();

		Game& game;

		State state = State::Playing;
		std::vector<std::string> menu_labels;
		int menu_cursor = 0;
		float menu_anim = 1.0f;
		bool menu_exiting = false;
		int continues = 0;

		RenderTexture2D play_area = {};
		RenderTexture2D pause_surf = {};
		Music music = {};
	};
}
