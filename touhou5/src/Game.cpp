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

		characters[0].name				= "Reimu Hakurei";
		characters[0].move_spd			= 3.75f;
		characters[0].focus_spd			= 1.75f;
		characters[0].radius			= 2.0f;
		characters[0].graze_radius		= 16.0f;
		characters[0].deathbomb_time	= 15.0f;
		characters[0].starting_bombs	= 2;
		characters[0].shot_type			= &Stage::reimu_shot_type;
		characters[0].bomb				= &Stage::reimu_bomb;

		{
			inipp::Ini<char> ini;
			std::ifstream f("options.ini");
			ini.parse(f);
			inipp::get_value(ini.sections["Game"], "Starting Lives", options.starting_lives);
			inipp::get_value(ini.sections["Audio"], "Master Volume", options.master_volume);
		}

		InitWindow(GAME_W, GAME_H, "th");
		InitAudioDevice();
		SetExitKey(0);
		set_window_mode(0);
		set_master_volume(options.master_volume);

		game_surf = LoadStrippedRenderTexture(GAME_W, GAME_H);
		sndSelect = LoadSound("se_select00.wav");
		sndOk = LoadSound("se_ok00.wav");
		sndCancel = LoadSound("se_cancel00.wav");
	}

	Game::~Game()
	{
		UnloadSound(sndCancel);
		UnloadSound(sndOk);
		UnloadSound(sndSelect);
		if (up_surf.id > 0) {
			UnloadRenderTexture(up_surf);
		}
		UnloadRenderTexture(game_surf);
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

#ifdef TH_DEBUG
			show_hitboxes	^= IsKeyPressed(KEY_F1);
			god_mode		^= IsKeyPressed(KEY_G);
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
			DrawText(text, GAME_W - MeasureText(text, 10), GAME_H - 10, 10, WHITE);
		}
		EndTextureMode();

		if (IsWindowFullscreen()) {
			BeginTextureMode(up_surf);
			{
				ClearBackground(BLACK);

				DrawTexturePro(
					game_surf.texture,
					{0.0f, 0.0f, float(game_surf.texture.width), -float(game_surf.texture.height)},
					{0.0f, 0.0f, float(up_surf.texture.width), float(up_surf.texture.height)},
					{0.0f, 0.0f},
					0.0f,
					WHITE
				);
			}
			EndTextureMode();
		}

		BeginDrawing();
		{
			ClearBackground(BLACK);

			float screen_w = float(GetScreenWidth());
			float screen_h = float(GetScreenHeight());
			if (IsWindowFullscreen()) {
				float up_surf_w = float(up_surf.texture.width);
				float up_surf_h = float(up_surf.texture.height);
				float scale = std::min(screen_w / up_surf_w, screen_h / up_surf_h);
				float final_w = up_surf_w * scale;
				float final_h = up_surf_h * scale;
				DrawTexturePro(
					up_surf.texture,
					{0.0f, 0.0f, up_surf_w, -up_surf_h},
					{screen_w / 2.0f, screen_h / 2.0f, final_w, final_h},
					{final_w / 2.0f, final_h / 2.0f},
					0.0f,
					WHITE
				);
			} else {
				DrawTexturePro(
					game_surf.texture,
					{0.0f, 0.0f, float(game_surf.texture.width), -float(game_surf.texture.height)},
					{0.0f, 0.0f, screen_w, screen_h},
					{0.0f, 0.0f},
					0.0f,
					WHITE
				);
			}

			if (debug_overlay) {
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
				DrawText(text, 0, 0, 20, RED);

				if (scene.index() == GAME_SCENE) {
					const char* text = TextFormat(
						"bullets %d\n"
						"player bullets %d\n"
						"enemies %d\n"
						"pickups %d\n"
						"coroutine calls %d\n"
						"physics calls %d",
						(int)std::get<GAME_SCENE>(scene).stage->bullets.size(),
						(int)std::get<GAME_SCENE>(scene).stage->player_bullets.size(),
						(int)std::get<GAME_SCENE>(scene).stage->enemies.size(),
						(int)std::get<GAME_SCENE>(scene).stage->pickups.size(),
						std::get<GAME_SCENE>(scene).stage->coroutine_calls,
						std::get<GAME_SCENE>(scene).stage->physics_calls
					);
					DrawText(text, 0, 250, 20, RED);
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
			_SetWindowMonitor(m, 0, 0, monitor_w, monitor_h, -1);

			ClearWindowState(FLAG_VSYNC_HINT);
			SetTargetFPS(GetMonitorRefreshRate(m) * 2);

			up_surf = LoadStrippedRenderTexture(GAME_W * max_scale, GAME_H * max_scale);
			SetTextureFilter(up_surf.texture, TEXTURE_FILTER_BILINEAR);
			SetTextureWrap(up_surf.texture, TEXTURE_WRAP_CLAMP);

			DisableCursor();
		} else {
			int window_w = GAME_W * (window_mode + 1);
			int window_h = GAME_H * (window_mode + 1);
			int window_x = int(GetMonitorPosition(m).x);
			int window_y = int(GetMonitorPosition(m).y);
			window_x += GetMonitorWidth(m) / 2;
			window_y += GetMonitorHeight(m) / 2;
			window_x -= window_w / 2;
			window_y -= window_h / 2;

			_SetWindowMonitor(-1, window_x, window_y, window_w, window_h, -1);

			SetWindowState(FLAG_VSYNC_HINT);
			SetTargetFPS(0);

			EnableCursor();
		}
	}

	void Game::set_master_volume(float vol)
	{
		options.master_volume = std::clamp(vol, 0.0f, 1.0f);
		SetMasterVolume(options.master_volume);
	}
}
