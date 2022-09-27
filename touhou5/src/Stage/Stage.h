#pragma once

#include "Objects.h"
#include "Sprite.h"
#include <raylib.h>
#include <filesystem>

namespace th
{
	constexpr float PHYSICS_DELTA = 0.2f;
	constexpr float COROUTINE_DELTA = 1.0f;

	constexpr float OFFSET = 64.0f;

	class Game;
	class GameScene;

	class Stage
	{
	private:
		// keep above
		sol::state lua;
		sol::thread co_runner;
		sol::coroutine co;

	public:
		Stage(Game& game, GameScene& scene);
		~Stage();

		Stage(const Stage&) = delete;
		Stage& operator=(const Stage&) = delete;
		Stage(Stage&&) = delete;
		Stage& operator=(Stage&&) = delete;

		void update(float delta);
		void draw(RenderTexture2D target, float delta);
		void player_get_hit();

		void reimu_shot_type(float delta);
		void reimu_bomb();

		void get_power(int power);
		void get_lives(int lives);
		void get_bombs(int bombs);
		void get_points(int points);

		Player player = {};
		std::optional<Boss> boss;
		std::vector<Bullet> bullets;
		std::vector<PlayerBullet> player_bullets;
		std::vector<Enemy> enemies;
		std::vector<Pickup> pickups;

		int coroutine_calls = 0;
		int physics_calls = 0;
		float player_dps = 0.0f;
		float player_dps_dealt = 0.0f;
		float player_dps_timer = 0.0f;

	private:
		void player_use_bombs(float delta);
		void player_animate(float delta);

		bool boss_update(Boss& b, float delta);
		void boss_start_phase(Boss& b);
		bool boss_end_phase(Boss& b);

		bool enemy_update(Enemy& e, float delta);

		void check_result(const sol::protected_function_result& pres);
		void quit_with_error(const std::string& what);

		Bullet* find_bullet(instance_id id);
		Enemy* find_enemy(instance_id id);

		Game& game;
		GameScene& scene;

		instance_id next_id = 0;
		float coroutine_timer = 0.0f;

		std::vector<Texture2D> loaded_textures;
		std::vector<Sprite> loaded_sprites;
	};
}
