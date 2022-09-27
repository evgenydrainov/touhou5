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
		texPickupPoint		= LoadTexture("pickuppower.png");
		texPickupBigPower	= LoadTexture("pickuppower.png");
		texPickupBomb		= LoadTexture("pickuppower.png");
		texPickupFullPower	= LoadTexture("pickuppower.png");
		texPickup1Up		= LoadTexture("pickuppower.png");
		texPickupScore		= LoadTexture("pickuppower.png");
		sndGraze			= LoadSound("se_graze.wav");
		sndPause			= LoadSound("se_pause.wav");
		sndReimuShoot		= LoadSound("se_plst00.wav");
		sndEnemyHit			= LoadSound("se_damage00.wav");
		sndPichuun			= LoadSound("se_pldead00.wav");
		sndEnemyShoot		= LoadSound("se_tan00.wav");
		sndPowerUp			= LoadSound("se_powerup.wav");
		snd1Up				= LoadSound("se_extend.wav");

		SetSoundVolume(sndEnemyShoot, 0.25f);
		SetSoundVolume(sndEnemyHit, 0.5f);
		SetSoundVolume(sndPichuun, 0.75f);

		stage.emplace(game, *this);
	}

	GameScene::~GameScene()
	{
		if (music.stream.buffer) {
			UnloadMusicStream(music);
		}
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
		if (IsKeyPressed(KEY_ESCAPE)) {
			if (state == State::Playing) {
				pause();
			} else if (state == State::Paused) {
				resume();
				PlaySound(game.sndCancel);
			}
		}

		switch (state) {
			case State::Playing: {
				stage->update(delta);
				break;
			}
			case State::Paused: {
				menu_cursor += IsKeyPressed(KEY_DOWN) - IsKeyPressed(KEY_UP);
				menu_cursor = cpml::emod(menu_cursor, menu_labels.size());
				if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP)) {
					PlaySound(game.sndSelect);
				}
				if (IsKeyPressed(KEY_Z)) {
					switch (menu_cursor) {
						case 0: resume(); break;
						case 1: quit_to_title_screen(); break;
					}
					if (menu_cursor < 1) {
						PlaySound(game.sndOk);
					} else {
						PlaySound(game.sndCancel);
					}
				}
				break;
			}
			case State::Lost: {
				menu_cursor += IsKeyPressed(KEY_DOWN) - IsKeyPressed(KEY_UP);
				menu_cursor = cpml::emod(menu_cursor, menu_labels.size());
				if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP)) {
					PlaySound(game.sndSelect);
				}
				if (IsKeyPressed(KEY_Z)) {
					switch (menu_cursor) {
						case 0: use_continue(); break;
						case 1: quit_to_title_screen(); break;
					}
				}
				break;
			}
		}

		if (music.stream.buffer) {
			UpdateMusicStream(music);
		}

#ifdef TH_DEBUG
		if (IsKeyDown(KEY_LEFT_SHIFT)) {
			if (IsKeyPressed(KEY_L))		stage->get_lives(8);
			if (IsKeyPressed(KEY_B))		stage->get_bombs(8);
			if (IsKeyPressed(KEY_P))		stage->get_power(128);
			if (IsKeyPressed(KEY_PERIOD))	stage->get_points(50);
			if (IsKeyPressed(KEY_G))		stats.graze += 50;
		} else if (IsKeyDown(KEY_LEFT_CONTROL)) {
			if (IsKeyPressed(KEY_P))		stage->get_power(1);
			if (IsKeyPressed(KEY_PERIOD))	stage->get_points(1);
			if (IsKeyPressed(KEY_G))		stats.graze++;
		} else if (IsKeyDown(KEY_LEFT_ALT)) {
			if (IsKeyPressed(KEY_L))								stage->player_get_hit();
			if (IsKeyPressed(KEY_B)			&& stats.bombs > 0)		stats.bombs--;
			if (IsKeyPressed(KEY_P)			&& stats.power > 0)		stats.power--;
			if (IsKeyPressed(KEY_PERIOD)	&& stats.points > 0)	stats.points--;
			if (IsKeyPressed(KEY_G)			&& stats.graze > 0)		stats.graze--;
		} else {
			if (IsKeyPressed(KEY_L))		stage->get_lives(1);
			if (IsKeyPressed(KEY_B))		stage->get_bombs(1);
			if (IsKeyPressed(KEY_P))		stage->get_power(8);
			if (IsKeyPressed(KEY_PERIOD))	stage->get_points(10);
			if (IsKeyPressed(KEY_G))		stats.graze += 10;
		}
#endif
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
						const char* text = menu_labels[i].c_str();
						DrawText(text, (PLAY_AREA_W - MeasureText(text, 10)) / 2, PLAY_AREA_H / 3 + i * 10, 10, (i == menu_cursor) ? YELLOW : WHITE);
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
		pause_surf = LoadStrippedRenderTexture(PLAY_AREA_W, PLAY_AREA_H);
		PlaySound(sndPause);
	}

	void GameScene::resume()
	{
		state = State::Playing;
		menu_labels.clear();
		UnloadRenderTexture(pause_surf);
		pause_surf.id = 0;
		StopSound(sndPause);
	}

	void GameScene::game_over()
	{
		state = State::Lost;
		menu_labels.emplace_back(fmt::format("Continue? ({})", continues));
		menu_labels.emplace_back("Quit to Title Screen");
		menu_cursor = 0;
		pause_surf = LoadStrippedRenderTexture(PLAY_AREA_W, PLAY_AREA_H);
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
		PlayMusicStream(music);
	}
}
