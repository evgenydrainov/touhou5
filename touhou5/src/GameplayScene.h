#pragma once

#include "Objects.h"
#include "Scene.h"

namespace th
{
	constexpr int PLAY_AREA_W = 384;
	constexpr int PLAY_AREA_H = 448;
	constexpr int PLAY_AREA_X = 32;
	constexpr int PLAY_AREA_Y = 16;

	constexpr float PHYSICS_DELTA = 0.2f;
	constexpr float CO_DELTA = 1.0f;

	constexpr float OFFSET = 64.0f;

	class GameplayScene : public Scene
	{
	public:
		GameplayScene(Game& game, const std::filesystem::path& script_path)
			: Scene(game), script_path(script_path)
		{
		}

		void Init() override;
		void Update(float delta) override;
		void Render(sf::RenderTarget& target, float delta) override;

		void UpdatePlayer(float delta);
		void DoPhysics(float delta);
		void PhysicsStep(float delta);
		bool UpdateBoss(Boss& boss, float delta);
		void DoCoro(float delta);
		bool UpdateEnemy(Enemy& e, float delta);

		void Register();
		void BossStartPhase(Boss& b);
		bool BossEndPhase(Boss& b);
		void CheckResult(sol::protected_function_result pres);
		void Error(const std::string& what);
		bool InBounds(float x, float y);

		Bullet* FindBullet(instance_id id);
		Enemy* FindEnemy(instance_id id);

		std::filesystem::path script_path;

		sf::RenderTexture play_area;
		int hiscore = 0;
		float co_timer = 0.0f;

		sol::state lua;
		sol::thread co_runner;
		sol::coroutine co;

		sf::Texture texReimu;
		std::shared_ptr<sf::Texture> texReimuCard;
		sf::Texture texBg;
		sf::Texture texHitbox;

		Character characters[2] = {};

		instance_id next_id = 0;
		Player player = {};
		std::optional<Boss> boss;
		std::vector<Bullet> bullets;
		std::vector<Bullet> player_bullets;
		std::vector<Enemy> enemies;
		std::vector<Powerup> powerups;

		int did_co = 0;
		int did_physics = 0;
	};
}
