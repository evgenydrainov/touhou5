#include "GameScene.h"

#include "Game.h"

#include "raylibx.h"
#include "cpml.h"
#include <fmt/format.h>

namespace th
{
	GameScene::GameScene(Game& game) : game(game)
	{
		continues = 3;
		reset_stats();

		play_area			= LoadStrippedRenderTexture(PLAY_AREA_W, PLAY_AREA_H);
		texReimuCard		= LoadTexture("reimucard.png");
		texReimuOrb			= LoadTexture("reimuorb.png");
		texReimuOrbShot		= LoadTexture("reimuorbshot.png");
		texBg				= LoadTexture("bg.png");
		texHitbox			= LoadTexture("hitbox.png");
		texSidebar			= LoadTexture("sidebar.png");
		texUiLife			= LoadTexture("uilife.png");
		texUiBomb			= LoadTexture("uibomb.png");
		texEnemyLabel		= LoadTexture("enemylabel.png");
		texPickupPower		= LoadTexture("pickuppower.png");
		texPickupPoint		= LoadTexture("pickuppoint.png");
		texPickupBigPower	= LoadTexture("pickupbigpower.png");
		texPickupBomb		= LoadTexture("pickupbomb.png");
		texPickupFullPower	= LoadTexture("pickupfullpower.png");
		texPickup1Up		= LoadTexture("pickup1up.png");
		texPickupScore		= LoadTexture("pickupscore.png");
		sndGraze			= LoadSound("se_graze.wav");
		sndPause			= LoadSound("se_pause.wav");
		sndReimuShoot		= LoadSound("se_plst00.wav");
		sndEnemyHit			= LoadSound("se_damage00.wav");
		sndPichuun			= LoadSound("se_pldead00.wav");
		sndEnemyShoot		= LoadSound("se_tan00.wav");
		sndPowerUp			= LoadSound("se_powerup.wav");
		snd1Up				= LoadSound("se_extend.wav");
		sndPickup			= LoadSound("se_item00.wav");
		sndLazer			= LoadSound("se_lazer00.wav");
		sndEnemyDie			= LoadSound("se_enep00.wav");
		sndBossDie			= LoadSound("se_enep01.wav");
		sndSpellCard		= LoadSound("se_cat00.wav");

		SetTextureFilter(texHitbox, TEXTURE_FILTER_BILINEAR);

		SetSoundVolume(sndEnemyShoot, 0.25f);
		SetSoundVolume(sndEnemyHit, 0.5f);
		SetSoundVolume(sndPichuun, 0.75f);
		SetSoundVolume(sndPickup, 0.75f);
		SetSoundVolume(sndLazer, 0.75f);
		SetSoundVolume(sndEnemyDie, 0.75f);
		SetSoundVolume(sndBossDie, 0.75f);
		SetSoundVolume(sndSpellCard, 0.75f);

		stage.emplace(game, *this);
	}

	GameScene::~GameScene()
	{
		if (music.stream.buffer) {
			UnloadMusicStream(music);
		}
		UnloadSound(sndSpellCard);
		UnloadSound(sndBossDie);
		UnloadSound(sndEnemyDie);
		UnloadSound(sndLazer);
		UnloadSound(sndPickup);
		UnloadSound(snd1Up);
		UnloadSound(sndPowerUp);
		UnloadSound(sndEnemyShoot);
		UnloadSound(sndPichuun);
		UnloadSound(sndEnemyHit);
		UnloadSound(sndReimuShoot);
		UnloadSound(sndPause);
		UnloadSound(sndGraze);
		UnloadTexture(texPickupScore);
		UnloadTexture(texPickup1Up);
		UnloadTexture(texPickupFullPower);
		UnloadTexture(texPickupBomb);
		UnloadTexture(texPickupBigPower);
		UnloadTexture(texPickupPoint);
		UnloadTexture(texPickupPower);
		UnloadTexture(texEnemyLabel);
		UnloadTexture(texUiBomb);
		UnloadTexture(texUiLife);
		UnloadTexture(texSidebar);
		UnloadTexture(texHitbox);
		UnloadTexture(texBg);
		UnloadTexture(texReimuOrbShot);
		UnloadTexture(texReimuOrb);
		UnloadTexture(texReimuCard);
		if (pause_surf.id > 0) {
			UnloadRenderTexture(pause_surf);
		}
		UnloadRenderTexture(play_area);
	}

	void GameScene::update(float delta)
	{
#ifdef TH_DEBUG
		if (IsKeyDown(KEY_LEFT_SHIFT)) {
			if (game.key_pressed == KEY_L)		get_lives(8);
			if (game.key_pressed == KEY_B)		get_bombs(8);
			if (game.key_pressed == KEY_P)		get_power(128);
			if (game.key_pressed == KEY_PERIOD)	get_points(50);
			if (game.key_pressed == KEY_G)		stats.graze += 50;
		} else if (IsKeyDown(KEY_LEFT_ALT)) {
			if (game.key_pressed == KEY_L)								stage->player_get_hit();
			if (game.key_pressed == KEY_B		&& stats.bombs > 0)		stats.bombs--;
			if (game.key_pressed == KEY_P		&& stats.power > 0)		stats.power--;
			if (game.key_pressed == KEY_PERIOD	&& stats.points > 0)	stats.points--;
			if (game.key_pressed == KEY_G		&& stats.graze > 0)		stats.graze--;
		} else {
			if (game.key_pressed == KEY_L)		get_lives(1);
			if (game.key_pressed == KEY_B)		get_bombs(1);
			if (game.key_pressed == KEY_P)		get_power(1);
			if (game.key_pressed == KEY_PERIOD)	get_points(1);
			if (game.key_pressed == KEY_G)		stats.graze++;
		}

		if (IsKeyPressed(KEY_R)) game.next_scene = GAME_SCENE;
#endif

		if (state == State::Playing) {
			if (IsKeyPressed(KEY_ESCAPE)) {
				pause();
			}
		} else if (state == State::Paused) {
			if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_X)) {
				if (!menu_exiting) {
					menu_cursor = 0;
					menu_exiting = true;
					//StopSound(sndPause);
					PlaySound(game.sndCancel);
				}
			}
		}

		switch (state) {
			case State::Playing: {
				stage->update(delta);
				break;
			}
			case State::Paused:
			case State::Lost: {
				if (menu_exiting) {
					if (menu_anim == 1.0f) {
						if (state == State::Paused) {
							switch (menu_cursor) {
								case 0: resume(); break;
								case 1: quit_to_title_screen(); break;
							}
						} else if (state == State::Lost) {
							switch (menu_cursor) {
								case 0: use_continue(); break;
								case 1: quit_to_title_screen(); break;
							}
						}
					}

					menu_anim = cpml::approach(menu_anim, 1.0f, 1.0f / 10.0f * delta);
				} else {
					menu_cursor += (game.key_pressed == KEY_DOWN) - (game.key_pressed == KEY_UP);
					menu_cursor = cpml::emod(menu_cursor, menu_labels.size());
					if ((game.key_pressed == KEY_DOWN) - (game.key_pressed == KEY_UP)) {
						PlaySound(game.sndSelect);
					}

					if (IsKeyPressed(KEY_Z)) {
						menu_exiting = true;
						//StopSound(sndPause);

						if (menu_cursor == menu_labels.size() - 1) {
							PlaySound(game.sndCancel);
						} else {
							PlaySound(game.sndOk);
						}
					}

					menu_anim = cpml::approach(menu_anim, 0.0f, 1.0f / 10.0f * delta);
				}
				break;
			}
			case State::Won: {
				if (IsKeyPressed(KEY_Z)) {
					quit_to_title_screen();
					PlaySound(game.sndOk);
				}
				break;
			}
		}

		if (music.stream.buffer) {
			UpdateMusicStream(music);
		}
	}

	void GameScene::get_power(int power)
	{
		while (power--) {
			if (stats.power < 128) {
				stats.power++;
				switch (stats.power) {
					case 8:
					case 16:
					case 32:
					case 48:
					case 64:
					case 80:
					case 96:
					case 128: PlaySound(sndPowerUp); break;
				}
			}
		}
	}

	void GameScene::get_lives(int lives)
	{
		while (lives--) {
			if (stats.lives < 8) {
				stats.lives++;
				PlaySound(snd1Up);
			} else {
				get_bombs(1);
			}
		}
	}

	void GameScene::get_bombs(int bombs)
	{
		while (bombs--) {
			if (stats.bombs < 8) {
				stats.bombs++;
			}
		}
	}

	void GameScene::get_points(int points)
	{
		while (points--) {
			stats.points++;
			if (stats.points >= 800) {
				if (stats.points % 200 == 0) {
					get_lives(1);
				}
			} else {
				switch (stats.points) {
					case 50:
					case 125:
					case 200:
					case 300:
					case 450: get_lives(1); break;
				}
			}
		}
	}

	void GameScene::get_score(int score)
	{
		stats.score += score;
		hiscore = std::max(hiscore, stats.score);
	}

	void GameScene::game_over()
	{
		state = State::Lost;
		menu_labels.emplace_back(fmt::format("Continue? ({})", continues));
		menu_labels.emplace_back("Quit to Title Screen");
		menu_cursor = 0;
		menu_anim = 1.0f;
		menu_exiting = false;
		pause_surf = LoadStrippedRenderTexture(PLAY_AREA_W, PLAY_AREA_H);
	}

	void GameScene::win()
	{
		state = State::Won;
		get_score((1000 + stats.power * 100 + stats.graze * 10) * stats.points);
		menu_anim = 1.0f;
		menu_exiting = false;
		pause_surf = LoadStrippedRenderTexture(PLAY_AREA_W, PLAY_AREA_H);
	}

	void GameScene::reset_stats()
	{
		stats = {};
		// stats.lives is "extra lives" and starting_lives is "all lives"
		// @TODO: Change?
		stats.lives = game.options.starting_lives - 1;
		stats.bombs = game.characters[game.character_id].starting_bombs;
	}

	void GameScene::pause()
	{
		state = State::Paused;
		menu_labels.emplace_back("Resume");
		menu_labels.emplace_back("Quit to Title Screen");
		menu_cursor = 0;
		menu_anim = 1.0f;
		menu_exiting = false;
		pause_surf = LoadStrippedRenderTexture(PLAY_AREA_W, PLAY_AREA_H);
		PlaySound(sndPause);
	}

	void GameScene::resume()
	{
		state = State::Playing;
		menu_labels.clear();
		UnloadRenderTexture(pause_surf);
		pause_surf.id = 0;
	}

	void GameScene::use_continue()
	{
		if (continues > 0) {
			reset_stats();
			resume();
			continues--;
		}
	}

	void GameScene::quit_to_title_screen()
	{
		game.next_scene = TITLE_SCENE;
	}

	void GameScene::play_music(const std::string& fname)
	{
		if (music.stream.buffer) {
			UnloadMusicStream(music);
		}
		music = LoadMusicStream((game.script_path / fname).string().c_str());
		SetMusicVolume(music, float(game.options.music_volume) / 10.0f);
		PlayMusicStream(music);
	}

	void GameScene::draw(RenderTexture2D target, float delta)
	{
		switch (state) {
			case State::Playing: {
				stage->draw(play_area, delta);
				break;
			}
			case State::Paused:
			case State::Lost: {
				BeginTextureMode(pause_surf);
				{
					ClearBackground(BLACK);

					DrawTextureRec(play_area.texture, {0, 0, PLAY_AREA_W, -PLAY_AREA_H}, {0, 0}, WHITE);

					DrawRectangle(0, 0, PLAY_AREA_W, PLAY_AREA_H, {0, 0, 0, 128});

					for (int i = 0; i < menu_labels.size(); i++) {
						Font f = game.font2;
						const char* text = menu_labels[i].c_str();
						float x = (PLAY_AREA_W - MeasureTextEx(f, text, f.baseSize, 1).x) / 2;
						float y = PLAY_AREA_H / 3 + i * f.baseSize;

						float off = menu_anim * 30.0f;
						if (i % 2 == 1) off = -off;
						x += off;

						x = std::floor(x);
						y = std::floor(y);

						DrawTextEx(f, text, {x + 1.0f, y + 1.0f}, f.baseSize, 1, BLACK);
						DrawTextEx(f, text, {x, y}, f.baseSize, 1, (i == menu_cursor) ? YELLOW : WHITE);
					}
				}
				EndTextureMode();
				break;
			}
			case State::Won: {
				BeginTextureMode(pause_surf);
				{
					ClearBackground(BLACK);

					DrawTextureRec(play_area.texture, {0, 0, PLAY_AREA_W, -PLAY_AREA_H}, {0, 0}, WHITE);

					DrawRectangle(0, 0, PLAY_AREA_W, PLAY_AREA_H, {0, 0, 0, 128});

					{
						Font f = game.font;
						const char* text = TextFormat(
							"Stage Clear\n"
							"\n"
							"Stage * 1000 = %5d\n"
							"Power *  100 = %5d\n"
							"Graze *   10 = %5d\n"
							"    * Point Item %3d\n"
							"\n"
							"Total     = %8d\n"
							"\n"
							"Press Z to Go Back",
							1000,
							stats.power * 100,
							stats.graze * 10,
							stats.points,
							(1000 + stats.power * 100 + stats.graze * 10) * stats.points
						);
						DrawTextEx(f, text, {32, 64}, f.baseSize, 1, WHITE);
					}
				}
				EndTextureMode();
				break;
			}
		}

		BeginTextureMode(target);
		{
			ClearBackground(BLACK);

			DrawTexture(texBg, 0, 0, WHITE);

			if (state == State::Playing) {
				DrawTextureRec(play_area.texture, {0, 0, PLAY_AREA_W, -PLAY_AREA_H}, {PLAY_AREA_X, PLAY_AREA_Y}, WHITE);
			} else {
				DrawTextureRec(pause_surf.texture, {0, 0, PLAY_AREA_W, -PLAY_AREA_H}, {PLAY_AREA_X, PLAY_AREA_Y}, WHITE);
			}

			DrawTexture(texSidebar, PLAY_AREA_X + PLAY_AREA_W + 16, PLAY_AREA_Y + 32, WHITE);

			DrawTextEx(game.font, TextFormat("%09d", hiscore), {PLAY_AREA_X + PLAY_AREA_W + 80, PLAY_AREA_Y + 32 + 16 * 0}, game.font.baseSize, 0, WHITE);
			DrawTextEx(game.font, TextFormat("%09d", stats.score), {PLAY_AREA_X + PLAY_AREA_W + 80, PLAY_AREA_Y + 32 + 16 * 1}, game.font.baseSize, 0, WHITE);
			for (int i = 0; i < stats.lives; i++) {
				DrawTexture(texUiLife, PLAY_AREA_X + PLAY_AREA_W + 80 + 16 * i, PLAY_AREA_Y + 32 + 16 * 3, WHITE);
			}
			for (int i = 0; i < stats.bombs; i++) {
				DrawTexture(texUiBomb, PLAY_AREA_X + PLAY_AREA_W + 80 + 16 * i, PLAY_AREA_Y + 32 + 16 * 4, WHITE);
			}
			DrawRectangle(PLAY_AREA_X + PLAY_AREA_W + 80, PLAY_AREA_Y + 32 + 16 * 6, 135 * stats.power / 128, 16, LIGHTGRAY);
			if (stats.power >= 128) {
				DrawTextEx(game.font, "MAX", {PLAY_AREA_X + PLAY_AREA_W + 80, PLAY_AREA_Y + 32 + 16 * 6}, game.font.baseSize, 0, WHITE);
			} else {
				DrawTextEx(game.font, TextFormat("%d", stats.power), {PLAY_AREA_X + PLAY_AREA_W + 80, PLAY_AREA_Y + 32 + 16 * 6}, game.font.baseSize, 0, WHITE);
			}
			DrawTextEx(game.font, TextFormat("%d", stats.graze), {PLAY_AREA_X + PLAY_AREA_W + 80, PLAY_AREA_Y + 32 + 16 * 7}, game.font.baseSize, 0, WHITE);
			{
				int next_point_level;
				if (stats.points >= 800) {
					next_point_level = 800 + ((stats.points - 800) / 200 + 1) * 200;
				} else if (stats.points >= 450) {
					next_point_level = 800;
				} else if (stats.points >= 300) {
					next_point_level = 450;
				} else if (stats.points >= 200) {
					next_point_level = 300;
				} else if (stats.points >= 125) {
					next_point_level = 200;
				} else if (stats.points >= 50) {
					next_point_level = 125;
				} else {
					next_point_level = 50;
				}
				DrawTextEx(game.font, TextFormat("%d/%d", stats.points, next_point_level), {PLAY_AREA_X + PLAY_AREA_W + 80, PLAY_AREA_Y + 32 + 16 * 8}, game.font.baseSize, 0, WHITE);
			}

			if (stage->boss) {
				int x = std::clamp(int(stage->boss->x), 0, PLAY_AREA_W);
				DrawTexture(texEnemyLabel, PLAY_AREA_X + x - texEnemyLabel.width / 2, PLAY_AREA_Y + PLAY_AREA_H, WHITE);
			}
		}
		EndTextureMode();
	}
}
