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

		play_area		= LoadStrippedRenderTexture(PLAY_AREA_W, PLAY_AREA_H);
		texReimu		= LoadTexture("reimu.png");
		texReimuCard	= LoadTexture("reimucard.png");
		texBg			= LoadTexture("bg.png");
		texHitbox		= LoadTexture("hitbox.png");
		sndGraze		= LoadSound("se_graze.wav");
		sndPause		= LoadSound("se_pause.wav");
		sndReimuShoot	= LoadSound("se_plst00.wav");
		sndEnemyHit		= LoadSound("se_damage00.wav");
		sndPichuun		= LoadSound("se_pldead00.wav");
		sndEnemyShoot	= LoadSound("se_tan00.wav");
		sndPowerUp		= LoadSound("se_powerup.wav");
		snd1Up			= LoadSound("se_extend.wav");

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
		UnloadTexture(texHitbox);
		UnloadTexture(texBg);
		UnloadTexture(texReimuCard);
		UnloadTexture(texReimu);
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

					DrawTextureRec(play_area.texture, {0.0f, 0.0f, float(PLAY_AREA_W), -float(PLAY_AREA_H)}, {0.0f, 0.0f}, WHITE);

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
				DrawTextureRec(play_area.texture, {0.0f, 0.0f, float(PLAY_AREA_W), -float(PLAY_AREA_H)}, {float(PLAY_AREA_X), float(PLAY_AREA_Y)}, WHITE);
			} else {
				DrawTextureRec(pause_surf.texture, {0.0f, 0.0f, float(PLAY_AREA_W), -float(PLAY_AREA_H)}, {float(PLAY_AREA_X), float(PLAY_AREA_Y)}, WHITE);
			}

			DrawText(TextFormat("HiScore %d", hiscore), PLAY_AREA_X + PLAY_AREA_W + 16, PLAY_AREA_Y + 32 + 16 * 0, 10, WHITE);
			DrawText(TextFormat("Score %d", stats.score), PLAY_AREA_X + PLAY_AREA_W + 16, PLAY_AREA_Y + 32 + 16 * 1, 10, WHITE);
			DrawText(TextFormat("Player %d", stats.lives), PLAY_AREA_X + PLAY_AREA_W + 16, PLAY_AREA_Y + 32 + 16 * 3, 10, WHITE);
			DrawText(TextFormat("Bomb %d", stats.bombs), PLAY_AREA_X + PLAY_AREA_W + 16, PLAY_AREA_Y + 32 + 16 * 4, 10, WHITE);
			if (stats.power >= 128) {
				DrawText("Power MAX", PLAY_AREA_X + PLAY_AREA_W + 16, PLAY_AREA_Y + 32 + 16 * 6, 10, WHITE);
			} else {
				DrawText(TextFormat("Power %d", stats.power), PLAY_AREA_X + PLAY_AREA_W + 16, PLAY_AREA_Y + 32 + 16 * 6, 10, WHITE);
			}
			DrawText(TextFormat("Graze %d", stats.graze), PLAY_AREA_X + PLAY_AREA_W + 16, PLAY_AREA_Y + 32 + 16 * 7, 10, WHITE);
			DrawText(TextFormat("Point %d", stats.points), PLAY_AREA_X + PLAY_AREA_W + 16, PLAY_AREA_Y + 32 + 16 * 8, 10, WHITE);
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
