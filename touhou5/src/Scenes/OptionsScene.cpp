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
		menu_cursor += (game.key_pressed == KEY_DOWN) - (game.key_pressed == KEY_UP);
		menu_cursor = cpml::emod(menu_cursor, menu_labels.size());
		if ((game.key_pressed == KEY_DOWN) - (game.key_pressed == KEY_UP)) {
			PlaySound(game.sndSelect);
		}
		switch (menu_cursor) {
			case 0: {
				if ((game.key_pressed == KEY_RIGHT) - (game.key_pressed == KEY_LEFT)) {
					int lives = game.options.starting_lives + (game.key_pressed == KEY_RIGHT) - (game.key_pressed == KEY_LEFT);
					if (lives >= 1 && lives <= 5) {
						game.options.starting_lives = lives;
						PlaySound(game.sndSelect);
					}
				}
				break;
			}
			case 1: {
				if ((game.key_pressed == KEY_RIGHT) - (game.key_pressed == KEY_LEFT)) {
					int vol = game.options.master_volume + (game.key_pressed == KEY_RIGHT) - (game.key_pressed == KEY_LEFT);
					if (vol >= 0 && vol <= 10) {
						game.options.master_volume = vol;
						SetMasterVolume(float(game.options.master_volume) / 10.0f);
						PlaySound(game.sndSelect);
					}
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
				game.options.master_volume * 200 / 10, 20,
				WHITE
			);
		}
		EndTextureMode();
	}
}
