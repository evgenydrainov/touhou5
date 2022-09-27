#include "Game.h"

#define RAYLIBX_IMPLEMENTATION
#include "raylibx.h"

#include <fmt/format.h>
#include <iostream>

#include <inipp.h>
#include <fstream>

namespace th
{
	Game::Game()
	{
		std::cout << "[TOUHOU] Starting the game...\n";
		std::cout << "[TOUHOU] USING " LUA_RELEASE "\n";
		std::cout << "[TOUHOU] USING SOL " SOL_VERSION_STRING "\n";

		{
			inipp::Ini<char> ini;
			std::ifstream f("options.ini");
			ini.parse(f);
			inipp::get_value(ini.sections["Game"], "Starting Lives", options.starting_lives);
			inipp::get_value(ini.sections["Audio"], "Master Volume", options.master_volume);
		}

		InitWindow(GAME_W, GAME_H, "th");
		SetExitKey(0);
		set_window_mode(0);
		InitAudioDevice();
		SetMasterVolume(float(options.master_volume) / 10.0f);

		characters[0].name					= "Reimu Hakurei";
		characters[0].move_spd				= 3.75f;
		characters[0].focus_spd				= 1.6f;
		characters[0].radius				= 2.0f;
		characters[0].graze_radius			= 16.0f;
		characters[0].deathbomb_time		= 15.0f;
		characters[0].starting_bombs		= 2;
		characters[0].shot_type				= &Stage::reimu_shot_type;
		characters[0].bomb					= &Stage::reimu_bomb;
		characters[0].idle_spr.tex			= LoadTexture("reimuidle.png");
		characters[0].idle_spr.frame_count	= 6;
		characters[0].idle_spr.anim_spd		= 0.15f;
		characters[0].turn_spr.tex			= LoadTexture("reimuturn.png");
		characters[0].turn_spr.frame_count	= 9;
		characters[0].turn_spr.anim_spd		= 0.2f;
		characters[0].turn_spr.loop_frame	= 4;

		game_surf	= LoadStrippedRenderTexture(GAME_W, GAME_H);
		sndSelect	= LoadSound("se_select00.wav");
		sndOk		= LoadSound("se_ok00.wav");
		sndCancel	= LoadSound("se_cancel00.wav");

		SetSoundVolume(sndSelect, 0.75f);
		SetSoundVolume(sndOk, 0.75f);
		SetSoundVolume(sndCancel, 0.75f);

		{
			font.texture = LoadTexture("font.png");
			font.baseSize = 15;
			font.glyphCount = 98;
			font.glyphs = (GlyphInfo*)malloc(font.glyphCount * sizeof(GlyphInfo));
			if (font.glyphs) memset(font.glyphs, 0, font.glyphCount * sizeof(GlyphInfo));
			font.recs = (Rectangle*)malloc(font.glyphCount * sizeof(Rectangle));
			if (font.recs) memset(font.recs, 0, font.glyphCount * sizeof(Rectangle));
			for (int i = 0; i < font.glyphCount; i++) {
				int ch = 30 + i;
				if (font.glyphs) font.glyphs[i].value = ch;
				int ind = 14 + i;
				int w = font.texture.width / 16;
				int x = ind % w;
				int y = ind / w;
				if (font.recs) {
					font.recs[i].x = float(x) * 16.0f;
					font.recs[i].y = float(y) * 16.0f;
					font.recs[i].width = 15.0f;
					font.recs[i].height = 15.0f;
				}
			}
		}
	}

	Game::~Game()
	{
		UnloadFont(font);
		UnloadSound(sndCancel);
		UnloadSound(sndOk);
		UnloadSound(sndSelect);
		if (up_surf.id > 0) {
			UnloadRenderTexture(up_surf);
		}
		UnloadRenderTexture(game_surf);
		UnloadTexture(characters[0].idle_spr.tex);
		UnloadTexture(characters[0].turn_spr.tex);
		CloseAudioDevice();
		CloseWindow();
	}

	void Game::run()
	{
		while (!WindowShouldClose() && !quit) {
			float delta = std::min(GetFrameTime(), 1.0f / 30.0f) * 60.0f;

			if (IsKeyPressed(KEY_F4)) {
				set_window_mode(window_mode + 1);
			}

			skip_frame = frame_advance;

			key_pressed = GetKeyPressed();
			char_pressed = GetCharPressed();

			//std::cout << key_pressed << '\n';
			//std::cout << char_pressed << '\n';

#ifdef TH_DEBUG
			show_hitboxes	^= IsKeyPressed(KEY_F1);
			god_mode		^= IsKeyPressed(KEY_I);
			debug_overlay	^= IsKeyPressed(KEY_F3);
			if (IsKeyPressed(KEY_F2)) {
				next_scene = TITLE_SCENE;
			}
			if (IsKeyDown(KEY_F)) {
				delta *= 5.0f;
			}
			if (IsKeyPressed(KEY_F5)) {
				frame_advance = true;
				skip_frame = false;
			}
			if (IsKeyPressed(KEY_F6)) {
				frame_advance = false;
			}
#endif

			if (!skip_frame) {
				update(delta);
			}

			draw(delta);

			time += delta;

			//std::cout << GetScreenWidth() << '\n';
			//std::cout << GetScreenHeight() << '\n';
			//
			//if (IsKeyPressed(KEY_SPACE)) {
			//	ToggleFullscreen();
			//}

			//std::cout << time << " active\n";
		}
	}

	void Game::update(float delta)
	{
		if (next_scene > 0) {
			int s = next_scene;
			next_scene = 0;
			switch (s) {
				case TITLE_SCENE: {
					scene.emplace<TITLE_SCENE>(*this);
					break;
				}
				case SCRIPT_SELECT_SCENE: {
					scene.emplace<SCRIPT_SELECT_SCENE>(*this);
					break;
				}
				case ERROR_SCENE: {
					scene.emplace<ERROR_SCENE>(*this);
					break;
				}
				case GAME_SCENE: {
					scene.emplace<GAME_SCENE>(*this);
					break;
				}
				case OPTIONS_SCENE: {
					scene.emplace<OPTIONS_SCENE>(*this);
					break;
				}
			}
		}

		switch (scene.index()) {
			case TITLE_SCENE: {
				std::get<TITLE_SCENE>(scene).update(delta);
				break;
			}
			case SCRIPT_SELECT_SCENE: {
				std::get<SCRIPT_SELECT_SCENE>(scene).update(delta);
				break;
			}
			case ERROR_SCENE: {
				std::get<ERROR_SCENE>(scene).update(delta);
				break;
			}
			case GAME_SCENE: {
				std::get<GAME_SCENE>(scene).update(delta);
				break;
			}
			case OPTIONS_SCENE: {
				std::get<OPTIONS_SCENE>(scene).update(delta);
				break;
			}
		}
	}

	void Game::draw(float delta)
	{
		switch (scene.index()) {
			case TITLE_SCENE: {
				std::get<TITLE_SCENE>(scene).draw(game_surf, delta);
				break;
			}
			case SCRIPT_SELECT_SCENE: {
				std::get<SCRIPT_SELECT_SCENE>(scene).draw(game_surf, delta);
				break;
			}
			case ERROR_SCENE: {
				std::get<ERROR_SCENE>(scene).draw(game_surf, delta);
				break;
			}
			case GAME_SCENE: {
				std::get<GAME_SCENE>(scene).draw(game_surf, delta);
				break;
			}
			case OPTIONS_SCENE: {
				std::get<OPTIONS_SCENE>(scene).draw(game_surf, delta);
				break;
			}
		}

		BeginTextureMode(game_surf);
		{
			const char* text = TextFormat("%dfps", GetFPS());
			Vector2 s = MeasureTextEx(font, text, font.baseSize, 0);
			DrawTextEx(font, text, {GAME_W - s.x, GAME_H - s.y}, font.baseSize, 0, WHITE);
		}
		EndTextureMode();

		if (IsWindowFullscreen()) {
			BeginTextureMode(up_surf);
			{
				ClearBackground(BLACK);

				DrawTexturePro(
					game_surf.texture,
					{0, 0, float(game_surf.texture.width), -float(game_surf.texture.height)},
					{0, 0, float(up_surf.texture.width), float(up_surf.texture.height)},
					{0, 0},
					0,
					WHITE
				);
			}
			EndTextureMode();
		}

		BeginDrawing();
		{
			ClearBackground(BLACK);

			float screen_w = GetScreenWidth();
			float screen_h = GetScreenHeight();
			if (IsWindowFullscreen()) {
				float up_surf_w = up_surf.texture.width;
				float up_surf_h = up_surf.texture.height;
				float scale = std::min(screen_w / up_surf_w, screen_h / up_surf_h);
				float final_w = up_surf_w * scale;
				float final_h = up_surf_h * scale;
				DrawTexturePro(
					up_surf.texture,
					{0, 0, up_surf_w, -up_surf_h},
					{screen_w / 2.0f, screen_h / 2.0f, final_w, final_h},
					{final_w / 2.0f, final_h / 2.0f},
					0,
					WHITE
				);
			} else {
				DrawTexturePro(
					game_surf.texture,
					{0, 0, float(game_surf.texture.width), -float(game_surf.texture.height)},
					{0, 0, screen_w, screen_h},
					{0, 0},
					0,
					WHITE
				);
			}

			if (debug_overlay) {
				Font f = GetFontDefault();
				float size = 20;
				float spacing = 2;

				const char* text = TextFormat(
					"monitor %d\n"
					"fullscreen %d\n"
					"vsync %d\n"
					"delta %f\n"
					"time %f\n"
					"show hitboxes %d\n"
					"god mode %d",
					GetCurrentMonitor(),
					IsWindowFullscreen(),
					IsWindowState(FLAG_VSYNC_HINT),
					delta,
					time,
					show_hitboxes,
					god_mode
				);
				int h = MeasureTextEx(f, text, size, spacing).y;
				DrawTextEx(f, text, {0, 0}, size, spacing, RAYWHITE);

				if (scene.index() == GAME_SCENE) {
					const GameScene& s = std::get<GAME_SCENE>(scene);
					const char* text = TextFormat(
						"bullets %d\n"
						"player bullets %d\n"
						"enemies %d\n"
						"pickups %d\n"
						"coroutine calls %d\n"
						"physics calls %d\n"
						"player dps %f",
						int(s.stage->bullets.size()),
						int(s.stage->player_bullets.size()),
						int(s.stage->enemies.size()),
						int(s.stage->pickups.size()),
						s.stage->coroutine_calls,
						s.stage->physics_calls,
						s.stage->player_dps
					);
					DrawTextEx(f, text, {0, h + 30.0f}, size, spacing, RAYWHITE);
				}
			}
		}
		EndDrawing();
	}

	void Game::set_window_mode(int mode)
	{
		int m = GetCurrentMonitor();
		int monitor_w = GetMonitorWidth(m);
		int monitor_h = GetMonitorHeight(m);
		int max_scale = std::min(monitor_w / GAME_W, monitor_h / GAME_H);
		bool barely = (GAME_W * max_scale) == monitor_w || (GAME_H * max_scale) == monitor_h;
		int last_mode = max_scale - barely;
		window_mode = mode;
		window_mode %= last_mode + 1;
		if (up_surf.id > 0) {
			UnloadRenderTexture(up_surf);
			up_surf.id = 0;
		}
		if (window_mode == last_mode) {
			ClearWindowState(FLAG_VSYNC_HINT);
			SetTargetFPS(GetMonitorRefreshRate(m) * 2);

			SetWindowState(FLAG_FULLSCREEN_MODE);

			up_surf = LoadStrippedRenderTexture(GAME_W * max_scale, GAME_H * max_scale);
			SetTextureFilter(up_surf.texture, TEXTURE_FILTER_BILINEAR);
			SetTextureWrap(up_surf.texture, TEXTURE_WRAP_CLAMP);

			DisableCursor();
		} else {
			int window_w = GAME_W * (window_mode + 1);
			int window_h = GAME_H * (window_mode + 1);
			int window_x = GetMonitorPosition(m).x;
			int window_y = GetMonitorPosition(m).y;
			window_x += monitor_w / 2;
			window_y += monitor_h / 2;
			window_x -= window_w / 2;
			window_y -= window_h / 2;

			SetWindowMonitor(-1, window_x, window_y, window_w, window_h, -1);

			SetWindowState(FLAG_VSYNC_HINT);
			SetTargetFPS(0);

			EnableCursor();
		}
	}
}
