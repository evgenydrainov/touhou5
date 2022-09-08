#include "Stage.h"

#include "Game.h"
#include "Scenes/GameScene.h"

#include "cpml.h"
#include <fmt/format.h>
#include <iostream>

namespace th
{
	static bool in_bounds(float x, float y)
	{
		float l = -OFFSET;
		float r = float(PLAY_AREA_W - 1) + OFFSET;
		float t = -OFFSET;
		float b = float(PLAY_AREA_H - 1) + OFFSET;
		return (x >= l) && (y >= t) && (x <= r) && (y <= b);
	}

	Stage::Stage(Game& _game, GameScene& _scene) : game(_game), scene(_scene)
	{
		std::cout << "[TOUHOU] Loading script " << game.script_path.string() << "...\n";

		player.x = PLAYER_STARTING_X;
		player.y = PLAYER_STARTING_Y;

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

		lua["package"]["path"] = fmt::format(
			"{};{}",
			(game.script_path  / "?.lua").string(),
			(game.scripts_path / "?.lua").string()
		);

		// register_api()

		lua["Shoot"] = [this](float x, float y, float spd, float dir, float acc, int texture_id, float radius, bool rotate) -> instance_id
		{
			Bullet& b = bullets.emplace_back();
			b.id = next_id++;
			b.x = x;
			b.y = y;
			b.spd = spd;
			b.dir = cpml::angle_wrap(dir);
			b.acc = acc;
			b.texture_id = texture_id;
			b.radius = radius;
			b.rotate = rotate;
			PlaySound(scene.sndEnemyShoot);
			return b.id;
		};

		lua["CreateBoss"] = [this](sol::table desc) -> instance_id
		{
			Boss& b = boss.emplace();
			b.state = BOSS_STATE_WAITING;
			b.wait_timer = 60.0f;
			b.id = next_id++;
			b.x = float(PLAY_AREA_W) / 2.0f;
			b.y = float(PLAY_AREA_H) / 4.0f;
			b.radius = 20.0f;
			b.texture_id = desc["Texture"];
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
			if (sol::optional<std::string> m = desc["Music"]) {
				scene.play_music(m.value().c_str());
			}
			return b.id;
		};

		lua["CreateEnemy"] = [this](float x, float y, sol::table desc) -> instance_id
		{
			Enemy& e = enemies.emplace_back();
			e.id = next_id++;
			e.x = x;
			e.y = y;
			e.radius = 5.0f;
			e.texture_id = desc["Texture"];
			e.hp = desc["HP"];
			e.co_runner = sol::thread::create(lua);
			sol::coroutine script = desc["Script"];
			e.co = sol::coroutine(e.co_runner.thread_state(), script);
			return e.id;
		};

		lua["LoadTexture"] = [this](std::string fname) -> int
		{
			std::filesystem::path path = game.script_path / fname;
			loaded_textures.emplace_back(LoadTexture(path.string().c_str()));
			return loaded_textures.size() - 1;
		};

		// bullet

		lua["BltGetX"]			= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->x; else return 0.0f; };
		lua["BltGetY"]			= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->y; else return 0.0f; };
		lua["BltGetSpd"]		= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->spd; else return 0.0f; };
		lua["BltGetDir"]		= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->dir; else return 0.0f; };
		lua["BltGetAcc"]		= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->acc; else return 0.0f; };
		lua["BltGetRadius"]		= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->radius; else return 0.0f; };
		lua["BltGetLifetime"]	= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->lifetime; else return 0.0f; };

		lua["BltSetX"]		= [this](instance_id id, float x) { if (Bullet* b = find_bullet(id)) b->x = x; };
		lua["BltSetY"]		= [this](instance_id id, float y) { if (Bullet* b = find_bullet(id)) b->y = y; };
		lua["BltSetSpd"]	= [this](instance_id id, float spd) { if (Bullet* b = find_bullet(id)) b->spd = spd; };
		lua["BltSetDir"]	= [this](instance_id id, float dir) { if (Bullet* b = find_bullet(id)) b->dir = cpml::angle_wrap(dir); };
		lua["BltSetAcc"]	= [this](instance_id id, float acc) { if (Bullet* b = find_bullet(id)) b->acc = acc; };
		lua["BltSetRadius"]	= [this](instance_id id, float radius) { if (Bullet* b = find_bullet(id)) b->radius = radius; };

		// enemy

		lua["EnmGetX"]		= [this](instance_id id) { if (Enemy* e = find_enemy(id)) return e->x; else return 0.0f; };
		lua["EnmGetY"]		= [this](instance_id id) { if (Enemy* e = find_enemy(id)) return e->y; else return 0.0f; };
		lua["EnmGetSpd"]	= [this](instance_id id) { if (Enemy* e = find_enemy(id)) return e->spd; else return 0.0f; };
		lua["EnmGetDir"]	= [this](instance_id id) { if (Enemy* e = find_enemy(id)) return e->dir; else return 0.0f; };
		lua["EnmGetAcc"]	= [this](instance_id id) { if (Enemy* e = find_enemy(id)) return e->acc; else return 0.0f; };
		lua["EnmGetRadius"]	= [this](instance_id id) { if (Enemy* e = find_enemy(id)) return e->radius; else return 0.0f; };

		lua["EnmSetX"]		= [this](instance_id id, float x) { if (Enemy* e = find_enemy(id)) e->x = x; };
		lua["EnmSetY"]		= [this](instance_id id, float y) { if (Enemy* e = find_enemy(id)) e->y = y; };
		lua["EnmSetSpd"]	= [this](instance_id id, float spd) { if (Enemy* e = find_enemy(id)) e->spd = spd; };
		lua["EnmSetDir"]	= [this](instance_id id, float dir) { if (Enemy* e = find_enemy(id)) e->dir = dir; };
		lua["EnmSetAcc"]	= [this](instance_id id, float acc) { if (Enemy* e = find_enemy(id)) e->acc = acc; };
		lua["EnmSetRadius"]	= [this](instance_id id, float radius) { if (Enemy* e = find_enemy(id)) e->radius = radius; };

		// player

		lua["PlrGetX"] = [this]() { return player.x; };
		lua["PlrGetY"] = [this]() { return player.y; };

		// boss

		lua["BossGetX"]			= [this](instance_id id) { if (boss && boss->id == id) return boss->x; else return 0.0f; };
		lua["BossGetY"]			= [this](instance_id id) { if (boss && boss->id == id) return boss->y; else return 0.0f; };
		lua["BossGetSpd"]		= [this](instance_id id) { if (boss && boss->id == id) return boss->spd; else return 0.0f; };
		lua["BossGetDir"]		= [this](instance_id id) { if (boss && boss->id == id) return boss->dir; else return 0.0f; };
		lua["BossGetAcc"]		= [this](instance_id id) { if (boss && boss->id == id) return boss->acc; else return 0.0f; };
		lua["BossGetRadius"]	= [this](instance_id id) { if (boss && boss->id == id) return boss->radius; else return 0.0f; };
		lua["BossGetPhase"]		= [this](instance_id id) { if (boss && boss->id == id) return int(boss->phase_index); else return 0; };

		lua["BossSetX"]			= [this](instance_id id, float x) { if (boss && boss->id == id) boss->x = x; };
		lua["BossSetY"]			= [this](instance_id id, float y) { if (boss && boss->id == id) boss->y = y; };
		lua["BossSetSpd"]		= [this](instance_id id, float spd) { if (boss && boss->id == id) boss->spd = spd; };
		lua["BossSetDir"]		= [this](instance_id id, float dir) { if (boss && boss->id == id) boss->dir = dir; };
		lua["BossSetAcc"]		= [this](instance_id id, float acc) { if (boss && boss->id == id) boss->acc = acc; };
		lua["BossSetRadius"]	= [this](instance_id id, float radius) { if (boss && boss->id == id) boss->radius = radius; };

		lua["BltExists"]	= [this](instance_id id) -> bool { return find_bullet(id); };
		lua["EnmExists"]	= [this](instance_id id) -> bool { return find_enemy(id); };
		lua["BossExists"]	= [this](instance_id id) -> bool { return boss && boss->id == id; };

		try {
			sol::table res = lua.unsafe_script_file((game.script_path / "stage.lua").string());
			co_runner = sol::thread::create(lua);
			if (sol::optional<sol::coroutine> script = res["Script"]) {
				co = sol::coroutine(co_runner.thread_state(), script.value());
			}
			if (sol::optional<std::string> m = res["Music"]) {
				scene.play_music(m.value());
			}
		} catch (const sol::error& err) {
			quit_with_error(err.what());
		}
	}

	Stage::~Stage()
	{
		for (size_t i = loaded_textures.size(); i--;) {
			UnloadTexture(loaded_textures[i]);
		}
		loaded_textures.clear();
	}

	void Stage::update(float delta)
	{
		// player_update()

		player.hsp = 0.0f;
		player.vsp = 0.0f;
		player.is_focused = false;

		switch (player.state) {
			case PLAYER_STATE_NORMAL: {
				int h = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
				int v = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);
				player.is_focused = IsKeyDown(KEY_LEFT_SHIFT);

				float spd =
					player.is_focused ?
					game.characters[game.character_id].focus_spd :
					game.characters[game.character_id].move_spd;

				if (v == 0) {
					player.hsp = float(h) * spd;
				} else {
					player.hsp = float(h) * spd * cpml::sqrt2 * 0.5f;
				}

				if (h == 0) {
					player.vsp = float(v) * spd;
				} else {
					player.vsp = float(v) * spd * cpml::sqrt2 * 0.5f;
				}

				(this->*game.characters[game.character_id].shot_type)(delta);

				player_use_bombs(delta);

				player.iframes -= delta;
				if (player.iframes < 0.0f) player.iframes = 0.0f;

				break;
			}
			case PLAYER_STATE_DYING: {
				if ((PLAYER_DEATH_TIME - player.timer) < game.characters[game.character_id].deathbomb_time) {
					player_use_bombs(delta);
				}

				if (player.timer <= 0.0f) {

					// player_die()

					player = {};

					player.state = PLAYER_STATE_APPEARING;
					player.x = PLAYER_STARTING_X;
					player.y = PLAYER_STARTING_Y;
					player.timer = PLAYER_DEATH_TIME;
					player.iframes = PLAYER_RESPAWN_IFRAMES;

					scene.stats.power -= 16;
					if (scene.stats.power < 0) scene.stats.power = 0;

					if (scene.stats.lives > 0) {
						scene.stats.lives--;
					} else {
						scene.game_over();
					}

					break;
				}

				player.timer -= delta;
				if (player.timer < 0.0f) player.timer = 0.0f;

				break;
			}
			case PLAYER_STATE_APPEARING: {
				if (player.timer <= 0.0f) {
					player.state = PLAYER_STATE_NORMAL;
					break;
				}

				player.timer -= delta;
				if (player.timer < 0.0f) player.timer = 0.0f;

				break;
			}
		}

		player.hitbox_alpha = cpml::approach(
			player.hitbox_alpha,
			player.is_focused ? 1.0f : 0.0f,
			0.1f * delta
		);

		// do_coroutines()

		coroutine_timer += delta;
		coroutine_calls = 0;
		while (coroutine_timer >= COROUTINE_DELTA) {
			try {
				if (co.runnable()) {
					check_result(co());
				}
				if (boss) {
					if (boss->state == BOSS_STATE_NORMAL && boss->co.runnable()) {
						check_result(boss->co(boss->id));
					}
				}
				for (Enemy& e : enemies) {
					if (e.co.runnable()) {
						check_result(e.co(e.id));
					}
				}
			} catch (const sol::error& err) {
				quit_with_error(err.what());
			}

			coroutine_timer -= COROUTINE_DELTA;
			coroutine_calls++;
		}

		// do_physics()

		float physics_timer = delta;
		physics_calls = 0;
		while (physics_timer > 0.0f) {
			float pdelta = std::min(physics_timer, PHYSICS_DELTA);

			// physics_step()

			player.x += player.hsp * pdelta;
			player.y += player.vsp * pdelta;

			for (auto b = bullets.begin(); b != bullets.end();) {
				b->spd += b->acc * pdelta;
				b->x += cpml::lengthdir_x(b->spd, b->dir) * pdelta;
				b->y += cpml::lengthdir_y(b->spd, b->dir) * pdelta;
				if (player.state == PLAYER_STATE_NORMAL && player.iframes <= 0.0f) {
					if (cpml::circle_vs_circle(b->x, b->y, b->radius, player.x, player.y, game.characters[game.character_id].radius)) {

						// player_get_hit()

						player.state = PLAYER_STATE_DYING;
						player.timer = PLAYER_DEATH_TIME;
						PlaySound(scene.sndPichuun);

						b = bullets.erase(b);
						continue;
					}
				}
				b++;
			}

			physics_timer -= pdelta;
			physics_calls++;
		}

		player.x = std::clamp(player.x, 0.0f, float(PLAY_AREA_W - 1));
		player.y = std::clamp(player.y, 0.0f, float(PLAY_AREA_H - 1));

		for (auto b = bullets.begin(); b != bullets.end();) {
			if (!in_bounds(b->x, b->y)) {
				b = bullets.erase(b);
				continue;
			}
			if (player.state == PLAYER_STATE_NORMAL) {
				if (!b->grazed) {
					if (cpml::circle_vs_circle(b->x, b->y, b->radius, player.x, player.y, game.characters[game.character_id].graze_radius)) {
						++scene.stats.graze;
						b->grazed = true;
						PlaySound(scene.sndGraze);
					}
				}
			}
			b->lifetime += delta;
			b++;
		}

		for (auto b = player_bullets.begin(); b != player_bullets.end();) {
			b->spd += b->acc * delta;
			b->x += cpml::lengthdir_x(b->spd, b->dir) * delta;
			b->y += cpml::lengthdir_y(b->spd, b->dir) * delta;
			if (!in_bounds(b->x, b->y)) {
				b = player_bullets.erase(b);
				continue;
			}
			switch (b->type) {
				case PLAYER_BULLET_REIMU_CARD: {
					b->reimu_card.rotation -= 10.0f * delta;
					break;
				}
			}
			b++;
		}

		if (boss) {
			if (!boss_update(*boss, delta)) {
				boss = std::nullopt;
			}
		}

		for (auto e = enemies.begin(); e != enemies.end();) {
			if (!enemy_update(*e, delta)) {
				e = enemies.erase(e);
				continue;
			}
			e++;
		}

		for (auto p = pickups.begin(); p != pickups.end();) {
			p->vsp += p->grv * delta;
			p->vsp = std::min(p->vsp, p->max_vsp);
			p->y += p->vsp * delta;
			if (!in_bounds(p->x, p->y)) {
				p = pickups.erase(p);
				continue;
			}
			if (cpml::circle_vs_circle(p->x, p->y, p->radius, player.x, player.y, game.characters[game.character_id].radius)) {
				switch (p->type) {
					case PICKUP_POINTS: {
						++scene.stats.points;
						break;
					}
					case PICKUP_POWER: {
						get_power(1);
						break;
					}
				}
				p = pickups.erase(p);
				continue;
			}
			p++;
		}

#ifdef TH_DEBUG
		if (IsKeyPressed(KEY_L)) {
			get_lives(8);
		}
		if (IsKeyPressed(KEY_B)) {
			get_bombs(8);
		}
		if (IsKeyPressed(KEY_P)) {
			get_power(128);
		}
		if (game.god_mode) {
			player.iframes = 30.0f;
		}
		if (IsKeyPressed(KEY_S)) {
			if (boss) {
				boss_end_phase(*boss);
			}
		}
#endif
	}

	void Stage::player_use_bombs(float delta)
	{
		if (IsKeyPressed(KEY_X)) {
			if (scene.stats.bombs > 0) {
				(this->*game.characters[game.character_id].bomb)();
				scene.stats.bombs--;
				player.state = PLAYER_STATE_NORMAL;
			}
		}
	}

	bool Stage::boss_update(Boss& b, float delta)
	{
		b.spd += b.acc * delta;
		if (b.spd < 0.0f) b.spd = 0.0f;
		b.x += cpml::lengthdir_x(b.spd, b.dir) * delta;
		b.y += cpml::lengthdir_y(b.spd, b.dir) * delta;

		switch (b.state) {
			case BOSS_STATE_NORMAL: {
				for (auto bb = player_bullets.begin(); bb != player_bullets.end();) {
					if (cpml::circle_vs_circle(bb->x, bb->y, bb->radius, b.x, b.y, b.radius)) {
						b.hp -= bb->dmg;
						bb = player_bullets.erase(bb);
						PlaySound(scene.sndEnemyHit);
						if (b.hp <= 0.0f) {
							b.hp = 0.0f;
							if (!boss_end_phase(b)) {
								return false;
							}
							break;
						}
						continue;
					}
					bb++;
				}

				if (b.timer <= 0.0f) {
					if (!boss_end_phase(b)) {
						return false;
					}
				}

				b.timer -= delta;
				if (b.timer < 0.0f) b.timer = 0.0f;

				break;
			}
			case BOSS_STATE_WAITING: {
				if (b.wait_timer <= 0.0f) {

					b.state = BOSS_STATE_NORMAL;
					b.hp = b.phases[b.phase_index].hp;
					b.timer = b.phases[b.phase_index].time;
					b.co_runner = sol::thread::create(lua);
					b.co = sol::nil;
					b.co = sol::coroutine(b.co_runner.thread_state(), b.phases[b.phase_index].script);

					break;
				}

				b.wait_timer -= delta;
				if (b.wait_timer < 0.0f) b.wait_timer = 0.0f;

				break;
			}
		}

		return true;
	}

	void Stage::boss_start_phase(Boss& b)
	{
		b.state = BOSS_STATE_WAITING;
		b.wait_timer = BOSS_WAIT_TIME;
	}

	bool Stage::boss_end_phase(Boss& b)
	{
		bullets.clear();
		if (b.phase_index + 1 >= b.phases.size()) {
			return false;
		}
		b.phase_index++;
		boss_start_phase(b);
		return true;
	}

	bool Stage::enemy_update(Enemy& e, float delta)
	{
		e.spd += e.acc * delta;
		e.x += cpml::lengthdir_x(e.spd, e.dir) * delta;
		e.y += cpml::lengthdir_y(e.spd, e.dir) * delta;
		if (!in_bounds(e.x, e.y)) {
			return false;
		}
		for (auto b = player_bullets.begin(); b != player_bullets.end();) {
			if (cpml::circle_vs_circle(b->x, b->y, b->radius, e.x, e.y, e.radius)) {
				e.hp -= b->dmg;
				b = player_bullets.erase(b);
				if (e.hp <= 0.0f) {
					Pickup& p = pickups.emplace_back();
					p.x = e.x;
					p.y = e.y;
					p.vsp = -1.5f;
					p.grv = 0.025f;
					p.max_vsp = 2.0f;
					p.radius = 5.0f;
					// play death sound
					return false;
				}
				PlaySound(scene.sndEnemyHit);
				continue;
			}
			b++;
		}
		return true;
	}

	void Stage::reimu_shot_type(float delta)
	{
		if (player.reimu.fire_timer <= 0.0f) {
			if (player.reimu.fire_queue > 0) {
				if (scene.stats.power >= 128) {
					for (int i = 0; i < 4; i++) {
						PlayerBullet& b = player_bullets.emplace_back();
						b.x = player.x;
						b.y = player.y - 10.0f;
						b.spd = 12.0f;
						b.dir = 90.0f - 7.5f + 5.0f * float(i);
						b.radius = 10.0f;
						b.dmg = 15.0f;
						b.type = PLAYER_BULLET_REIMU_CARD;
					}
				} else if (scene.stats.power >= 32) {
					for (int i = 0; i < 3; i++) {
						PlayerBullet& b = player_bullets.emplace_back();
						b.x = player.x;
						b.y = player.y - 10.0f;
						b.spd = 12.0f;
						b.dir = 90.0f - 5.0f + 5.0f * float(i);
						b.radius = 10.0f;
						b.dmg = 15.0f;
						b.type = PLAYER_BULLET_REIMU_CARD;
					}
				} else if (scene.stats.power >= 8) {
					for (int i = 0; i < 2; i++) {
						PlayerBullet& b = player_bullets.emplace_back();
						b.x = player.x - 8.0f + 16.0f * float(i);
						b.y = player.y - 10.0f;
						b.spd = 12.0f;
						b.dir = 90.0f;
						b.radius = 10.0f;
						b.dmg = 15.0f;
						b.type = PLAYER_BULLET_REIMU_CARD;
					}
				} else {
					PlayerBullet& b = player_bullets.emplace_back();
					b.x = player.x;
					b.y = player.y - 10.0f;
					b.spd = 12.0f;
					b.dir = 90.0f;
					b.radius = 10.0f;
					b.dmg = 15.0f;
					b.type = PLAYER_BULLET_REIMU_CARD;
				}
				player.reimu.fire_timer = 4;
				player.reimu.fire_queue--;
				PlaySound(scene.sndReimuShoot);
			} else if (IsKeyDown(KEY_Z)) {
				player.reimu.fire_queue = 6;
			}
		} else {
			player.reimu.fire_timer -= delta;
		}
	}

	void Stage::reimu_bomb()
	{
		bullets.clear();
		player.iframes = PLAYER_RESPAWN_IFRAMES;
	}

	void Stage::draw(RenderTexture2D target, float delta)
	{
		// draw order: bosses, enemies, player, player bullets, enemy bullets, pickups

		BeginTextureMode(target);
		{
			ClearBackground(BLACK);

			if (boss) {
				Texture2D t = loaded_textures[boss->texture_id];
				DrawTextureCentered(t, boss->x, boss->y);
			}

			for (const Enemy& e : enemies) {
				Texture2D t = loaded_textures[e.texture_id];
				DrawTextureCentered(t, e.x, e.y);
			}

			// player_draw()
			{
				Texture2D player_tex = scene.texReimu;
				Color player_col = WHITE;
				float player_scale = 1.0f;

				switch (player.state) {
					case PLAYER_STATE_NORMAL: {
						if (player.iframes > 0.0f) {
							player_col.a = (int(game.time / 5.0f) % 2) ? 255 : 128;
						}
						break;
					}
					case PLAYER_STATE_DYING: {
						player_col.a = uint8_t(std::lerp(255.0f, 0.0f, 1.0f - player.timer / PLAYER_DEATH_TIME));
						break;
					}
					case PLAYER_STATE_APPEARING: {
						player_col.a = uint8_t(std::lerp(0.0f, 255.0f, 1.0f - player.timer / PLAYER_DEATH_TIME));
						break;
					}
				}

				DrawTextureCentered(player_tex, player.x, player.y, player_scale, player_scale, 0, player_col);
			}

			if (player.hitbox_alpha > 0.0f) {
				DrawTextureCentered(
					scene.texHitbox,
					player.x,
					player.y,
					1.0f,
					1.0f,
					game.time,
					{255, 255, 255, uint8_t(player.hitbox_alpha * 255.0f)}
				);
			}

			for (const PlayerBullet& b : player_bullets) {
				switch (b.type) {
					case PLAYER_BULLET_REIMU_CARD: {
						DrawTextureCentered(scene.texReimuCard, b.x, b.y, 1.5f, 1.5f, b.reimu_card.rotation, {255, 255, 255, 128});
						break;
					}
				}
			}

			for (const Bullet& b : bullets) {
				Texture2D t = loaded_textures[b.texture_id];
				float rot = 0.0f;
				if (b.rotate) rot = -b.dir;
				DrawTextureCentered(t, b.x, b.y, 1.0f, 1.0f, rot);
			}

			for (const Pickup& p : pickups) {

			}

			if (boss) {
				const char* text = TextFormat(
					"%s\n"
					"%f/%fhp\n"
					"phase %d/%d\n"
					"timer %f",
					boss->name.c_str(),
					boss->hp, boss->phases[boss->phase_index].hp,
					int(boss->phase_index), int(boss->phases.size()),
					boss->timer / 60.0f
				);
				DrawText(text, 0, 0, 10, WHITE);
			}

			if (game.show_hitboxes) {
				if (boss) {
					DrawCircleV({boss->x, boss->y}, boss->radius, {255, 0, 0, 128});
				}
				DrawCircleV({player.x, player.y}, game.characters[game.character_id].graze_radius, BLACK);
				DrawCircleV({player.x, player.y}, game.characters[game.character_id].radius, WHITE);
				for (const PlayerBullet& b : player_bullets) {
					DrawCircleV({b.x, b.y}, b.radius, {0, 0, 255, 128});
				}
				for (const Bullet& b : bullets) {
					DrawCircleV({b.x, b.y}, b.radius, RED);
				}
			}
		}
		EndTextureMode();
	}

	void Stage::get_power(int power)
	{
		while (power--) {
			if (scene.stats.power < 128) {
				scene.stats.power++;
				switch (scene.stats.power) {
					case 8:
					case 16:
					case 32:
					case 48:
					case 64:
					case 80:
					case 96:
					case 128: {
						PlaySound(scene.sndPowerUp);
						break;
					}
				}
			}
		}
	}

	void Stage::get_lives(int lives)
	{
		while (lives--) {
			if (scene.stats.lives < 8) {
				scene.stats.lives++;
				PlaySound(scene.snd1Up);
			}
		}
	}

	void Stage::get_bombs(int bombs)
	{
		while (bombs--) {
			if (scene.stats.bombs < 8) {
				scene.stats.bombs++;
			}
		}
	}

	void Stage::check_result(const sol::protected_function_result& pres)
	{
		if (!pres.valid()) {
			sol::error err = pres;
			quit_with_error(err.what());
		}
	}

	void Stage::quit_with_error(const std::string& what)
	{
		game.error_msg = what;
		game.next_scene = ERROR_SCENE;
	}

	typedef ptrdiff_t ssize;

	Bullet* Stage::find_bullet(instance_id id)
	{
		ssize left = 0;
		ssize right = (ssize)(bullets.size()) - 1;

		while (left <= right) {
			ssize middle = (left + right) / 2;
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

	Enemy* Stage::find_enemy(instance_id id)
	{
		ssize left = 0;
		ssize right = (ssize)(enemies.size()) - 1;

		while (left <= right) {
			ssize middle = (left + right) / 2;
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
