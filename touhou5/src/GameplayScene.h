#pragma once
#include "Scene.h"
#include "Objects.h"
#include <sol/sol.hpp>

namespace th
{
	class GameplayScene : public Scene
	{
	public:
		constexpr static int PLAY_AREA_W = 384;
		constexpr static int PLAY_AREA_H = 448;
		constexpr static int PLAY_AREA_X = 32;
		constexpr static int PLAY_AREA_Y = 16;

		GameplayScene(Game& game) : Scene(game) {}

		void Init() override;
		void Update(float delta) override;
		void Render(sf::RenderTarget& target, float delta) override;

		Bullet* FindBullet(instance_id id);
		Boss* FindBoss(instance_id id);

		int hiscore = 0;
		float fixed_timer = 0.0f;
		sf::RenderTexture play_area;

		// sol
		sol::state lua;
		sol::thread co_runner;
		sol::coroutine co;

		// objects
		Player player = {};
		std::vector<Bullet> bullets;
		std::vector<Bullet> player_bullets;
		std::vector<Enemy> enemies;
		std::vector<Boss> bosses;
		std::vector<Powerup> powerups;
		instance_id next_id = 0;
	};
}
