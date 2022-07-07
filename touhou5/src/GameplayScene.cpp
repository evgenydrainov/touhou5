#include "GameplayScene.h"

#include "cpml.h"
#include <fmt/format.h>

namespace th
{
	void GameplayScene::Init()
	{
		{
			play_area.create(PLAY_AREA_W, PLAY_AREA_H);

			player.x = PLAY_AREA_W / 2;
			player.y = PLAY_AREA_H / 4 * 3;
			player.radius = 3.0f;
			player.move_spd = 3.75f;
			player.focus_spd = 1.75f;
			player.lives = 5;
			player.bombs = 2;
		}

		{
			lua.open_libraries(
				sol::lib::base,
				sol::lib::package,
				sol::lib::coroutine,
				sol::lib::string,
				sol::lib::math,
				sol::lib::table,
				sol::lib::utf8
			);

			lua["CreateBullet"] = [this](float x, float y, float spd, float acc, float dir, float radius) -> instance_id
			{
				instance_id id = next_id++;
				bullets.emplace_back(id, x, y, spd, acc, dir, radius);
				return id;
			};

			lua["CreateBoss"] = [this](sol::table desc, float x, float y, float radius) -> instance_id
			{
				instance_id id = next_id++;

				Boss& b = bosses.emplace_back(id, x, y, radius);

				//const char* name = desc["Name"];
				//printf("%s\n", name);

				try {
					int v = desc["V"];
					printf("%d\n", v);
				}
				catch (...) {

				}

				return id;
			};

			lua["BulletGetX"] = [this](instance_id id) { if (Bullet* b = FindBullet(id)) return b->x; else return 0.0f; };
			lua["BulletSetX"] = [this](instance_id id, float x) { if (Bullet* b = FindBullet(id)) b->x = x; };

			lua["BulletGetY"] = [this](instance_id id) { if (Bullet* b = FindBullet(id)) return b->y; else return 0.0f; };
			lua["BulletSetY"] = [this](instance_id id, float y) { if (Bullet* b = FindBullet(id)) b->y = y; };

			lua["BulletGetSpd"] = [this](instance_id id) { if (Bullet* b = FindBullet(id)) return b->spd; else return 0.0f; };
			lua["BulletSetSpd"] = [this](instance_id id, float spd) { if (Bullet* b = FindBullet(id)) b->spd = spd; };

			lua["BulletGetAcc"] = [this](instance_id id) { if (Bullet* b = FindBullet(id)) return b->acc; else return 0.0f; };
			lua["BulletSetAcc"] = [this](instance_id id, float acc) { if (Bullet* b = FindBullet(id)) b->acc = acc; };

			lua["BulletGetDir"] = [this](instance_id id) { if (Bullet* b = FindBullet(id)) return b->dir; else return 0.0f; };
			lua["BulletSetDir"] = [this](instance_id id, float dir) { if (Bullet* b = FindBullet(id)) b->dir = dir; };

			lua["BulletGetRadius"] = [this](instance_id id) { if (Bullet* b = FindBullet(id)) return b->radius; else return 0.0f; };
			lua["BulletSetRadius"] = [this](instance_id id, float radius) { if (Bullet* b = FindBullet(id)) b->radius = radius; };

			sol::protected_function_result pres = lua.safe_script_file("stage.lua");
			if (pres.valid()) {
				if (sol::optional<sol::table> res = pres) {
					if (sol::optional<sol::coroutine> script = res.value()["Script"]) {
						co_runner = sol::thread::create(lua);
						co = sol::coroutine(co_runner.thread_state(), script.value());
					} else {
						printf("expected result.Script to be function got %s\n", lua_typename(lua, (int)res.value()["Script"].get_type()));
					}
				} else {
					printf("expected result to be table got %s\n", lua_typename(lua, (int)pres.get_type()));
				}
			} else {
				sol::error err = pres;
				printf("%s\n", err.what());
			}
		}
	}

	void GameplayScene::Update(float delta)
	{
		{
			int h = input.Check(Key::Right) - input.Check(Key::Left);
			int v = input.Check(Key::Down) - input.Check(Key::Up);
			player.is_focused = input.Check(Key::Shift);
			float spd = player.is_focused ? player.focus_spd : player.move_spd;

			if (v == 0) {
				player.x += (float)h * spd * delta;
			} else {
				player.x += (float)h * spd * cpml::sqrt2 * 0.5f * delta;
			}

			if (h == 0) {
				player.y += (float)v * spd * delta;
			} else {
				player.y += (float)v * spd * cpml::sqrt2 * 0.5f * delta;
			}

			player.x = std::clamp(player.x, 0.0f, (float)(PLAY_AREA_W - 1));
			player.y = std::clamp(player.y, 0.0f, (float)(PLAY_AREA_H - 1));
		}

		for (Bullet& b : bullets) {
			b.spd += b.acc * delta;
			b.x += cpml::lengthdir_x(b.spd, b.dir) * delta;
			b.y += cpml::lengthdir_y(b.spd, b.dir) * delta;
		}

		for (Bullet& b : player_bullets) {
			b.spd += b.acc * delta;
			b.x += cpml::lengthdir_x(b.spd, b.dir) * delta;
			b.y += cpml::lengthdir_y(b.spd, b.dir) * delta;
		}

		//for (Enemy& e : enemies) {
		//	
		//}

		//for (Boss& b : bosses) {
		//
		//}

		for (Powerup& p : powerups) {
			p.vsp += p.grv * delta;
			p.vsp = std::min(p.vsp, p.max_vsp);
			p.y += p.vsp * delta;
		}

		fixed_timer += delta;
		while (fixed_timer >= 1.0f) {
			if (co.runnable()) {
				sol::protected_function_result pres = co();
				if (pres.valid()) {

				} else {
					sol::error err = pres;
					printf("%s\n", err.what());
				}
			}

			fixed_timer -= 1.0f;
		}
	}

	void GameplayScene::Render(sf::RenderTarget& target, float delta)
	{
		play_area.clear();

		{
			sf::CircleShape c;
			c.setRadius(player.radius);
			c.setOrigin(player.radius, player.radius);
			c.setPosition(player.x, player.y);
			play_area.draw(c);
		}

		for (Bullet& b : bullets) {
			sf::CircleShape c;
			c.setRadius(b.radius);
			c.setOrigin(b.radius, b.radius);
			c.setPosition(b.x, b.y);
			play_area.draw(c);
		}

		for (Bullet& b : player_bullets) {
			sf::CircleShape c;
			c.setRadius(b.radius);
			c.setOrigin(b.radius, b.radius);
			c.setPosition(b.x, b.y);
			play_area.draw(c);
		}

		for (Enemy& e : enemies) {
			sf::CircleShape c;
			c.setRadius(e.radius);
			c.setOrigin(e.radius, e.radius);
			c.setPosition(e.x, e.y);
			play_area.draw(c);
		}

		for (Boss& b : bosses) {
			sf::CircleShape c;
			c.setRadius(b.radius);
			c.setOrigin(b.radius, b.radius);
			c.setPosition(b.x, b.y);
			play_area.draw(c);
		}

		play_area.display();

		{
			sf::Sprite s(play_area.getTexture());
			s.setPosition(PLAY_AREA_X, PLAY_AREA_Y);
			target.draw(s);

			static sf::Text t;
			t.setFont(game.font);
			t.setCharacterSize(16);
			t.setString(fmt::format(
				"{}\n{}\n\n{}\n{}\n\n{}\n{}\n{}",
				hiscore, player.score, player.lives, player.bombs, player.power, player.graze, player.points
			));
			t.setPosition(PLAY_AREA_X + PLAY_AREA_W + 16, PLAY_AREA_Y + 32);
			target.draw(t);
		}
	}

	Bullet* GameplayScene::FindBullet(instance_id id)
	{
		size_t left = 0;
		size_t right = bullets.size() - 1;

		while (left <= right) {
			size_t middle = (left + right) / 2;
			if (bullets[middle].id < id) {
				left = middle + 1;
			} else if (bullets[middle].id > id) {
				right = middle - 1;
			} else {
				return &bullets[middle];
			}
		}

		return nullptr;
	}

	Boss* GameplayScene::FindBoss(instance_id id)
	{
		size_t left = 0;
		size_t right = bosses.size() - 1;

		while (left <= right) {
			size_t middle = (left + right) / 2;
			if (bosses[middle].id < id) {
				left = middle + 1;
			} else if (bosses[middle].id > id) {
				right = middle - 1;
			} else {
				return &bosses[middle];
			}
		}

		return nullptr;
	}
}
