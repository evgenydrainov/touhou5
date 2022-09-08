#include "OptionsScene.h"

#include "Game.h"

#include "cpml.h"

#include <inipp.h>
#include <fstream>
#include <fmt/format.h>

namespace th
{
	OptionsScene::OptionsScene(Game& game) : game(game)
	{}

	OptionsScene::~OptionsScene()
	{
		inipp::Ini<char> ini;
		ini.sections["Game"]["Starting Lives"] = fmt::format("{}", game.options.starting_lives);
		ini.sections["Audio"]["Master Volume"] = fmt::format("{}", game.options.master_volume);
		std::ofstream f("options.ini");
		ini.generate(f);
	}

	void OptionsScene::update(float delta)
	{
		if (IsKeyPressed(KEY_X)) {
			if (menu_cursor == menu_labels.size() - 1) {
				game.next_scene = TITLE_SCENE;
			} else {
				menu_cursor = menu_labels.size() - 1;
			}
			PlaySound(game.sndCancel);
		}
		menu_cursor += IsKeyPressed(KEY_DOWN) - IsKeyPressed(KEY_UP);
		menu_cursor = cpml::emod(menu_cursor, menu_labels.size());
		if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP)) {
			PlaySound(game.sndSelect);
		}
		switch (menu_cursor) {
			case 0: {
				if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
					game.options.starting_lives += IsKeyPressed(KEY_RIGHT) - IsKeyPressed(KEY_LEFT);
					game.options.starting_lives = std::clamp(game.options.starting_lives, 1, 5);
					PlaySound(game.sndSelect);
				}
				break;
			}
			case 1: {
				if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
					float vol = game.options.master_volume;
					if (IsKeyPressed(KEY_LEFT))  vol -= 0.1f;
					if (IsKeyPressed(KEY_RIGHT)) vol += 0.1f;
					game.set_master_volume(vol);
					PlaySound(game.sndSelect);
				}
				break;
			}
			case 2: {
				if (IsKeyPressed(KEY_Z)) {
					game.next_scene = TITLE_SCENE;
					PlaySound(game.sndCancel);
				}
				break;
			}
		}
	}

	void OptionsScene::draw(RenderTexture2D target, float delta)
	{
		BeginTextureMode(target);
		{
			ClearBackground(BLACK);

			int menu_x = GAME_W / 10;
			int menu_y = GAME_H / 8;

			for (int i = 0; i < menu_labels.size(); i++) {
				DrawText(
					menu_labels[i].c_str(),
					menu_x, menu_y + i * 20,
					20, (i == menu_cursor) ? YELLOW : WHITE
				);
			}

			int off = 200;

			int lives_i = 0;
			for (int i = 0; i < 5; i++) {
				int lives = 1 + i;
				char text[2];
				text[0] = '0' + lives;
				text[1] = 0;
				DrawText(
					text,
					menu_x + off + 40 * i, menu_y + lives_i * 20,
					20, (lives == game.options.starting_lives) ? WHITE : DARKGRAY
				);
			}

			int master_slider_i = 1;
			DrawRectangle(
				menu_x + off, menu_y + master_slider_i * 20,
				200, 20,
				DARKGRAY
			);
			DrawRectangle(
				menu_x + off, menu_y + master_slider_i * 20,
				int(game.options.master_volume * 200.0f), 20,
				WHITE
			);
		}
		EndTextureMode();
	}
}
