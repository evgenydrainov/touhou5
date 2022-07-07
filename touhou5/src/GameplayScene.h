#pragma once
#include "Objects.h"
#include "Scene.h"

namespace th
{
	constexpr static int PLAY_AREA_W = 384;
	constexpr static int PLAY_AREA_H = 448;
	constexpr static int PLAY_AREA_X = 32;
	constexpr static int PLAY_AREA_Y = 16;

	class GameplayScene : public Scene
	{
	public:
		GameplayScene(Game& game) : Scene(game) {}

		void Init() override;
		void Update(float delta) override;
		void Render(sf::RenderTarget& target, float delta) override;
		Bullet* FindBullet(instance_id id);
		Boss* FindBoss(instance_id id);
		void Error(std::string what);

		int hiscore = 0;
		float fixed_timer = 0.0f;
		sf::RenderTexture play_area;
		bool error = false;
		std::string err_what;
		sol::state lua;
		sol::thread co_runner;
		sol::coroutine co;
		instance_id next_id = 0;
		Player player = {};
		std::vector<Bullet> bullets;
		std::vector<Bullet> player_bullets;
		std::vector<Enemy> enemies;
		std::vector<Boss> bosses;
		std::vector<Powerup> powerups;
	};
}
