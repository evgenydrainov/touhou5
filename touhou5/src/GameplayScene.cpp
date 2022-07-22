#include "GameplayScene.h"

#include "ErrorScene.h"

#include "cpml.h"
#include "misc.h"
#include <fmt/format.h>

namespace th
{
	void GameplayScene::Init()
	{
		texReimu.loadFromFile("reimu.png");
		texReimuCard = std::make_shared<sf::Texture>();
		texReimuCard->loadFromFile("reimucard.png");
		texBg.loadFromFile("bg.png");
		texHitbox.loadFromFile("hitbox.png");

		characters[0].name = "Reimu Hakurei";
		characters[0].move_spd = 3.75f;
		characters[0].focus_spd = 1.75f;
		characters[0].radius = 2.0f;
		characters[0].graze_radius = 16.0f;
		characters[0].texture = &texReimu;

		player.x = PLAY_AREA_W / 2;
		player.y = PLAY_AREA_H / 4 * 3;
		player.lives = 5;
		player.bombs = 2;

		play_area.create(PLAY_AREA_W, PLAY_AREA_H);

		lua.open_libraries(
			sol::lib::base,
			sol::lib::package,
			sol::lib::coroutine,
			sol::lib::string,
			sol::lib::math,
			sol::lib::table,
			sol::lib::utf8
		);

		lua["package"]["cpath"] = "";

		lua["package"]["path"] =
			(script_path / "?.lua").string() +
			";" +
			(game.scripts_path / "?.lua").string();

		Register();

		try {
			sol::table res = lua.unsafe_script_file((script_path / "stage.lua").string());
			co_runner = sol::thread::create(lua);
			sol::coroutine script = res["Script"];
			co = sol::coroutine(co_runner.thread_state(), script);
		} catch (const sol::error& err) {
			Error(err.what());
		}
	}

	void GameplayScene::Update(float delta)
	{
		UpdatePlayer(delta);

		DoCoro(delta);

		DoPhysics(delta);

		player.x = std::clamp(player.x, 0.0f, (float)(PLAY_AREA_W - 1));
		player.y = std::clamp(player.y, 0.0f, (float)(PLAY_AREA_H - 1));

		for (auto b = bullets.begin(); b != bullets.end();) {
			if (!InBounds(b->x, b->y)) {
				b = bullets.erase(b);
				continue;
			}
			++b;
		}

		for (auto b = player_bullets.begin(); b != player_bullets.end();) {
			b->spd += b->acc * delta;
			b->x += cpml::lengthdir_x(b->spd, b->dir) * delta;
			b->y += cpml::lengthdir_y(b->spd, b->dir) * delta;
			if (!InBounds(b->x, b->y)) {
				b = player_bullets.erase(b);
				continue;
			}
			++b;
		}

		if (boss) {
			if (!UpdateBoss(*boss, delta)) {
				boss = std::nullopt;
			}
		}

		for (auto e = enemies.begin(); e != enemies.end();) {
			if (!UpdateEnemy(*e, delta)) {
				e = enemies.erase(e);
				continue;
			}
			++e;
		}

		for (auto p = powerups.begin(); p != powerups.end();) {
			p->vsp += p->grv * delta;
			p->vsp = std::min(p->vsp, p->max_vsp);
			p->y += p->vsp * delta;
			if (!InBounds(p->x, p->y)) {
				p = powerups.erase(p);
				continue;
			}
			if (cpml::circle_vs_circle(p->x, p->y, p->radius, player.x, player.y, characters[player.character_id].radius)) {
				switch (p->type) {
					case PowerupType::Points: {
						player.points++;
						break;
					}
					case PowerupType::Power: {
						player.power++;
						break;
					}
				}
				p = powerups.erase(p);
				continue;
			}
			++p;
		}
	}

	void GameplayScene::Render(sf::RenderTarget& target, float delta)
	{
		play_area.clear();
		{
			if (boss) {
				if (boss->texture) {
					sf::Sprite s;
					s.setTexture(*boss->texture);
					s.setPosition(boss->x, boss->y);
					s.setOrigin(sf::Vector2f(boss->texture->getSize()) / 2.0f);
					play_area.draw(s);
				}
				if (game.debug) {
					sf::CircleShape c;
					c.setRadius(boss->radius);
					c.setOrigin(boss->radius, boss->radius);
					c.setPosition(boss->x, boss->y);
					c.setFillColor(sf::Color(255, 255, 255, 100));
					play_area.draw(c);
				}
			}

			for (Enemy& e : enemies) {
				if (e.texture) {
					sf::Sprite s;
					s.setTexture(*e.texture);
					s.setPosition(e.x, e.y);
					s.setOrigin(sf::Vector2f(e.texture->getSize()) / 2.0f);
					play_area.draw(s);
				}
				if (game.debug) {
					sf::CircleShape c;
					c.setRadius(e.radius);
					c.setOrigin(e.radius, e.radius);
					c.setPosition(e.x, e.y);
					play_area.draw(c);
				}
			}

			{
				sf::Sprite s;
				s.setTexture(*characters[player.character_id].texture);
				s.setPosition(player.x, player.y);
				s.setOrigin(sf::Vector2f(characters[player.character_id].texture->getSize()) / 2.0f);
				play_area.draw(s);
				{
					static float alpha = 0.0f;
					alpha = cpml::approach(alpha, player.is_focused ? 1.0f : 0.0f, 0.1f * delta);
					if (alpha > 0.0f) {
						sf::Sprite s;
						s.setTexture(texHitbox);
						s.setPosition(player.x, player.y);
						s.setOrigin(sf::Vector2f(texHitbox.getSize()) / 2.0f);
						s.setRotation(game.time);
						s.setColor(sf::Color(255, 255, 255, alpha * 255.0f));
						play_area.draw(s);
					}
				}
				if (game.debug) {
					{
						sf::CircleShape c;
						c.setRadius(characters[player.character_id].graze_radius);
						c.setOrigin(characters[player.character_id].graze_radius, characters[player.character_id].graze_radius);
						c.setPosition(player.x, player.y);
						c.setFillColor(sf::Color::Black);
						play_area.draw(c);
					}
					{
						sf::CircleShape c;
						c.setRadius(characters[player.character_id].radius);
						c.setOrigin(characters[player.character_id].radius, characters[player.character_id].radius);
						c.setPosition(player.x, player.y);
						play_area.draw(c);
					}
				}
			}

			for (Bullet& b : player_bullets) {
				if (b.texture) {
					sf::Sprite s;
					s.setTexture(*b.texture);
					s.setPosition(b.x, b.y);
					s.setOrigin(sf::Vector2f(b.texture->getSize()) / 2.0f);
					s.setScale(1.5f, 1.5f);
					s.setColor(sf::Color(255, 255, 255, 128));
					play_area.draw(s);
				}
				if (game.debug) {
					sf::CircleShape c;
					c.setRadius(b.radius);
					c.setOrigin(b.radius, b.radius);
					c.setPosition(b.x, b.y);
					c.setFillColor(sf::Color(255, 255, 255, 100));
					play_area.draw(c);
				}
			}

			for (Bullet& b : bullets) {
				if (b.texture) {
					sf::Sprite s;
					s.setTexture(*b.texture);
					s.setPosition(b.x, b.y);
					s.setOrigin(sf::Vector2f(b.texture->getSize()) / 2.0f);
					play_area.draw(s);
				}
				if (game.debug) {
					sf::CircleShape c;
					c.setRadius(b.radius);
					c.setOrigin(b.radius, b.radius);
					c.setPosition(b.x, b.y);
					c.setFillColor(sf::Color::Red);
					play_area.draw(c);
				}
			}

			for (Powerup& p : powerups) {
				if (game.debug) {
					sf::CircleShape c;
					c.setRadius(p.radius);
					c.setOrigin(p.radius, p.radius);
					c.setPosition(p.x, p.y);
					play_area.draw(c);
				}
			}

			if (boss) {
				static sf::Text t;
				t.setFont(game.font);
				t.setCharacterSize(16);
				t.setString(fmt::format(
					"{}\n"
					"{}/{}hp\n"
					"phase {}/{}\n"
					"timer {}",
					boss->name,
					boss->hp, boss->phases[boss->phase_index].hp,
					boss->phase_index, boss->phases.size(),
					boss->timer / 60.0f
				));
				play_area.draw(t);
			}
		}
		play_area.display();

		{
			target.draw(sf::Sprite(texBg));
		}

		{
			sf::Sprite s;
			s.setTexture(play_area.getTexture());
			s.setPosition(PLAY_AREA_X, PLAY_AREA_Y);
			target.draw(s);
		}

		{
			static sf::Text t;
			t.setFont(game.font);
			t.setCharacterSize(16);
			t.setString(fmt::format(
				"HiScore {}\n"
				"Score {}\n\n"
				"Player {}\n"
				"Bomb {}\n\n"
				"Power {}\n"
				"Graze {}\n"
				"Point {}",
				hiscore,
				player.score,
				player.lives,
				player.bombs,
				player.power,
				player.graze,
				player.points
			));
			t.setPosition(PLAY_AREA_X + PLAY_AREA_W + 16, PLAY_AREA_Y + 32);
			target.draw(t);
		}

		if (game.debug) {
			static sf::Text t;
			t.setFont(game.font);
			t.setCharacterSize(16);
			t.setString(fmt::format(
				"bullets: {}\n"
				"enemies: {}\n"
				"player bullets: {}\n"
				"powerups: {}\n"
				"did physics: {}\n"
				"did co: {}",
				bullets.size(),
				enemies.size(),
				player_bullets.size(),
				powerups.size(),
				did_physics,
				did_co
			));
			t.setPosition(GAME_W, 0);
			AlignText(t, HAlign::Right, VAlign::Top);
			target.draw(t);
		}
	}

	void GameplayScene::UpdatePlayer(float delta)
	{
		int h = input.Check(Key::Right) - input.Check(Key::Left);
		int v = input.Check(Key::Down) - input.Check(Key::Up);
		player.is_focused = input.Check(Key::Shift);

		float spd =
			player.is_focused ?
			characters[player.character_id].focus_spd :
			characters[player.character_id].move_spd;

		player.hsp = 0.0f;
		player.vsp = 0.0f;

		if (v == 0) {
			player.hsp = (float)h * spd;
		} else {
			player.hsp = (float)h * spd * cpml::sqrt2 * 0.5f;
		}

		if (h == 0) {
			player.vsp = (float)v * spd;
		} else {
			player.vsp = (float)v * spd * cpml::sqrt2 * 0.5f;
		}

		if (player.reimu.fire_timer <= 0) {
			if (player.reimu.fire_queue > 0) {
				for (int i = 0; i < 4; i++) {
					player_bullets.emplace_back(
						next_id++,
						player.x,
						player.y - 10.0f,
						12.0f,
						90.0f - 7.5f + (float)i * 5.0f,
						0.0f,
						10.0f,
						texReimuCard,
						15.0f,
						false
					);
				}
				player.reimu.fire_timer = 4.0f;
				--player.reimu.fire_queue;
			} else if (input.Check(Key::Z)) {
				player.reimu.fire_queue = 6;
			}
		} else {
			player.reimu.fire_timer -= delta;
		}

		if (input.CheckPressed(Key::X)) {
			if (player.bombs > 0) {
				bullets.clear();
				player.bombs--;
			}
		}
	}

	void GameplayScene::DoPhysics(float delta)
	{
		did_physics = 0;
		while (delta > 0.0f) {
			float physics_delta = std::min(delta, PHYSICS_DELTA);
			PhysicsStep(physics_delta);
			delta -= physics_delta;
			did_physics++;
		}
	}

	void GameplayScene::PhysicsStep(float delta)
	{
		player.x += player.hsp * delta;
		player.y += player.vsp * delta;

		for (auto b = bullets.begin(); b != bullets.end();) {
			b->spd += b->acc * delta;
			b->x += cpml::lengthdir_x(b->spd, b->dir) * delta;
			b->y += cpml::lengthdir_y(b->spd, b->dir) * delta;
			if (cpml::circle_vs_circle(b->x, b->y, b->radius, player.x, player.y, characters[player.character_id].radius)) {
				player.lives--;
				b = bullets.erase(b);
				continue;
			}
			++b;
		}
	}

	bool GameplayScene::UpdateBoss(Boss& boss, float delta)
	{
		boss.spd += boss.acc * delta;
		boss.x += cpml::lengthdir_x(boss.spd, boss.dir) * delta;
		boss.y += cpml::lengthdir_y(boss.spd, boss.dir) * delta;

		for (auto b = player_bullets.begin(); b != player_bullets.end();) {
			if (cpml::circle_vs_circle(b->x, b->y, b->radius, boss.x, boss.y, boss.radius)) {
				boss.hp -= b->dmg;
				b = player_bullets.erase(b);
				if (boss.hp <= 0.0f) {
					if (!BossEndPhase(boss)) {
						return false;
					}
				}
				continue;
			}
			++b;
		}

		if (boss.timer <= 0.0f) {
			if (!BossEndPhase(boss)) {
				return false;
			}
		}

		boss.timer -= std::min(delta, boss.timer);

		return true;
	}

	void GameplayScene::DoCoro(float delta)
	{
		co_timer += delta;
		did_co = 0;
		while (co_timer >= CO_DELTA) {
			try {
				if (co.runnable()) {
					CheckResult(co());
				}
				if (boss) {
					if (boss->co.runnable()) {
						CheckResult(boss->co());
					}
				}
				for (Enemy& e : enemies) {
					if (e.co.runnable()) {
						CheckResult(e.co(e.id));
					}
				}
			} catch (const sol::error& err) {
				Error(err.what());
			}

			co_timer -= CO_DELTA;
			did_co++;
		}
	}

	bool GameplayScene::UpdateEnemy(Enemy& e, float delta)
	{
		e.spd += e.acc * delta;
		e.x += cpml::lengthdir_x(e.spd, e.dir) * delta;
		e.y += cpml::lengthdir_y(e.spd, e.dir) * delta;
		if (!InBounds(e.x, e.y)) {
			return false;
		}
		for (auto b = player_bullets.begin(); b != player_bullets.end();) {
			if (cpml::circle_vs_circle(b->x, b->y, b->radius, e.x, e.y, e.radius)) {
				e.hp -= b->dmg;
				b = player_bullets.erase(b);
				if (e.hp <= 0.0f) {
					powerups.emplace_back(e.x, e.y, -1.5f, 0.025f, 2.0f, 5.0f);
					return false;
				}
				continue;
			}
			++b;
		}
		return true;
	}

	void GameplayScene::Register()
	{
		lua["Shoot"] = [this](float x, float y, float spd, float dir, float acc, std::shared_ptr<sf::Texture> texture, float radius, bool rotate)
		{
			instance_id id = next_id++;
			bullets.emplace_back(
				id,
				x,
				y,
				spd,
				dir,
				acc,
				radius,
				texture,
				rotate
			);
			return id;
		};

		lua["CreateBoss"] = [this](sol::table desc)
		{
			Boss& b = boss.emplace();
			b.x = PLAY_AREA_W / 2;
			b.y = PLAY_AREA_H / 4;
			b.radius = 20.0f;
			b.texture = desc["Texture"];
			b.name = desc["Name"];
			sol::nested<std::vector<sol::table>> phases = desc["Phases"];
			for (const sol::table& phase : phases.value()) {
				sol::coroutine script = phase["Script"];
				float time = phase["Time"];
				float hp = phase["HP"];
				b.phases.emplace_back(
					sol::coroutine(lua, script),
					time * 60.0f,
					hp
				);
			}
			BossStartPhase(b);
		};

		lua["CreateEnemy"] = [this](float x, float y, sol::table desc)
		{
			instance_id id = next_id++;
			Enemy& e = enemies.emplace_back(id, x, y);
			e.radius = 5.0f;
			e.texture = desc["Texture"];
			e.hp = desc["HP"];
			e.co_runner = sol::thread::create(lua);
			sol::coroutine script = desc["Script"];
			e.co = sol::coroutine(e.co_runner.thread_state(), script);
			return id;
		};

		lua["LoadTexture"] = [this](std::string fname)
		{
			std::shared_ptr<sf::Texture> t = std::make_shared<sf::Texture>();
			t->loadFromFile((script_path / fname).string());
			return t;
		};

		// bullet

		lua["BltGetX"] = [this](instance_id id) { if (Bullet* b = FindBullet(id)) return b->x; else return 0.0f; };
		lua["BltGetY"] = [this](instance_id id) { if (Bullet* b = FindBullet(id)) return b->y; else return 0.0f; };
		lua["BltGetSpd"] = [this](instance_id id) { if (Bullet* b = FindBullet(id)) return b->spd; else return 0.0f; };
		lua["BltGetDir"] = [this](instance_id id) { if (Bullet* b = FindBullet(id)) return b->dir; else return 0.0f; };
		lua["BltGetAcc"] = [this](instance_id id) { if (Bullet* b = FindBullet(id)) return b->acc; else return 0.0f; };
		lua["BltGetRadius"] = [this](instance_id id) { if (Bullet* b = FindBullet(id)) return b->radius; else return 0.0f; };

		lua["BltSetX"] = [this](instance_id id, float x) { if (Bullet* b = FindBullet(id)) b->x = x; };
		lua["BltSetY"] = [this](instance_id id, float y) { if (Bullet* b = FindBullet(id)) b->y = y; };
		lua["BltSetSpd"] = [this](instance_id id, float spd) { if (Bullet* b = FindBullet(id)) b->spd = spd; };
		lua["BltSetDir"] = [this](instance_id id, float dir) { if (Bullet* b = FindBullet(id)) b->dir = dir; };
		lua["BltSetAcc"] = [this](instance_id id, float acc) { if (Bullet* b = FindBullet(id)) b->acc = acc; };
		lua["BltSetRadius"] = [this](instance_id id, float radius) { if (Bullet* b = FindBullet(id)) b->radius = radius; };

		// enemy

		lua["EnmGetX"] = [this](instance_id id) { if (Enemy* e = FindEnemy(id)) return e->x; else return 0.0f; };
		lua["EnmGetY"] = [this](instance_id id) { if (Enemy* e = FindEnemy(id)) return e->y; else return 0.0f; };
		lua["EnmGetSpd"] = [this](instance_id id) { if (Enemy* e = FindEnemy(id)) return e->spd; else return 0.0f; };
		lua["EnmGetDir"] = [this](instance_id id) { if (Enemy* e = FindEnemy(id)) return e->dir; else return 0.0f; };
		lua["EnmGetAcc"] = [this](instance_id id) { if (Enemy* e = FindEnemy(id)) return e->acc; else return 0.0f; };
		lua["EnmGetRadius"] = [this](instance_id id) { if (Enemy* e = FindEnemy(id)) return e->radius; else return 0.0f; };

		lua["EnmSetX"] = [this](instance_id id, float x) { if (Enemy* e = FindEnemy(id)) e->x = x; };
		lua["EnmSetY"] = [this](instance_id id, float y) { if (Enemy* e = FindEnemy(id)) e->y = y; };
		lua["EnmSetSpd"] = [this](instance_id id, float spd) { if (Enemy* e = FindEnemy(id)) e->spd = spd; };
		lua["EnmSetDir"] = [this](instance_id id, float dir) { if (Enemy* e = FindEnemy(id)) e->dir = dir; };
		lua["EnmSetAcc"] = [this](instance_id id, float acc) { if (Enemy* e = FindEnemy(id)) e->acc = acc; };
		lua["EnmSetRadius"] = [this](instance_id id, float radius) { if (Enemy* e = FindEnemy(id)) e->radius = radius; };

		// player

		lua["PlrGetX"] = [this]() { return player.x; };
		lua["PlrGetY"] = [this]() { return player.y; };

		// boss

		lua["BossGetX"] = [this]() { if (boss) return boss->x; else return 0.0f; };
		lua["BossGetY"] = [this]() { if (boss) return boss->y; else return 0.0f; };
		lua["BossGetSpd"] = [this]() { if (boss) return boss->spd; else return 0.0f; };
		lua["BossGetDir"] = [this]() { if (boss) return boss->dir; else return 0.0f; };
		lua["BossGetAcc"] = [this]() { if (boss) return boss->acc; else return 0.0f; };
		lua["BossGetRadius"] = [this]() { if (boss) return boss->radius; else return 0.0f; };

		lua["BossSetX"] = [this](float x) { if (boss) boss->x = x; };
		lua["BossSetY"] = [this](float y) { if (boss) boss->y = y; };
		lua["BossSetSpd"] = [this](float spd) { if (boss) boss->spd = spd; };
		lua["BossSetDir"] = [this](float dir) { if (boss) boss->dir = dir; };
		lua["BossSetAcc"] = [this](float acc) { if (boss) boss->acc = acc; };
		lua["BossSetRadius"] = [this](float radius) { if (boss) boss->radius = radius; };
	}

	void GameplayScene::BossStartPhase(Boss& b)
	{
		const BossPhase& p = b.phases[b.phase_index];
		b.hp = p.hp;
		b.timer = p.time;
		b.co_runner = sol::thread::create(lua);
		b.co = sol::nil;
		b.co = sol::coroutine(b.co_runner.thread_state(), p.script);
	}

	bool GameplayScene::BossEndPhase(Boss& b)
	{
		bullets.clear();
		if (b.phase_index + 1 >= b.phases.size()) {
			return false;
		}
		++b.phase_index;
		BossStartPhase(b);
		return true;
	}

	void GameplayScene::CheckResult(sol::protected_function_result pres)
	{
		if (!pres.valid()) {
			sol::error err = pres;
			Error(err.what());
		}
	}

	void GameplayScene::Error(const std::string& what)
	{
		game.next_scene = std::make_unique<ErrorScene>(game, what);
	}

	bool GameplayScene::InBounds(float x, float y)
	{
		float l = -OFFSET;
		float r = (float)(PLAY_AREA_W - 1) + OFFSET;
		float t = -OFFSET;
		float b = (float)(PLAY_AREA_H - 1) + OFFSET;
		return (x >= l && y >= t) && (x <= r && y <= b);
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

	Enemy* GameplayScene::FindEnemy(instance_id id)
	{
		size_t left = 0;
		size_t right = enemies.size() - 1;

		while (left <= right) {
			size_t middle = (left + right) / 2;
			if (enemies[middle].id < id) {
				left = middle + 1;
			} else if (enemies[middle].id > id) {
				right = middle - 1;
			} else {
				return &enemies[middle];
			}
		}

		return nullptr;
	}
}
