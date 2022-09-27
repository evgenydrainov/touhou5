#include "Stage.h"

#include "Game.h"
#include "Scenes/GameScene.h"

#include "cpml.h"
#include <raylibx.h>
#include <fmt/format.h>
#include <iostream>

namespace th
{
	static bool in_bounds(float x, float y)
	{
		float l = -OFFSET;
		float r = PLAY_AREA_W - 1.0f + OFFSET;
		float t = -OFFSET;
		float b = PLAY_AREA_H - 1.0f + OFFSET;
		return (x >= l) && (y >= t) && (x <= r) && (y <= b);
	}

	Stage::Stage(Game& _game, GameScene& _scene) : game(_game), scene(_scene)
	{
		std::cout << "[TOUHOU] Loading script " << game.script_path.string() << "...\n";

		player.x = PLAYER_STARTING_X;
		player.y = PLAYER_STARTING_Y;
		player.spr = game.characters[game.character_id].idle_spr;
		player.xscale = 1.0f;

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

		lua["PHASE_NONSPELL"]	= 0;
		lua["PHASE_SPELLCARD"]	= 1;
		lua["PLAY_AREA_W"]		= PLAY_AREA_W;
		lua["PLAY_AREA_H"]		= PLAY_AREA_H;
		lua["BOSS_STARTING_X"]	= BOSS_STARTING_X;
		lua["BOSS_STARTING_Y"]	= BOSS_STARTING_Y;

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
			b.id = next_id++;
			b.x = float(PLAY_AREA_W) / 2.0f;
			b.y = float(PLAY_AREA_H) / 4.0f;
			b.radius = 25.0f;
			b.xscale = 1.0f;
			//b.texture_id = desc["Texture"].get_or(-1);
			b.sprite_id = desc["Sprite"].get_or(-1);
			b.name = desc["Name"].get_or(std::string());

			if (sol::optional<std::string> m = desc["Music"]) {
				scene.play_music(m.value().c_str());
			}

			if (desc["Phases"].valid()) {
				sol::nested<std::vector<sol::table>> phases = desc["Phases"];
				for (const sol::table& phase : phases.value()) {
					BossPhase& p = b.phases.emplace_back();
					p.time = phase["Time"].get_or(30.0f) * 60.0f;
					p.hp = phase["HP"].get_or(1000.0f);
					p.type = phase["Type"].get_or(0);
					if (sol::optional<sol::coroutine> script = phase["Script"]) {
						p.script = sol::coroutine(lua, script.value());
					}
					if (p.type == PHASE_SPELLCARD) {
						p.name = phase["Name"].get_or(std::string());
					}
				}
			}

			if (b.phases.empty()) {
				boss = std::nullopt;
				return -1;
			}

			boss_start_phase(b);

			return b.id;
		};

		lua["CreateEnemy"] = [this](float x, float y, sol::table desc) -> instance_id
		{
			Enemy& e = enemies.emplace_back();
			e.id = next_id++;
			e.x = x;
			e.y = y;
			e.radius = 5.0f;
			e.texture_id = desc["Texture"].get_or(-1);
			e.hp = desc["HP"].get_or(100.0f);
			if (sol::optional<sol::coroutine> script = desc["Script"]) {
				e.co_runner = sol::thread::create(lua);
				e.co = sol::coroutine(e.co_runner.thread_state(), script.value());
			}
			return e.id;
		};

		lua["LoadTexture"] = [this](std::string fname) -> int
		{
			std::filesystem::path path = game.script_path / fname;
			loaded_textures.emplace_back(LoadTexture(path.string().c_str()));
			return loaded_textures.size() - 1;
		};

		lua["LoadSprite"] = [this](std::string fname, int frame_count, float anim_spd, int loop_frame) -> int
		{
			Sprite& spr = loaded_sprites.emplace_back();
			spr.tex = LoadTexture((game.script_path / fname).string().c_str());
			spr.frame_count = frame_count;
			spr.anim_spd = anim_spd;
			spr.loop_frame = loop_frame;
			return loaded_sprites.size() - 1;
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
		lua["EnmSetDir"]	= [this](instance_id id, float dir) { if (Enemy* e = find_enemy(id)) e->dir = cpml::angle_wrap(dir); };
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
		lua["BossGetSpr"]		= [this](instance_id id) { if (boss && boss->id == id) return boss->sprite_id; else return 0; };
		lua["BossIsActive"]		= [this](instance_id id) { if (boss && boss->id == id) return boss->wait_timer <= 0.0f; else return false; };

		lua["BossSetX"]			= [this](instance_id id, float x) { if (boss && boss->id == id) boss->x = x; };
		lua["BossSetY"]			= [this](instance_id id, float y) { if (boss && boss->id == id) boss->y = y; };
		lua["BossSetSpd"]		= [this](instance_id id, float spd) { if (boss && boss->id == id) boss->spd = spd; };
		lua["BossSetDir"]		= [this](instance_id id, float dir) { if (boss && boss->id == id) boss->dir = cpml::angle_wrap(dir); };
		lua["BossSetAcc"]		= [this](instance_id id, float acc) { if (boss && boss->id == id) boss->acc = acc; };
		lua["BossSetRadius"]	= [this](instance_id id, float radius) { if (boss && boss->id == id) boss->radius = radius; };
		lua["BossSetSpr"]		= [this](instance_id id, int sprite_id)
		{
			if (boss && boss->id == id) {
				boss->sprite_id = sprite_id;
				boss->frame_index = 0.0f;
			}
		};

		lua["BltExists"]	= [this](instance_id id) -> bool { return find_bullet(id); };
		lua["EnmExists"]	= [this](instance_id id) -> bool { return find_enemy(id); };
		lua["BossExists"]	= [this](instance_id id) -> bool { return boss && boss->id == id; };

		try {
			sol::table res = lua.unsafe_script_file((game.script_path / "stage.lua").string());
			if (sol::optional<sol::coroutine> script = res["Script"]) {
				co_runner = sol::thread::create(lua);
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
		for (size_t i = loaded_sprites.size(); i--;) {
			UnloadTexture(loaded_sprites[i].tex);
		}
		loaded_sprites.clear();

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
				{
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
				}

				(this->*game.characters[game.character_id].shot_type)(delta);

				player_use_bombs(delta);

				player.iframes -= delta;
				if (player.iframes < 0.0f) player.iframes = 0.0f;

				if (player.hsp != 0.0f) {
					if (player.spr.tex.id != game.characters[game.character_id].turn_spr.tex.id) {
						player.spr = game.characters[game.character_id].turn_spr;
						player.frame_index = 0.0f;
					}

					if (player.hsp > 0.0f) {
						player.xscale = -1.0f;
					} else {
						player.xscale = 1.0f;
					}
				}

				player_animate(delta);

				break;
			}
			case PLAYER_STATE_DYING: {
				if ((PLAYER_DEATH_TIME - player.timer) < game.characters[game.character_id].deathbomb_time) {
					player_use_bombs(delta);
				}

				if (player.timer <= 0.0f) {

					// player_die()

					player = {};
					player.spr = game.characters[game.character_id].idle_spr;
					player.xscale = 1.0f;

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

				player_animate(delta);

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
					if (boss->wait_timer <= 0.0f) {
						if (boss->co.runnable()) {
							check_result(boss->co(boss->id));
						}
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
				if (b->spd < 0.0f) b->spd = 0.0f;
				b->x += cpml::lengthdir_x(b->spd, b->dir) * pdelta;
				b->y += cpml::lengthdir_y(b->spd, b->dir) * pdelta;
				if (player.state == PLAYER_STATE_NORMAL && player.iframes <= 0.0f) {
					if (cpml::circle_vs_circle(b->x, b->y, b->radius, player.x, player.y, game.characters[game.character_id].radius)) {
						player_get_hit();
						b = bullets.erase(b);
						continue;
					}
				}
				b++;
			}

			physics_timer -= pdelta;
			physics_calls++;
		}

		player.x = std::clamp(player.x, 0.0f, PLAY_AREA_W - 1.0f);
		player.y = std::clamp(player.y, 0.0f, PLAY_AREA_H - 1.0f);

		for (auto b = bullets.begin(); b != bullets.end();) {
			if (!in_bounds(b->x, b->y)) {
				b = bullets.erase(b);
				continue;
			}
			if (player.state == PLAYER_STATE_NORMAL) {
				if (!b->grazed) {
					if (cpml::circle_vs_circle(b->x, b->y, b->radius, player.x, player.y, game.characters[game.character_id].graze_radius)) {
						scene.stats.graze++;
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
				case PLAYER_BULLET_REIMU_ORB_SHOT: {
					if (boss && boss->wait_timer <= 0.0f) {
						//float dir = cpml::point_direction(b->x, b->y, boss->x, boss->y);
						//float diff = cpml::angle_difference(dir, b->dir);
						////b->dir += std::clamp(diff, -5.0f, 5.0f);
						//b->dir += diff * 0.1f;

						float hsp = cpml::lengthdir_x(b->spd, b->dir);
						float vsp = cpml::lengthdir_y(b->spd, b->dir);
						float dx = boss->x - b->x;
						float dy = boss->y - b->y;
						dx = std::clamp(dx, -12.0f, 12.0f);
						dy = std::clamp(dy, -12.0f, 12.0f);
						hsp = cpml::approach(hsp, dx, 1.0f);
						vsp = cpml::approach(vsp, dy, 1.0f);
						//hsp = std::lerp(hsp, dx, 0.1f);
						//vsp = std::lerp(vsp, dy, 0.1f);
						//hsp += dx * 0.1f;
						//vsp += dy * 0.1f;
						b->spd = cpml::point_distance(0.0f, 0.0f, hsp, vsp);
						//if (b->spd > 12.0f) b->spd = 12.0f;
						b->dir = cpml::point_direction(0.0f, 0.0f, hsp, vsp);
					}
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
			if (cpml::circle_vs_circle(p->x, p->y, p->radius, player.x, player.y, game.characters[game.character_id].graze_radius)) {
				switch (p->type) {
					case PICKUP_POINT:	scene.stats.points++;	break;
					case PICKUP_POWER:	get_power(1);			break;
				}
				p = pickups.erase(p);
				continue;
			}
			p++;
		}

		player_dps_timer += delta;
		if (player_dps_timer >= 60.0f) {
			player_dps = player_dps_dealt / (player_dps_timer / 60.0f);
			player_dps_dealt = 0.0f;
			player_dps_timer = 0.0f;
		}

		// detect end of level

		if (!co.runnable() && !boss && bullets.empty()) {

		}

#ifdef TH_DEBUG
		if (game.god_mode) {
			player.iframes = 30.0f;
		}
		if (IsKeyPressed(KEY_S)) {
			if (boss) {
				if (!boss_end_phase(*boss)) {
					boss = std::nullopt;
				}
			}
		}
#endif
	}

	void Stage::player_get_hit()
	{
		player.state = PLAYER_STATE_DYING;
		player.timer = PLAYER_DEATH_TIME;
		PlaySound(scene.sndPichuun);
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

	void Stage::player_animate(float delta)
	{
		if (player.hsp == 0.0f && player.spr.tex.id == game.characters[game.character_id].turn_spr.tex.id) {
			if (player.frame_index > float(player.spr.loop_frame - 1)) player.frame_index = float(player.spr.loop_frame - 1);
			player.frame_index -= player.spr.anim_spd * delta;
			if (player.frame_index < 0.0f) {
				player.spr = game.characters[game.character_id].idle_spr;
				player.frame_index = 0.0f;
			}
		} else {
			player.frame_index += player.spr.anim_spd * delta;
		}

		if (player.frame_index >= float(player.spr.frame_count)) {
			player.frame_index = float(player.spr.loop_frame + (int(player.frame_index) - player.spr.loop_frame) % (player.spr.frame_count - player.spr.loop_frame));
		}
	}

	static float launch_towards_point(float xfrom, float yfrom, float target_x, float target_y, float acc)
	{
		if (acc < 0.0f) acc = -acc;
		float dist = cpml::point_distance(xfrom, yfrom, target_x, target_y);
		return std::sqrt(dist * acc * 2.0f);
	}

	bool Stage::boss_update(Boss& b, float delta)
	{
		b.spd += b.acc * delta;
		if (b.spd < 0.0f) b.spd = 0.0f;
		b.x += cpml::lengthdir_x(b.spd, b.dir) * delta;
		b.y += cpml::lengthdir_y(b.spd, b.dir) * delta;

		if (b.sprite_id >= 0) {
			b.frame_index += loaded_sprites[b.sprite_id].anim_spd * delta;
			if (b.frame_index >= float(loaded_sprites[b.sprite_id].frame_count)) {
				b.frame_index = float(loaded_sprites[b.sprite_id].loop_frame + (int(b.frame_index) - loaded_sprites[b.sprite_id].loop_frame) % (loaded_sprites[b.sprite_id].frame_count - loaded_sprites[b.sprite_id].loop_frame));
			}
		}

		if (b.spd > 0.0f) {
			if (90.0f <= b.dir && b.dir < 270.0f) {
				b.xscale = -1.0f;
			} else {
				b.xscale = 1.0f;
			}
		}

		if (b.wait_timer <= BOSS_WAIT_TIME) {
			if (b.wait_flag == 0) {
				b.acc = -0.02f;
				b.dir = cpml::point_direction(b.x, b.y, BOSS_STARTING_X, BOSS_STARTING_Y);
				b.spd = launch_towards_point(b.x, b.y, BOSS_STARTING_X, BOSS_STARTING_Y, 0.02f);

				if (b.phases[b.phase_index].type == PHASE_SPELLCARD) {
					// spawn background
				}

				b.wait_flag = 1;
			}
		}

		bool alive = true;
		if (b.wait_timer <= 0.0f) {
			for (auto bb = player_bullets.begin(); bb != player_bullets.end();) {
				if (cpml::circle_vs_circle(bb->x, bb->y, bb->radius, b.x, b.y, b.radius)) {
					b.hp -= bb->dmg;

					player_dps_dealt += bb->dmg;

					bb = player_bullets.erase(bb);
					PlaySound(scene.sndEnemyHit);
					if (b.hp <= 0.0f) {
						b.hp = 0.0f;
						if (!boss_end_phase(b)) {
							alive = false;
						}
						break;
					}
					continue;
				}
				bb++;
			}

			if (b.timer <= 0.0f) {
				if (!boss_end_phase(b)) {
					alive = false;
				}
			}

			b.timer -= delta;
			if (b.timer < 0.0f) b.timer = 0.0f;
		}

		b.wait_timer -= delta;
		if (b.wait_timer < 0.0f) b.wait_timer = 0.0f;

		return alive;
	}

	void Stage::boss_start_phase(Boss& b)
	{
		if (b.phases[b.phase_index].type == PHASE_SPELLCARD) {
			b.wait_timer = BOSS_WAIT_TIME * 2.0f;
		} else {
			b.wait_timer = BOSS_WAIT_TIME;
		}
		b.wait_flag = 0;

		b.hp = b.phases[b.phase_index].hp;
		b.timer = b.phases[b.phase_index].time;
		b.co_runner = sol::thread::create(lua);
		b.co = sol::nil;
		b.co = sol::coroutine(b.co_runner.thread_state(), b.phases[b.phase_index].script);
	}

	bool Stage::boss_end_phase(Boss& b)
	{
		bullets.clear();
		if (b.phase_index + 1 >= b.phases.size()) {
			return false;
		}
		if (b.phase_index != b.phases.size() - 1) {
			if (b.phases[b.phase_index].type == PHASE_SPELLCARD) {
				Pickup& p = pickups.emplace_back();
				p.x = b.x;
				p.y = b.y;
				p.vsp = -1.5f;
				p.grv = 0.025f;
				p.max_vsp = 2.0f;
				p.radius = 5.0f;
			}
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
		//
		// 75 DPS@no power, 150@full
		// 
		// 15 SHOTS PER SECOND
		// 
		// Cards do 2/3 of DPS and homing orbs do 1/3
		// 
		// POWER LEVELS
		//   8: more cards
		//  16: more orbs
		//  32: more cards
		//  48: more orbs
		//  64: more orbs
		//  80: more orbs
		//  96: more orbs
		// 128: more cards
		//

		if (player.reimu.fire_queue == 0) {
			if (IsKeyDown(KEY_Z)) {
				player.reimu.fire_queue = 8;
			}
		}

		if (player.reimu.fire_queue > 0) {
			if (player.reimu.fire_timer <= 0.0f) {
				//std::cout << (8 - player.reimu.fire_queue) << '\n';

				{
					constexpr float card_fraction = 2.0f / 3.0f;
					constexpr float shots_per_sec = 15.0f - 1.0f;

					if (scene.stats.power >= 128) {
						constexpr float dps = std::lerp(75.0f, 150.0f, 3.0f / 3.0f);
						constexpr float shot_count = 4.0f;
						constexpr float card_dmg = dps * card_fraction / shots_per_sec / shot_count;

						for (int i = 0; i < 4; i++) {
							PlayerBullet& b = player_bullets.emplace_back();
							b.x = player.x;
							b.y = player.y - 10.0f;
							b.spd = 12.0f;
							b.dir = 90.0f - 7.5f + 5.0f * float(i);
							b.radius = 12.0f;
							b.dmg = card_dmg;
							b.type = PLAYER_BULLET_REIMU_CARD;
						}
					} else if (scene.stats.power >= 32) {
						constexpr float dps = std::lerp(75.0f, 150.0f, 2.0f / 3.0f);
						constexpr float shot_count = 3.0f;
						constexpr float card_dmg = dps * card_fraction / shots_per_sec / shot_count;

						for (int i = 0; i < 3; i++) {
							PlayerBullet& b = player_bullets.emplace_back();
							b.x = player.x;
							b.y = player.y - 10.0f;
							b.spd = 12.0f;
							b.dir = 90.0f - 5.0f + 5.0f * float(i);
							b.radius = 12.0f;
							b.dmg = card_dmg;
							b.type = PLAYER_BULLET_REIMU_CARD;
						}
					} else if (scene.stats.power >= 8) {
						constexpr float dps = std::lerp(75.0f, 150.0f, 1.0f / 3.0f);
						constexpr float shot_count = 2.0f;
						constexpr float card_dmg = dps * card_fraction / shots_per_sec / shot_count;

						for (int i = 0; i < 2; i++) {
							PlayerBullet& b = player_bullets.emplace_back();
							b.x = player.x - 8.0f + 16.0f * float(i);
							b.y = player.y - 10.0f;
							b.spd = 12.0f;
							b.dir = 90.0f;
							b.radius = 12.0f;
							b.dmg = card_dmg;
							b.type = PLAYER_BULLET_REIMU_CARD;
						}
					} else {
						constexpr float dps = std::lerp(75.0f, 150.0f, 0.0f / 3.0f);
						constexpr float shot_count = 1.0f;
						constexpr float card_dmg = dps * card_fraction / shots_per_sec / shot_count;

						PlayerBullet& b = player_bullets.emplace_back();
						b.x = player.x;
						b.y = player.y - 10.0f;
						b.spd = 12.0f;
						b.dir = 90.0f;
						b.radius = 12.0f;
						b.dmg = card_dmg;
						b.type = PLAYER_BULLET_REIMU_CARD;
					}
				}

				{
					constexpr float orb_fraction = 1.0f / 3.0f;

					if (scene.stats.power >= 96) {
						constexpr float dps = std::lerp(75.0f, 150.0f, 5.0f / 5.0f);
						constexpr float shots_per_sec = 15.0f - 1.0f;
						constexpr float shot_count = 2.0f;
						constexpr float orb_dmg = dps * orb_fraction / shots_per_sec / shot_count;

						float off = 30.0f + 15.0f * float((8 - player.reimu.fire_queue) % 4);

						for (int i = 0; i < 2; i++) {
							PlayerBullet& b = player_bullets.emplace_back();
							b.x = player.x;
							b.y = player.y;
							b.spd = 12.0f;
							b.dir = 90.0f + (i == 0 ? -off : off);
							b.radius = 12.0f;
							b.dmg = orb_dmg;
							b.type = PLAYER_BULLET_REIMU_ORB_SHOT;
						}
					} else if (scene.stats.power >= 80) {
						constexpr float dps = std::lerp(75.0f, 150.0f, 4.0f / 5.0f);

					} else if (scene.stats.power >= 64) {
						constexpr float dps = std::lerp(75.0f, 150.0f, 3.0f / 5.0f);

					} else if (scene.stats.power >= 48) {
						constexpr float dps = std::lerp(75.0f, 150.0f, 2.0f / 5.0f);

					} else if (scene.stats.power >= 16) {
						constexpr float dps = std::lerp(75.0f, 150.0f, 1.0f / 5.0f);

					} else {
						constexpr float dps = std::lerp(75.0f, 150.0f, 0.0f / 5.0f);
						constexpr float shots_per_sec = 60.0f / (4.0f * 8.0f);
						constexpr float shot_count = 2.0f;
						constexpr float orb_dmg = dps * orb_fraction / shots_per_sec / shot_count;

						if (player.reimu.fire_queue == 8) {
							for (int i = 0; i < 2; i++) {
								PlayerBullet& b = player_bullets.emplace_back();
								b.x = player.x;
								b.y = player.y;
								b.spd = 12.0f;
								b.dir = 90.0f + 70.0f * (i == 0 ? -1.0f : 1.0f);
								b.radius = 12.0f;
								b.dmg = orb_dmg;
								b.type = PLAYER_BULLET_REIMU_ORB_SHOT;
							}
						}
					}
				}

				player.reimu.fire_timer = 4.0f;
				player.reimu.fire_queue--;
				PlaySound(scene.sndReimuShoot);
			}

			player.reimu.fire_timer -= delta;
			if (player.reimu.fire_timer < 0.0f) player.reimu.fire_timer = 0.0f;
		}
	}

	void Stage::reimu_bomb()
	{
		bullets.clear();
		player.iframes = PLAYER_RESPAWN_IFRAMES;
	}

	static void draw_sprite(Sprite spr, int frame_index, float x, float y, float xscale = 1.0f, float yscale = 1.0f, float rotation = 0.0f, Color tint = WHITE)
	{
		if (frame_index >= spr.frame_count) {
			frame_index = spr.loop_frame + (frame_index - spr.loop_frame) % (spr.frame_count - spr.loop_frame);
		}
		int w = spr.tex.width / spr.frame_count;
		int u = frame_index * w;
		if (xscale < 0.0f) {
			w = -w;
			xscale = -xscale;
		}
		Rectangle src = {float(u), 0.0f, float(w), float(spr.tex.height)};
		Rectangle dest = {x, y, std::abs(src.width) * xscale, src.height * yscale};
		Vector2 origin = {dest.width / 2.0f, dest.height / 2.0f};
		DrawTexturePro(spr.tex, src, dest, origin, rotation, tint);
	}

	void Stage::draw(RenderTexture2D target, float delta)
	{
		BeginTextureMode(target);
		{
			ClearBackground(BLACK);

			if (boss) {
				//if (boss->texture_id >= 0) {
				//	Texture2D t = loaded_textures[boss->texture_id];
				//	DrawTextureCentered(t, boss->x, boss->y);
				//}
				if (boss->sprite_id >= 0) {
					draw_sprite(loaded_sprites[boss->sprite_id], boss->frame_index, boss->x, boss->y, boss->xscale, 1.0f, 0.0f, WHITE);
				}
			}

			for (const Enemy& e : enemies) {
				if (e.texture_id >= 0) {
					Texture2D t = loaded_textures[e.texture_id];
					DrawTextureCentered(t, e.x, e.y);
				}
			}

			for (const PlayerBullet& b : player_bullets) {
				switch (b.type) {
					case PLAYER_BULLET_REIMU_CARD: {
						DrawTextureCentered(scene.texReimuCard, b.x, b.y, 1.5f, 1.5f, b.reimu_card.rotation, {255, 255, 255, 100});
						break;
					}
					case PLAYER_BULLET_REIMU_ORB_SHOT: {
						DrawTextureCentered(scene.texReimuOrbShot, b.x, b.y, 1.5f, 1.5f, b.dir, {255, 255, 255, 100});
						break;
					}
				}
			}

			// player_draw()
			{
				Color player_col = WHITE;
				float player_xscale = 1.0f;
				float player_yscale = 1.0f;

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

				draw_sprite(player.spr, player.frame_index, player.x, player.y, player.xscale * player_xscale, player_yscale, 0.0f, player_col);

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
			}

			for (const Pickup& p : pickups) {
				Texture2D t = {};
				switch (p.type) {
					case PICKUP_POWER:		t = scene.texPickupPower;		break;
					case PICKUP_POINT:		t = scene.texPickupPoint;		break;
					case PICKUP_BIG_POWER:	t = scene.texPickupBigPower;	break;
					case PICKUP_BOMB:		t = scene.texPickupBomb;		break;
					case PICKUP_FULL_POWER:	t = scene.texPickupFullPower;	break;
					case PICKUP_1UP:		t = scene.texPickup1Up;			break;
					case PICKUP_SCORE:		t = scene.texPickupScore;		break;
				}
				DrawTextureCentered(t, p.x, p.y);
			}

			for (const Bullet& b : bullets) {
				if (b.texture_id >= 0) {
					Texture2D t = loaded_textures[b.texture_id];
					float rot = 0.0f;
					if (b.rotate) rot = -b.dir;
					DrawTextureCentered(t, b.x, b.y, 1.0f, 1.0f, rot);
				}
			}

			if (boss) {
				{
					const char* text = TextFormat("%d", int(boss->phases.size() - boss->phase_index - 1));
					DrawTextEx(game.font, text, {0, 0}, game.font.baseSize, 1, WHITE);
				}

				{
					const char* text = TextFormat("%d", int(boss->timer / 60.0f));
					int w = MeasureTextEx(game.font, text, game.font.baseSize, 1).x;
					DrawTextEx(game.font, text, {float(PLAY_AREA_W - w), 0}, game.font.baseSize, 1, WHITE);
				}

				{
					int healthbar_x = 32 + 2;
					int healthbar_y = 4;
					int healthbar_w = PLAY_AREA_W - 64 - 4;
					int healthbar_h = 4;
					DrawRectangle(healthbar_x, healthbar_y, int(float(healthbar_w) * boss->hp / boss->phases[boss->phase_index].hp), healthbar_h, WHITE);
				}

				DrawText(boss->name.c_str(), 16, 20, 10, WHITE);

				if (boss->phases[boss->phase_index].type == PHASE_SPELLCARD) {
					const char* text = boss->phases[boss->phase_index].name.c_str();
					int w = MeasureText(text, 10);
					DrawText(text, PLAY_AREA_W - 16 - w, 20, 10, WHITE);
				}
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
				for (const Pickup& p : pickups) {
					DrawCircleV({p.x, p.y}, p.radius, {255, 255, 255, 128});
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
					case 128: PlaySound(scene.sndPowerUp); break;
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
			} else {
				get_bombs(1);
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

	void Stage::get_points(int points)
	{
		while (points--) {
			scene.stats.points++;
			if (scene.stats.points >= 800) {
				if (scene.stats.points % 200 == 0) {
					get_lives(1);
				}
			} else {
				switch (scene.stats.points) {
					case 50:
					case 125:
					case 200:
					case 300:
					case 450: get_lives(1); break;
				}
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
