#pragma once

#include "Objects.h"
#include <raylib.h>
#include <filesystem>
#include <random>

namespace th
{
	constexpr float PHYSICS_DELTA   = .2f;
	constexpr float COROUTINE_DELTA = 1.f;

	class Game;
	class GameScene;

	class Stage
	{
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

	private:
		// keep above
		sol::state lua;
		sol::thread co_runner;
		sol::coroutine co;

	public:
		Player player = {};
		std::optional<Boss> boss;
		std::vector<Bullet> bullets;
		std::vector<PlayerBullet> player_bullets;
		std::vector<Enemy> enemies;
		std::vector<Pickup> pickups;

		int coroutine_calls    = 0;
		int physics_calls      = 0;
		float player_dps       = 0.f;
		float player_dps_dealt = 0.f;
		float player_dps_timer = 0.f;

	private:
		float random();
		void check_result(const sol::protected_function_result& pres);
		void quit_with_error(const std::string& what);
		Bullet* find_bullet(instance_id id);
		Enemy* find_enemy(instance_id id);

		void player_use_bombs(float delta);
		void player_animate(float delta);
		bool boss_update(Boss& b, float delta);
		void boss_start_phase(Boss& b);
		bool boss_end_phase(Boss& b);
		bool enemy_update(Enemy& e, float delta);
		instance_id CreateBossEx(float x, float y, float radius, const sol::table& desc, bool midboss);
		instance_id CreateEnemyEx(float x, float y, float spd, float dir, float acc, float radius, float hp, int sprite_id, const sol::coroutine& script);

		Game& game;
		GameScene& scene;

		instance_id next_id = 0;
		float coroutine_timer = 0.f;
		float time = 0.f;
		float gameplay_delta = 1.f;
		float win_timer = 0.f;
		std::mt19937 rengine;

		std::vector<Texture2D> loaded_textures;
		std::vector<Sprite> loaded_sprites;
		RenderTexture2D bg_surf = {};
		Camera3D bg_cam = {};
		Model bg_model = {};
		Texture2D bg_tex = {};
		Shader bg_shader = {};
		Texture2D spell_bg_tex = {};
		float spell_bg_alpha = 0.f;
		int u_texel = 0;
		int u_time = 0;
		int u_fogOrigin = 0;
		int u_fogColor = 0;
		int u_fogNear = 0;
		int u_fogFar = 0;
		int u_fogEnabled = 0;

		Vector3 fogOrigin = {0.f, 0.f, 0.f};
		Vector4 fogColor = {0.f, 0.f, 0.f, 1.f};
		float fogNear = 0.f;
		float fogFar = 100.f;
		int fogEnabled = 0;

		bool debug_3d = false;
	};
}
