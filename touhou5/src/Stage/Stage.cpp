#include "Stage.h"

#include "Game.h"
#include "Scenes/GameScene.h"

#include "cpml.h"
#include <raylibx.h>
#include <fmt/format.h>
#include <iostream>

namespace th
{
	static bool in_bounds(float x, float y, float offset)
	{
		float l = -offset;
		float r = float(PLAY_AREA_W - 1) + offset;
		float t = -offset;
		float b = float(PLAY_AREA_H - 1) + offset;
		return (x >= l) && (y >= t) && (x <= r) && (y <= b);
	}

	instance_id Stage::CreateBossEx(float x, float y, float radius, const sol::table& desc, bool midboss)
	{
		Boss& b = boss.emplace();
		b.id = next_id++;
		b.wait_timer = BOSS_WAIT_TIME * 1.5f;
		b.x = x;
		b.y = y;
		b.radius = radius;
		b.xscale = 1.f;
		b.sprite_id = desc["Sprite"].get_or(-1);
		b.name = desc["Name"].get_or(std::string());
		b.midboss = midboss;

		if (sol::optional<std::string> m = desc["Music"]) {
			scene.play_music(m.value().c_str());
		}

		if (desc["Phases"].valid()) {
			sol::nested<std::vector<sol::table>> phases = desc["Phases"];
			for (const sol::table& phase : phases.value()) {
				BossPhase& p = b.phases.emplace_back();
				p.time = phase["Time"].get_or(30.f) * 60.f;
				p.hp = phase["HP"].get_or(1000.f);
				p.type = phase["Type"].get_or(PHASE_NONSPELL);
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
	}

	instance_id Stage::CreateEnemyEx(float x, float y, float spd, float dir, float acc, float radius, float hp, int sprite_id, const sol::coroutine& script)
	{
		Enemy& e = enemies.emplace_back();
		e.id = next_id++;
		e.x = x;
		e.y = y;
		e.spd = spd;
		e.dir = dir;
		e.acc = acc;
		e.radius = radius;
		e.sprite_id = sprite_id;
		e.hp = hp;
		if (script.valid()) {
			e.co_runner = std::make_unique<sol::thread>(sol::thread::create(lua));
			e.co = std::make_unique<sol::coroutine>(e.co_runner->thread_state(), script);
		}
		return e.id;
	}

	Stage::Stage(Game& _game, GameScene& _scene) : game(_game), scene(_scene)
	{
		std::cout << "[TOUHOU] Loading script " << game.script_path.string() << "...\n";

		rengine.seed(0);

		player.x = PLAYER_STARTING_X;
		player.y = PLAYER_STARTING_Y;
		player.spr = game.characters[game.character_id].idle_spr;
		player.xscale = 1.f;

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

		lua["PHASE_NONSPELL"]	= PHASE_NONSPELL;
		lua["PHASE_SPELLCARD"]	= PHASE_SPELLCARD;
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

			b.type = PROJECTILE_BULLET;
			b.radius = radius;
			b.rotate = rotate;

			PlaySound(scene.sndEnemyShoot);
			return b.id;
		};

		lua["ShootLazer"] = [this](float x, float y, float spd, float dir, float acc, int texture_id, float length, float thickness) -> instance_id
		{
			if (spd == 0.f) {
				spd = 1.f;
			}

			Bullet& b = bullets.emplace_back();
			b.id = next_id++;
			b.x = x;
			b.y = y;
			b.spd = spd;
			b.dir = cpml::angle_wrap(dir);
			b.acc = acc;
			b.texture_id = texture_id;

			b.type = PROJECTILE_LAZER;
			b.length = length;
			b.thickness = thickness;

			PlaySound(scene.sndLazer);
			return b.id;
		};

		lua["ShootRect"] = [this](float x, float y, float spd, float dir, float acc, int texture_id, float radius) -> instance_id
		{
			Bullet& b = bullets.emplace_back();
			b.id = next_id++;
			b.x = x;
			b.y = y;
			b.spd = spd;
			b.dir = cpml::angle_wrap(dir);
			b.acc = acc;
			b.texture_id = texture_id;

			b.type = PROJECTILE_RECT;
			b.radius = radius;

			PlaySound(scene.sndEnemyShoot);
			return b.id;
		};

		lua["CreateBoss"] = [this](sol::table desc) -> instance_id
		{
			return CreateBossEx(BOSS_STARTING_X, BOSS_STARTING_Y, 25.f, desc, false);
		};

		lua["CreateMidboss"] = [this](sol::table desc) -> instance_id
		{
			return CreateBossEx(BOSS_STARTING_X, BOSS_STARTING_Y, 25.f, desc, true);
		};

		lua["CreateBossEx"] = [this](float x, float y, float radius, sol::table desc, bool midboss) -> instance_id
		{
			return CreateBossEx(x, y, radius, desc, midboss);
		};

		lua["CreateEnemy"] = [this](float x, float y, float spd, float dir, float acc, float hp, int sprite_id, sol::coroutine script) -> instance_id
		{
			return CreateEnemyEx(x, y, spd, dir, acc, 25.f, hp, sprite_id, script);
		};

		lua["CreateEnemyEx"] = [this](float x, float y, float spd, float dir, float acc, float radius, float hp, int sprite_id, sol::coroutine script) -> instance_id
		{
			return CreateEnemyEx(x, y, spd, dir, acc, radius, hp, sprite_id, script);
		};

		lua["LoadTexture"] = [this](std::string fname)
		{
			std::filesystem::path path = game.script_path / fname;
			Texture2D t = LoadTexture(path.string().c_str());
			loaded_textures.emplace_back(t);
			return loaded_textures.size() - 1;
		};

		lua["LoadSprite"] = [this](std::string fname, int frame_count, float anim_spd, int loop_frame)
		{
			std::filesystem::path path = game.script_path / fname;
			loaded_sprites.emplace_back(LoadSprite(path.string().c_str(), frame_count, anim_spd, loop_frame));
			return loaded_sprites.size() - 1;
		};

		lua["Random"] = [this]()
		{
			return random();
		};

		lua["math"]["random"] = [](sol::variadic_args va)
		{
			throw sol::error("Cannot use built-in random number generator");
		};

		// bullet

		lua["BltGetX"]			= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->x;          else return 0.f; };
		lua["BltGetY"]			= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->y;          else return 0.f; };
		lua["BltGetSpd"]		= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->spd;        else return 0.f; };
		lua["BltGetDir"]		= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->dir;        else return 0.f; };
		lua["BltGetAcc"]		= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->acc;        else return 0.f; };
		lua["BltGetRadius"]		= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->radius;     else return 0.f; };
		lua["BltGetLifetime"]	= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->lifetime;   else return 0.f; };
		lua["BltGetTex"]		= [this](instance_id id) { if (Bullet* b = find_bullet(id)) return b->texture_id; else return 0;    };

		lua["BltSetX"]		= [this](instance_id id, float x)        { if (Bullet* b = find_bullet(id)) b->x = x; };
		lua["BltSetY"]		= [this](instance_id id, float y)        { if (Bullet* b = find_bullet(id)) b->y = y; };
		lua["BltSetSpd"]	= [this](instance_id id, float spd)      { if (Bullet* b = find_bullet(id)) b->spd = spd; };
		lua["BltSetDir"]	= [this](instance_id id, float dir)      { if (Bullet* b = find_bullet(id)) b->dir = cpml::angle_wrap(dir); };
		lua["BltSetAcc"]	= [this](instance_id id, float acc)      { if (Bullet* b = find_bullet(id)) b->acc = acc; };
		lua["BltSetRadius"]	= [this](instance_id id, float radius)   { if (Bullet* b = find_bullet(id)) b->radius = radius; };
		lua["BltSetTex"]	= [this](instance_id id, int texture_id) { if (Bullet* b = find_bullet(id)) b->texture_id = texture_id; };

		// enemy

		lua["EnmGetX"]		= [this](instance_id id) { if (Enemy* e = find_enemy(id)) return e->x;      else return 0.f; };
		lua["EnmGetY"]		= [this](instance_id id) { if (Enemy* e = find_enemy(id)) return e->y;      else return 0.f; };
		lua["EnmGetSpd"]	= [this](instance_id id) { if (Enemy* e = find_enemy(id)) return e->spd;    else return 0.f; };
		lua["EnmGetDir"]	= [this](instance_id id) { if (Enemy* e = find_enemy(id)) return e->dir;    else return 0.f; };
		lua["EnmGetAcc"]	= [this](instance_id id) { if (Enemy* e = find_enemy(id)) return e->acc;    else return 0.f; };
		lua["EnmGetRadius"]	= [this](instance_id id) { if (Enemy* e = find_enemy(id)) return e->radius; else return 0.f; };

		lua["EnmSetX"]		= [this](instance_id id, float x)      { if (Enemy* e = find_enemy(id)) e->x = x; };
		lua["EnmSetY"]		= [this](instance_id id, float y)      { if (Enemy* e = find_enemy(id)) e->y = y; };
		lua["EnmSetSpd"]	= [this](instance_id id, float spd)    { if (Enemy* e = find_enemy(id)) e->spd = spd; };
		lua["EnmSetDir"]	= [this](instance_id id, float dir)    { if (Enemy* e = find_enemy(id)) e->dir = cpml::angle_wrap(dir); };
		lua["EnmSetAcc"]	= [this](instance_id id, float acc)    { if (Enemy* e = find_enemy(id)) e->acc = acc; };
		lua["EnmSetRadius"]	= [this](instance_id id, float radius) { if (Enemy* e = find_enemy(id)) e->radius = radius; };

		// player

		lua["PlrGetX"] = [this]() { return player.x; };
		lua["PlrGetY"] = [this]() { return player.y; };

		// boss

		lua["BossGetX"]			= [this](instance_id id) { if (boss && boss->id == id) return boss->x;           else return 0.f; };
		lua["BossGetY"]			= [this](instance_id id) { if (boss && boss->id == id) return boss->y;           else return 0.f; };
		lua["BossGetSpd"]		= [this](instance_id id) { if (boss && boss->id == id) return boss->spd;         else return 0.f; };
		lua["BossGetDir"]		= [this](instance_id id) { if (boss && boss->id == id) return boss->dir;         else return 0.f; };
		lua["BossGetAcc"]		= [this](instance_id id) { if (boss && boss->id == id) return boss->acc;         else return 0.f; };
		lua["BossGetRadius"]	= [this](instance_id id) { if (boss && boss->id == id) return boss->radius;      else return 0.f; };
		lua["BossGetPhase"]		= [this](instance_id id) { if (boss && boss->id == id) return boss->phase_index; else return 0_sz; };
		lua["BossGetSpr"]		= [this](instance_id id) { if (boss && boss->id == id) return boss->sprite_id;   else return 0; };
		lua["BossIsActive"]		= [this](instance_id id)
		{
			if (boss && boss->id == id) {
				return boss->wait_timer <= 0.f;
			} else {
				return false;
			}
		};

		lua["BossSetX"]			= [this](instance_id id, float x)      { if (boss && boss->id == id) boss->x = x; };
		lua["BossSetY"]			= [this](instance_id id, float y)      { if (boss && boss->id == id) boss->y = y; };
		lua["BossSetSpd"]		= [this](instance_id id, float spd)    { if (boss && boss->id == id) boss->spd = spd; };
		lua["BossSetDir"]		= [this](instance_id id, float dir)    { if (boss && boss->id == id) boss->dir = cpml::angle_wrap(dir); };
		lua["BossSetAcc"]		= [this](instance_id id, float acc)    { if (boss && boss->id == id) boss->acc = acc; };
		lua["BossSetRadius"]	= [this](instance_id id, float radius) { if (boss && boss->id == id) boss->radius = radius; };
		lua["BossSetSpr"]		= [this](instance_id id, int sprite_id)
		{
			if (boss && boss->id == id) {
				boss->sprite_id = sprite_id;
				boss->frame_index = 0.f;
			}
		};

		lua["BltExists"]	= [this](instance_id id) { return find_bullet(id) != nullptr; };
		lua["EnmExists"]	= [this](instance_id id) { return find_enemy(id)  != nullptr; };
		lua["BossExists"]	= [this](instance_id id) { return boss && (boss->id == id); };

		lua["BGCamSetPos"]		= [this](float x, float y, float z) { bg_cam.position = {x, y, z}; };
		lua["BGCamSetTarget"]	= [this](float x, float y, float z) { bg_cam.target   = {x, y, z}; };
		lua["BGFogSetOrigin"]	= [this](float x, float y, float z) { fogOrigin       = {x, y, z}; };
		lua["BGFogSetColor"]	= [this](float r, float g, float b, float a) { fogColor = {r/255.f, g/255.f, b/255.f, a/255.f}; };
		lua["BGFogSetNear"]		= [this](float near)  { fogNear    = near; };
		lua["BGFogSetFar"]		= [this](float far)   { fogFar     = far; };
		lua["BGFogEnable"]		= [this](bool enable) { fogEnabled = enable; };

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

		bg_cam.up         = {0.f, 1.f, 0.f};
		bg_cam.fovy       = 45.f;
		bg_cam.projection = CAMERA_PERSPECTIVE;

		{
			std::filesystem::path model_path = game.script_path / "background.obj";
			if (std::filesystem::exists(model_path)) {
				bg_model = LoadModel(model_path.string().c_str());

				bg_surf = LoadStrippedRenderTexture(PLAY_AREA_W, PLAY_AREA_H);

				{
					std::filesystem::path tex_path = game.script_path / "background.png";
					if (std::filesystem::exists(tex_path)) {
						bg_tex = LoadTexture(tex_path.string().c_str());
						SetTextureFilter(bg_tex, TEXTURE_FILTER_BILINEAR);
						bg_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = bg_tex;
					}
				}

				{
					std::filesystem::path vs_path = game.script_path / "background.vs";
					std::filesystem::path fs_path = game.script_path / "background.fs";
					if (std::filesystem::exists(vs_path) && std::filesystem::exists(fs_path)) {
						bg_shader = LoadShader(vs_path.string().c_str(), fs_path.string().c_str());
					} else {
						bg_shader = LoadShader("background.vs", "background.fs");
					}
				}

				bg_model.materials[0].shader = bg_shader;

				u_texel			= GetShaderLocation(bg_shader, "u_texelSize");
				u_time			= GetShaderLocation(bg_shader, "u_time");
				u_fogOrigin		= GetShaderLocation(bg_shader, "u_fogOrigin");
				u_fogColor		= GetShaderLocation(bg_shader, "u_fogColor");
				u_fogNear		= GetShaderLocation(bg_shader, "u_fogNear");
				u_fogFar		= GetShaderLocation(bg_shader, "u_fogFar");
				u_fogEnabled	= GetShaderLocation(bg_shader, "u_fogEnabled");
			}
		}

		{
			std::filesystem::path spell_bg_path = game.script_path / "spell_background.png";
			spell_bg_tex = LoadTexture(spell_bg_path.string().c_str());
			if (spell_bg_tex.id) {
				SetTextureFilter(spell_bg_tex, TEXTURE_FILTER_BILINEAR);
			}
		}
	}

	Stage::~Stage()
	{
		UnloadTexture(spell_bg_tex);
		UnloadShader(bg_shader);
		UnloadTexture(bg_tex);
		UnloadModel(bg_model);
		UnloadRenderTexture(bg_surf);

		for (size_t i = loaded_sprites.size(); i--;) {
			UnloadSprite(loaded_sprites[i]);
		}
		loaded_sprites.clear();

		for (size_t i = loaded_textures.size(); i--;) {
			UnloadTexture(loaded_textures[i]);
		}
		loaded_textures.clear();
	}

	void Stage::update(float delta)
	{
		delta *= gameplay_delta;

#ifdef TH_DEBUG
		if (game.god_mode) {
			player.iframes = 30.f;
		}
		if (IsKeyPressed(KEY_S)) {
			if (boss) {
				if (!boss_end_phase(*boss)) {
					boss = std::nullopt;
				}
			}
		}
		if (IsKeyPressed(KEY_THREE)) {
			if (!debug_3d) {
				SetCameraMode(bg_cam, CAMERA_FREE);
				debug_3d = true;
			} else {
				SetCameraMode(bg_cam, CAMERA_CUSTOM);
				debug_3d = false;
			}
		}
#endif

		if (debug_3d) {
			UpdateCamera(&bg_cam);
		}

		// player_update()

		player.hsp = 0.f;
		player.vsp = 0.f;
		player.is_focused = false;

		switch (player.state) {
			case PLAYER_STATE_NORMAL: {
				{
					int h = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
					int v = IsKeyDown(KEY_DOWN)  - IsKeyDown(KEY_UP);
					player.is_focused = IsKeyDown(KEY_LEFT_SHIFT);

					float spd =
						player.is_focused ?
						game.characters[game.character_id].focus_spd :
						game.characters[game.character_id].move_spd;

					if (v == 0) {
						player.hsp = float(h) * spd;
					} else {
						player.hsp = float(h) * spd * cpml::sqrt2 * .5f;
					}

					if (h == 0) {
						player.vsp = float(v) * spd;
					} else {
						player.vsp = float(v) * spd * cpml::sqrt2 * .5f;
					}
				}

				if (game.characters[game.character_id].shot_type) (this->*game.characters[game.character_id].shot_type)(delta);

				player_use_bombs(delta);

				player.iframes -= delta;
				if (player.iframes < 0.f) player.iframes = 0.f;

				if (player.hsp != 0.f) {
					if (player.spr.tex.id != game.characters[game.character_id].turn_spr.tex.id) {
						player.spr = game.characters[game.character_id].turn_spr;
						player.frame_index = 0.f;
					}

					if (player.hsp > 0.f) {
						player.xscale = -1.f;
					} else {
						player.xscale = 1.f;
					}
				}

				player_animate(delta);

				break;
			}
			case PLAYER_STATE_DYING: {
				if ((PLAYER_DEATH_TIME - player.timer) < game.characters[game.character_id].deathbomb_time) {
					player_use_bombs(delta);
				}

				if (player.timer > 0.f) {
					player.timer -= delta;
					if (player.timer < 0.f) player.timer = 0.f;
				} else {

					// player_die()

					player = {};
					player.spr = game.characters[game.character_id].idle_spr;
					player.xscale = 1.f;

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

				break;
			}
			case PLAYER_STATE_APPEARING: {
				if (player.timer > 0.f) {
					player.timer -= delta;
					if (player.timer < 0.f) player.timer = 0.f;
				} else {
					player.state = PLAYER_STATE_NORMAL;
				}

				player_animate(delta);

				break;
			}
		}

		// do_physics()

		float physics_timer = delta;
		physics_calls = 0;
		while (physics_timer > 0.f) {
			float pdelta = std::min(physics_timer, PHYSICS_DELTA);

			// physics_step()

			player.x += player.hsp * pdelta;
			player.y += player.vsp * pdelta;

			for (auto b = bullets.begin(); b != bullets.end();) {
				bool skip = false;
				if (b->type == PROJECTILE_LAZER) {
					float t = b->length / b->spd;
					if (b->lifetime < t) skip = true;
				}
				if (!skip) {
					b->spd += b->acc * pdelta;
					if (b->spd < 0.f) b->spd = 0.f;
					b->x += cpml::lengthdir_x(b->spd, b->dir) * pdelta;
					b->y += cpml::lengthdir_y(b->spd, b->dir) * pdelta;
				}
				if (b->lifetime >= 10.f) {
					if (player.state == PLAYER_STATE_NORMAL && player.iframes <= 0.f) {
						bool col = false;
						switch (b->type) {
							case PROJECTILE_BULLET: {
								col = cpml::circle_vs_circle(b->x, b->y, b->radius, player.x, player.y, game.characters[game.character_id].radius);
								break;
							}
							case PROJECTILE_LAZER: {
								float t = b->length / b->spd;
								float l = b->length;
								if (b->lifetime < t) l = b->lifetime * b->spd;
								float rect_x = b->x;
								float rect_y = b->y;
								rect_x += cpml::lengthdir_x(l / 2.f, b->dir);
								rect_y += cpml::lengthdir_y(l / 2.f, b->dir);
								col = cpml::circle_vs_rotated_rect(player.x, player.y, game.characters[game.character_id].radius, rect_x, rect_y, b->thickness, l, b->dir);
								break;
							}
							case PROJECTILE_RECT: {
								col = cpml::circle_vs_rotated_rect(player.x, player.y, game.characters[game.character_id].radius, b->x, b->y, b->radius * 2.f, b->radius * 2.f, b->dir);
								break;
							}
						}
						if (col) {
							player_get_hit();
							b = bullets.erase(b);
							continue;
						}
					}
				}
				b++;
			}

			physics_timer -= pdelta;
			physics_calls++;
		}

		player.x = std::clamp(player.x, 0.f, float(PLAY_AREA_W - 1));
		player.y = std::clamp(player.y, 0.f, float(PLAY_AREA_H - 1));

		for (auto b = bullets.begin(); b != bullets.end();) {
			if (!in_bounds(b->x, b->y, 100.f)) {
				b = bullets.erase(b);
				continue;
			}
			if (b->lifetime > 60.f * 60.f) {
				b = bullets.erase(b);
				continue;
			}
			if (player.state == PLAYER_STATE_NORMAL) {
				if (!b->grazed) {
					bool col = false;
					switch (b->type) {
						case PROJECTILE_BULLET: {
							col = cpml::circle_vs_circle(b->x, b->y, b->radius, player.x, player.y, game.characters[game.character_id].graze_radius);
							break;
						}
						case PROJECTILE_LAZER: {
							float rect_x = b->x;
							float rect_y = b->y;
							rect_x += cpml::lengthdir_x(b->length / 2.f, b->dir);
							rect_y += cpml::lengthdir_y(b->length / 2.f, b->dir);
							col = cpml::circle_vs_rotated_rect(player.x, player.y, game.characters[game.character_id].graze_radius, rect_x, rect_y, b->thickness, b->length, b->dir);
							break;
						}
						case PROJECTILE_RECT: {
							col = cpml::circle_vs_rotated_rect(player.x, player.y, game.characters[game.character_id].graze_radius, b->x, b->y, b->radius * 2.f, b->radius * 2.f, b->dir);
							break;
						}
					}
					if (col) {
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
			if (!in_bounds(b->x, b->y, 0.f)) {
				b = player_bullets.erase(b);
				continue;
			}
			switch (b->type) {
				case PLAYER_BULLET_REIMU_CARD: {
					b->reimu_card.rotation -= 16.f * delta;
					break;
				}
				case PLAYER_BULLET_REIMU_ORB_SHOT: {
					Enemy* enemy_target = nullptr;
					float enemy_dist = 0.f;
					for (Enemy& e : enemies) {
						float dist = cpml::point_distance(b->x, b->y, e.x, e.y);
						if (dist < enemy_dist || !enemy_target) {
							enemy_target = &e;
							enemy_dist = dist;
						}
					}
					Boss* boss_target = nullptr;
					float boss_dist = 0.f;
					if (boss && boss->wait_timer <= 0.0f) {
						boss_target = &(*boss);
						boss_dist = cpml::point_distance(b->x, b->y, boss->x, boss->y);
					}
					if (enemy_target || boss_target) {
						float target_x;
						float target_y;
						if (enemy_target && boss_target) {
							target_x = (enemy_dist < boss_dist) ? enemy_target->x : boss_target->x;
							target_y = (enemy_dist < boss_dist) ? enemy_target->y : boss_target->y;
						} else if (enemy_target) {
							target_x = enemy_target->x;
							target_y = enemy_target->y;
						} else {
							target_x = boss_target->x;
							target_y = boss_target->y;
						}

						float hsp = cpml::lengthdir_x(b->spd, b->dir);
						float vsp = cpml::lengthdir_y(b->spd, b->dir);
						float dx = target_x - b->x;
						float dy = target_y - b->y;
						dx = std::clamp(dx, -12.f, 12.f);
						dy = std::clamp(dy, -12.f, 12.f);
						hsp = cpml::approach(hsp, dx, 1.5f * delta);
						vsp = cpml::approach(vsp, dy, 1.5f * delta);
						b->spd = cpml::point_distance(0.f, 0.f, hsp, vsp);
						b->dir = cpml::point_direction(0.f, 0.f, hsp, vsp);
					}
					break;
				}
			}
			b++;
		}

		if (boss) {
			Boss& b = *boss;
			b.spd += b.acc * delta;
			if (b.spd < 0.f) b.spd = 0.f;
			b.x += cpml::lengthdir_x(b.spd, b.dir) * delta;
			b.y += cpml::lengthdir_y(b.spd, b.dir) * delta;
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
			if (!in_bounds(p->x, p->y, 100.f)) {
				p = pickups.erase(p);
				continue;
			}
			if (cpml::circle_vs_circle(p->x, p->y, p->radius, player.x, player.y, game.characters[game.character_id].graze_radius)) {
				switch (p->type) {
					case PICKUP_POINT: scene.get_points(1); break;
					case PICKUP_POWER: scene.get_power(1);  break;
				}
				p = pickups.erase(p);
				PlaySound(scene.sndPickup);
				continue;
			}
			p++;
		}

		// do_coroutines()

		coroutine_timer += delta;
		coroutine_calls = 0;
		while (coroutine_timer >= COROUTINE_DELTA) {
			try {
				if (co.runnable()) {
					check_result(co.call());
				}
				if (boss) {
					if (boss->wait_timer <= 0.f) {
						if (boss->co.runnable()) {
							check_result(boss->co.call(boss->id));
						}
					}
				}
				for (Enemy& e : enemies) {
					if (e.co) {
						if (e.co->runnable()) {
							check_result(e.co->call(e.id));
						}
					}
				}
			} catch (const sol::error& err) {
				quit_with_error(err.what());
				boss = std::nullopt;
				return;
			}

			coroutine_timer -= COROUTINE_DELTA;
			coroutine_calls++;
		}

		if (boss) {
			if (boss->phases[boss->phase_index].type == PHASE_SPELLCARD && boss->wait_flag == 1) {
				spell_bg_alpha = cpml::approach(spell_bg_alpha, 1.f, 1.f / 30.f * delta);
			} else {
				spell_bg_alpha = 0.f;
			}

			if (!boss_update(*boss, delta)) {
				boss = std::nullopt;
			}
		} else {
			spell_bg_alpha = 0.f;
		}

		// detect end of level

		if (!co.runnable() && !boss && bullets.empty() && enemies.empty()) {
			if (win_timer < 170.f) {
				win_timer += delta;
			} else if (win_timer < 200.f) {
				if (pickups.empty()) {
					win_timer += delta;
				}
			} else {
				scene.win();
			}
		}

		player.hitbox_alpha = cpml::approach(
			player.hitbox_alpha,
			player.is_focused ? 1.f : 0.f,
			.1f * delta
		);

		player_dps_timer += delta;
		if (player_dps_timer >= 60.f) {
			player_dps = player_dps_dealt / (player_dps_timer / 60.f);
			player_dps_dealt = 0.f;
			player_dps_timer = 0.f;
		}

		time += delta;
	}

	float Stage::random()
	{
		std::uniform_real_distribution<float> dist(0.f, 1.f);
		float r = dist(rengine);
		return r;
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
				if (game.characters[game.character_id].bomb) (this->*game.characters[game.character_id].bomb)();
				scene.stats.bombs--;
				player.state = PLAYER_STATE_NORMAL;
			}
		}
	}

	void Stage::player_animate(float delta)
	{
		if (player.hsp == 0.f && player.spr.tex.id == game.characters[game.character_id].turn_spr.tex.id) {
			if (player.frame_index > float(player.spr.loop_frame - 1)) player.frame_index = float(player.spr.loop_frame - 1);
			player.frame_index -= player.spr.anim_spd * delta;
			if (player.frame_index < 0.f) {
				player.spr = game.characters[game.character_id].idle_spr;
				player.frame_index = 0.f;
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
		if (acc < 0.f) acc = -acc;
		float dist = cpml::point_distance(xfrom, yfrom, target_x, target_y);
		return std::sqrt(dist * acc * 2.f);
	}

	bool Stage::boss_update(Boss& b, float delta)
	{
		if (b.spd > 0.f) {
			if (90.f <= b.dir && b.dir < 270.f) {
				b.xscale = -1.f;
			} else {
				b.xscale = 1.f;
			}
		}

		if (b.wait_timer <= BOSS_WAIT_TIME) {
			if (b.wait_flag == 0) {
				b.acc = -.05f;
				b.dir = cpml::point_direction(b.x, b.y, BOSS_STARTING_X, BOSS_STARTING_Y);
				b.spd = launch_towards_point(b.x, b.y, BOSS_STARTING_X, BOSS_STARTING_Y, -b.acc);

				if (b.phases[b.phase_index].type == PHASE_SPELLCARD) {
					PlaySound(scene.sndSpellCard);
				}

				b.wait_flag = 1;
			}
		}

		bool alive = true;
		if (b.wait_timer <= 0.f) {
			for (auto bb = player_bullets.begin(); bb != player_bullets.end();) {
				if (cpml::circle_vs_circle(bb->x, bb->y, bb->radius, b.x, b.y, b.radius)) {
					b.hp -= bb->dmg;

					player_dps_dealt += bb->dmg;

					bb = player_bullets.erase(bb);
					PlaySound(scene.sndEnemyHit);
					if (b.hp <= 0.f) {
						b.hp = 0.f;
						if (!boss_end_phase(b)) {
							alive = false;
						}
						break;
					}
					continue;
				}
				bb++;
			}

			if (b.timer > 0.f) {
				b.timer -= delta;
				if (b.timer < 0.f) b.timer = 0.f;
			} else {
				if (!boss_end_phase(b)) {
					alive = false;
				}
			}
		}

		b.wait_timer -= delta;
		if (b.wait_timer < 0.f) b.wait_timer = 0.f;

		if (b.sprite_id >= 0) {
			b.frame_index += loaded_sprites[b.sprite_id].anim_spd * delta;
			if (b.frame_index >= float(loaded_sprites[b.sprite_id].frame_count)) {
				b.frame_index = float(loaded_sprites[b.sprite_id].loop_frame + (int(b.frame_index) - loaded_sprites[b.sprite_id].loop_frame) % (loaded_sprites[b.sprite_id].frame_count - loaded_sprites[b.sprite_id].loop_frame));
			}
		}

		return alive;
	}

	void Stage::boss_start_phase(Boss& b)
	{
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

		PlaySound(scene.sndEnemyDie);

		if (b.phase_index + 1_sz >= b.phases.size()) {
			if (!b.midboss) {
				PlaySound(scene.sndBossDie);
			}
			return false;
		}

		if (b.phases[b.phase_index].type == PHASE_NONSPELL) {
			b.wait_timer = BOSS_WAIT_TIME;
		} else {
			b.wait_timer = BOSS_WAIT_TIME * 1.5f;
		}

		if (b.phases[b.phase_index].type == PHASE_SPELLCARD) {
			Pickup& p = pickups.emplace_back();
			p.x       = b.x;
			p.y       = b.y;
			p.vsp     = -1.5f;
			p.grv     = .025f;
			p.max_vsp = 2.f;
			p.radius  = 5.f;
		}

		b.phase_index++;
		boss_start_phase(b);

		return true;
	}

	bool Stage::enemy_update(Enemy& e, float delta)
	{
		e.spd += e.acc * delta;
		if (e.spd < 0.f) e.spd = 0.f;
		e.x += cpml::lengthdir_x(e.spd, e.dir) * delta;
		e.y += cpml::lengthdir_y(e.spd, e.dir) * delta;

		if (!in_bounds(e.x, e.y, 0.f)) {
			return false;
		}

		for (auto b = player_bullets.begin(); b != player_bullets.end();) {
			if (cpml::circle_vs_circle(b->x, b->y, b->radius, e.x, e.y, e.radius)) {
				e.hp -= b->dmg;

				player_dps_dealt += b->dmg;

				b = player_bullets.erase(b);
				PlaySound(scene.sndEnemyHit);
				if (e.hp <= 0.f) {
					Pickup& p = pickups.emplace_back();
					p.x = e.x;
					p.y = e.y;
					p.vsp = -1.5f;
					p.grv = .025f;
					p.max_vsp = 2.f;
					p.radius = 5.f;

					if (random() < .5f) {
						p.type = PICKUP_POINT;
					} else {
						p.type = PICKUP_POWER;
					}

					PlaySound(scene.sndEnemyDie);
					return false;
				}
				continue;
			}
			b++;
		}

		if (e.sprite_id >= 0) {
			e.frame_index += loaded_sprites[e.sprite_id].anim_spd * delta;
			if (e.frame_index >= float(loaded_sprites[e.sprite_id].frame_count)) {
				e.frame_index = float(loaded_sprites[e.sprite_id].loop_frame + (int(e.frame_index) - loaded_sprites[e.sprite_id].loop_frame) % (loaded_sprites[e.sprite_id].frame_count - loaded_sprites[e.sprite_id].loop_frame));
			}
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

		player.reimu.fire_timer += delta;
		while (player.reimu.fire_timer >= 4.f) {
			if (player.reimu.fire_queue == 0) {
				if (IsKeyDown(KEY_Z)) {
					player.reimu.fire_queue = 8;
				}
			}

			if (player.reimu.fire_queue > 0) {
				int level = 0;
				int card_shot_type = 0;
				int orb_shot_type = 0;

				if (scene.stats.power >= 128) {
					level = 8;
				} else if (scene.stats.power >= 96) {
					level = 7;
				} else if (scene.stats.power >= 80) {
					level = 6;
				} else if (scene.stats.power >= 64) {
					level = 5;
				} else if (scene.stats.power >= 48) {
					level = 4;
				} else if (scene.stats.power >= 32) {
					level = 3;
				} else if (scene.stats.power >= 16) {
					level = 2;
				} else if (scene.stats.power >= 8) {
					level = 1;
				}

				if (scene.stats.power >= 128) {
					card_shot_type = 3;
				} else if (scene.stats.power >= 32) {
					card_shot_type = 2;
				} else if (scene.stats.power >= 8) {
					card_shot_type = 1;
				}

				if (scene.stats.power >= 96) {
					orb_shot_type = 5;
				} else if (scene.stats.power >= 80) {
					orb_shot_type = 4;
				} else if (scene.stats.power >= 64) {
					orb_shot_type = 3;
				} else if (scene.stats.power >= 48) {
					orb_shot_type = 2;
				} else if (scene.stats.power >= 16) {
					orb_shot_type = 1;
				}

				float min_dps = 75.f;
				float max_dps = 150.f;
				float dps = std::lerp(min_dps, max_dps, float(level) / 8.f);

				{
					float card_fraction = 2.f / 3.f;
					float card_dps = dps * card_fraction;
					float shots_per_sec = 15.f;

					switch (card_shot_type) {
						case 0: {
							int shot_count = 1;
							float card_dmg = card_dps / shots_per_sec / float(shot_count);

							PlayerBullet& b = player_bullets.emplace_back();
							b.x             = player.x;
							b.y             = player.y - 10.f;
							b.spd           = 16.f;
							b.dir           = 90.f;
							b.radius        = 12.f;
							b.dmg           = card_dmg;
							b.type          = PLAYER_BULLET_REIMU_CARD;
							break;
						}
						case 1: {
							int shot_count = 2;
							float card_dmg = card_dps / shots_per_sec / float(shot_count);

							for (int i = 0; i < shot_count; i++) {
								PlayerBullet& b = player_bullets.emplace_back();
								b.x             = player.x - 8.f + float(i) * 16.f;
								b.y             = player.y - 10.f;
								b.spd           = 16.f;
								b.dir           = 90.f;
								b.radius        = 12.f;
								b.dmg           = card_dmg;
								b.type          = PLAYER_BULLET_REIMU_CARD;
							}
							break;
						}
						case 2: {
							int shot_count = 3;
							float card_dmg = card_dps / shots_per_sec / float(shot_count);

							for (int i = 0; i < shot_count; i++) {
								PlayerBullet& b = player_bullets.emplace_back();
								b.x             = player.x;
								b.y             = player.y - 10.f;
								b.spd           = 16.f;
								b.dir           = 90.f - 5.f + float(i) * 5.f;
								b.radius        = 12.f;
								b.dmg           = card_dmg;
								b.type          = PLAYER_BULLET_REIMU_CARD;
							}
							break;
						}
						case 3: {
							int shot_count = 4;
							float card_dmg = card_dps / shots_per_sec / float(shot_count);

							for (int i = 0; i < shot_count; i++) {
								PlayerBullet& b = player_bullets.emplace_back();
								b.x             = player.x;
								b.y             = player.y - 10.f;
								b.spd           = 16.f;
								b.dir           = 90.f - 7.5f + float(i) * 5.f;
								b.radius        = 12.f;
								b.dmg           = card_dmg;
								b.type          = PLAYER_BULLET_REIMU_CARD;
							}
							break;
						}
					}
				}

				{
					int frame = 8 - player.reimu.fire_queue;
					float orb_fraction = 1.f / 3.f;
					float orb_dps = dps * orb_fraction;

					switch (orb_shot_type) {
						case 0:
						case 1:
						case 2:
						case 3:
						case 4: {
							int shot_count = 2;
							float shots_per_sec = 60.f / (4.f * 8.f);
							float orb_dmg = orb_dps / shots_per_sec / float(shot_count);

							if (frame % 8 == 0) {
								for (int i = 0; i < shot_count; i++) {
									PlayerBullet& b = player_bullets.emplace_back();
									b.x             = player.x;
									b.y             = player.y;
									b.spd           = 12.f;
									b.dir           = 90.f + 70.f * ((i == 0) ? -1.f : 1.f);
									b.radius        = 12.f;
									b.dmg           = orb_dmg;
									b.type          = PLAYER_BULLET_REIMU_ORB_SHOT;
								}
							}
							break;
						}
						case 5: {
							int shot_count = 2;
							float shots_per_sec = 15.f;
							float orb_dmg = orb_dps / shots_per_sec / float(shot_count);

							float off = 30.f + 15.f * float(frame % 4);

							for (int i = 0; i < shot_count; i++) {
								PlayerBullet& b = player_bullets.emplace_back();
								b.x             = player.x;
								b.y             = player.y;
								b.spd           = 12.f;
								b.dir           = 90.f + ((i == 0) ? -off : off);
								b.radius        = 12.f;
								b.dmg           = orb_dmg;
								b.type          = PLAYER_BULLET_REIMU_ORB_SHOT;
							}
							break;
						}
					}
				}

				player.reimu.fire_queue--;
				PlaySound(scene.sndReimuShoot);
			}

			player.reimu.fire_timer -= 4.f;
		}
	}

	void Stage::reimu_bomb()
	{
		bullets.clear();
		player.iframes = PLAYER_RESPAWN_IFRAMES;
	}

	static void draw_sprite(Sprite spr, int frame_index, float x, float y, float xscale = 1.f, float yscale = 1.f, float rotation = 0.f, Color tint = WHITE)
	{
		if (frame_index >= spr.frame_count) {
			frame_index = spr.loop_frame + (frame_index - spr.loop_frame) % (spr.frame_count - spr.loop_frame);
		}
		int w = spr.tex.width / spr.frame_count;
		int u = frame_index * w;
		if (xscale < 0.f) {
			w = -w;
			xscale = -xscale;
		}
		Rectangle src = {float(u), 0.f, float(w), float(spr.tex.height)};
		Rectangle dest = {x, y, fabsf(src.width) * xscale, src.height * yscale};
		Vector2 origin = {dest.width / 2.f, dest.height / 2.f};
		DrawTexturePro(spr.tex, src, dest, origin, rotation, tint);
	}

	void Stage::draw(RenderTexture2D target, float delta)
	{
		delta *= gameplay_delta;

		if (bg_model.meshCount > 0 && spell_bg_alpha < 1.f) {
			{
				Vector2 texel;
				texel.x = 1.f / float(bg_tex.width);
				texel.y = 1.f / float(bg_tex.height);
				SetShaderValue(bg_shader, u_texel, &texel, SHADER_UNIFORM_VEC2);
				SetShaderValue(bg_shader, u_time, &time, SHADER_UNIFORM_FLOAT);
				SetShaderValue(bg_shader, u_fogOrigin, &fogOrigin, SHADER_UNIFORM_VEC3);
				SetShaderValue(bg_shader, u_fogColor, &fogColor, SHADER_UNIFORM_VEC4);
				SetShaderValue(bg_shader, u_fogNear, &fogNear, SHADER_UNIFORM_FLOAT);
				SetShaderValue(bg_shader, u_fogFar, &fogFar, SHADER_UNIFORM_FLOAT);
				SetShaderValue(bg_shader, u_fogEnabled, &fogEnabled, SHADER_UNIFORM_INT);
			}

			BeginTextureMode(bg_surf);
			{
				ClearBackground(BLACK);

				BeginMode3D(bg_cam);
				{
					DrawModel(bg_model, {0.f, 0.f, 0.f}, 1.f, WHITE);

					if (debug_3d) {
						DrawGrid(16, 8.f);
					}
				}
				EndMode3D();
			}
			EndTextureMode();
		}

		BeginTextureMode(target);
		{
			ClearBackground(BLACK);

			if (bg_model.meshCount > 0 && (spell_bg_alpha < 1.f || spell_bg_tex.id == 0)) {
				DrawTextureRec(bg_surf.texture, {0.f, 0.f, float(bg_surf.texture.width), float(-bg_surf.texture.height)}, {0.f, 0.f}, WHITE);
			}

			if (spell_bg_alpha > 0.f && spell_bg_tex.id != 0) {
				Color tint = WHITE;
				tint.a = uint8_t(spell_bg_alpha * 255.f);
				float scale = float(PLAY_AREA_W) / float(spell_bg_tex.width);
				Rectangle src;
				src.x = 0.f;
				src.y = time * .2f;
				src.width = float(spell_bg_tex.width);
				src.height = float(spell_bg_tex.height) * scale;
				Rectangle dest = {0.f, 0.f, float(PLAY_AREA_W), float(PLAY_AREA_H)};
				DrawTexturePro(spell_bg_tex, src, dest, {0.f, 0.f}, 0.f, tint);
			}

			for (const Pickup& p : pickups) {
				Texture2D t;
				switch (p.type) {
					case PICKUP_POWER:		t = scene.texPickupPower;		break;
					case PICKUP_POINT:		t = scene.texPickupPoint;		break;
					case PICKUP_BIG_POWER:	t = scene.texPickupBigPower;	break;
					case PICKUP_BOMB:		t = scene.texPickupBomb;		break;
					case PICKUP_FULL_POWER:	t = scene.texPickupFullPower;	break;
					case PICKUP_1UP:		t = scene.texPickup1Up;			break;
					case PICKUP_SCORE:		t = scene.texPickupScore;		break;
				}
				DrawTextureCentered(t, floorf(p.x), floorf(p.y));
			}

			if (boss) {
				const Boss& b = *boss;
				float x = floorf(b.x);
				float y = floorf(b.y);
				if (b.sprite_id >= 0) {
					draw_sprite(loaded_sprites[b.sprite_id], b.frame_index, x, y, b.xscale, 1.f);
				} else {
					DrawRectangleV({x - 16.f, y - 32.f}, {32.f, 64.f}, MAROON);
				}
			}

			for (const Enemy& e : enemies) {
				float x = floorf(e.x);
				float y = floorf(e.y);
				if (e.sprite_id >= 0) {
					draw_sprite(loaded_sprites[e.sprite_id], e.frame_index, x, y);
				} else {
					DrawRectangleV({x - 16.f, y - 32.f}, {32.f, 64.f}, MAROON);
				}
			}

			for (const PlayerBullet& b : player_bullets) {
				float x = floorf(b.x);
				float y = floorf(b.y);
				Color c = {255, 255, 255, 100};
				switch (b.type) {
					case PLAYER_BULLET_REIMU_CARD: {
						DrawTextureCentered(scene.texReimuCard, x, y, 1.5f, 1.5f, b.reimu_card.rotation, c);
						break;
					}
					case PLAYER_BULLET_REIMU_ORB_SHOT: {
						DrawTextureCentered(scene.texReimuOrbShot, x, y, 1.5f, 1.5f, b.dir, c);
						break;
					}
				}
			}

			// player_draw()
			{
				Color player_col = WHITE;
				float player_xscale = 1.f;
				float player_yscale = 1.f;

				switch (player.state) {
					case PLAYER_STATE_NORMAL: {
						if (player.iframes > 0.f) {
							player_col.a = (int(time / 5.f) % 2) ? 255 : 128;
						}
						break;
					}
					case PLAYER_STATE_DYING: {
						player_col.a = uint8_t(std::lerp(255.f, 0.f, 1.f - player.timer / PLAYER_DEATH_TIME));
						break;
					}
					case PLAYER_STATE_APPEARING: {
						player_col.a = uint8_t(std::lerp(0.f, 255.f, 1.f - player.timer / PLAYER_DEATH_TIME));
						break;
					}
				}

				float x = floorf(player.x);
				float y = floorf(player.y);
				draw_sprite(player.spr, player.frame_index, x, y, player.xscale * player_xscale, player_yscale, 0.f, player_col);

				if (player.hitbox_alpha > 0.f) {
					Color c = {255, 255, 255, uint8_t(player.hitbox_alpha * 255.f)};
					DrawTextureCentered(scene.texHitbox, x, y, 1.f, 1.f, time, c);
				}
			}

			for (const Bullet& b : bullets) {
				float x = b.x;
				float y = b.y;
				switch (b.type) {
					case PROJECTILE_BULLET:
					case PROJECTILE_RECT: {
						if (b.texture_id >= 0) {
							Texture2D t = loaded_textures[b.texture_id];
							float rot = 0.f;
							if (b.rotate) rot = -b.dir;
							DrawTextureCentered(t, x, y, 1.f, 1.f, rot);
						}
						break;
					}
					case PROJECTILE_LAZER: {
						if (b.texture_id >= 0) {
							float t = b.length / b.spd;
							float l = b.length;
							if (b.lifetime < t) l = b.lifetime * b.spd;
							Texture2D tex = loaded_textures[b.texture_id];
							Rectangle src = {0.f, 0.f, float(tex.width), float(-tex.height)};
							Rectangle dest = {x, y, b.thickness + 2.f, l};
							Vector2 o = {(b.thickness + 2.f) / 2.f, 0.f};
							DrawTexturePro(tex, src, dest, o, -b.dir + 270.f, WHITE);
						}
						break;
					}
				}
			}

			if (boss) {
				{
					const char* text = TextFormat("%d", int(boss->phases.size() - boss->phase_index - 1));
					float s = float(game.font.baseSize);
					DrawTextEx(game.font, text, {0.f, 0.f}, s, 1.f, WHITE);
				}

				{
					const char* text = TextFormat("%d", int(boss->timer / 60.f));
					float s = float(game.font.baseSize);
					int w = int(MeasureTextEx(game.font, text, s, 1.f).x);
					DrawTextEx(game.font, text, {float(PLAY_AREA_W - w), 0.f}, s, 1.f, WHITE);
				}

				{
					int healthbar_x = 32 + 2;
					int healthbar_y = 4;
					int healthbar_w = PLAY_AREA_W - 64 - 4;
					int healthbar_h = 4;
					int w = int(float(healthbar_w) * boss->hp / boss->phases[boss->phase_index].hp);
					DrawRectangle(healthbar_x,     healthbar_y,     healthbar_w, healthbar_h,     BLACK);
					DrawRectangle(healthbar_x + 1, healthbar_y + 1, w - 2,       healthbar_h - 2, WHITE);
				}

				{
					Font f = game.font2;
					const char* text = boss->name.c_str();
					float x = 16.f;
					float y = 20.f;
					float s = float(f.baseSize);
					DrawTextEx(f, text, {x + 1.f, y + 1.f}, s, 1.f, BLACK);
					DrawTextEx(f, text, {x,       y      }, s, 1.f, {230, 230, 255, 255});
				}

				if (boss->phases[boss->phase_index].type == PHASE_SPELLCARD) {
					Font f = game.font2;
					const char* text = boss->phases[boss->phase_index].name.c_str();
					float s = float(f.baseSize);
					int w = int(MeasureTextEx(f, text, s, 1).x);
					float x = float(PLAY_AREA_W - 16 - w);
					float y = 20.f;
					DrawTextEx(f, text, {x + 1.f, y + 1.f}, s, 1.f, BLACK);
					DrawTextEx(f, text, {x,       y      }, s, 1.f, {230, 230, 255, 255});
				}
			}

			if (game.show_hitboxes) {
				if (boss) {
					DrawCircleV({boss->x, boss->y}, boss->radius, {255, 0, 0, 128});
				}
				for (const Enemy& e : enemies) {
					DrawCircleV({e.x, e.y}, e.radius, RED);
				}
				DrawCircleV({player.x, player.y}, game.characters[game.character_id].graze_radius, BLACK);
				DrawCircleV({player.x, player.y}, game.characters[game.character_id].radius, WHITE);
				for (const PlayerBullet& b : player_bullets) {
					DrawCircleV({b.x, b.y}, b.radius, {0, 0, 255, 128});
				}
				for (const Bullet& b : bullets) {
					switch (b.type) {
						case PROJECTILE_BULLET: {
							DrawCircleV({b.x, b.y}, b.radius, RED);
							break;
						}
						case PROJECTILE_LAZER: {
							float t = b.length / b.spd;
							float l = b.length;
							if (b.lifetime < t) l = b.lifetime * b.spd;
							Rectangle r = {b.x, b.y, b.thickness, l};
							Vector2 o = {b.thickness / 2.f, 0.f};
							DrawRectanglePro(r, o, -b.dir + 270.f, RED);
							break;
						}
						case PROJECTILE_RECT: {
							Rectangle r = {b.x, b.y, b.radius * 2.f, b.radius * 2.f};
							Vector2 o = {b.radius, b.radius};
							DrawRectanglePro(r, o, -b.dir, RED);
							break;
						}
					}
				}
				for (const Pickup& p : pickups) {
					DrawCircleV({p.x, p.y}, p.radius, {255, 255, 255, 128});
				}
			}
		}
		EndTextureMode();
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
