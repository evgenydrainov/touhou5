#pragma once

#include "Objects.h"
#include <raylib.h>
#include <filesystem>

namespace th
{
	constexpr float PHYSICS_DELTA = 0.2f;
	constexpr float COROUTINE_DELTA = 1.0f;

	constexpr float OFFSET = 64.0f;

	struct Game;
	struct GameScene;

	struct Stage
	{
		Game& game;
		GameScene& scene;

		float coroutine_timer = 0.0f;
		int coroutine_calls = 0;
		int physics_calls = 0;

		// resources
		std::vector<Texture2D> loaded_textures;

		sol::state lua;
		sol::thread co_runner;
		sol::coroutine co;

		instance_id next_id = 0;
		Player player = {};
		std::optional<Boss> boss;
		std::vector<Bullet> bullets;
		std::vector<PlayerBullet> player_bullets;
		std::vector<Enemy> enemies;
		std::vector<Pickup> pickups;

		Stage(const Stage&) = delete;
		Stage& operator=(const Stage&) = delete;
		Stage(Stage&&) = delete;
		Stage& operator=(Stage&&) = delete;

		Stage(Game& game, GameScene& scene);
		~Stage();

		void update(float delta);

		void player_use_bombs(float delta);

		bool boss_update(Boss& b, float delta);
		void boss_start_phase(Boss& b);
		bool boss_end_phase(Boss& b);

		void do_coroutines(float delta);
		bool enemy_update(Enemy& e, float delta);

		void reimu_shot_type(float delta);
		void reimu_bomb();

		void draw(RenderTexture2D target, float delta);

		void get_power(int power);
		void get_lives(int lives);
		void get_bombs(int bombs);

		void register_api();
		void check_result(const sol::protected_function_result& pres);
		void quit_with_error(const std::string& what);

		Bullet* find_bullet(instance_id id);
		Enemy* find_enemy(instance_id id);
	};
}
