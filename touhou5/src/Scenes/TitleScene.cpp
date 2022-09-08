#include "TitleScene.h"

#include "Game.h"

#include "cpml.h"

namespace th
{
	TitleScene::TitleScene(Game& game) : game(game)
	{}

	void TitleScene::update(float delta)
	{
		if (IsKeyPressed(KEY_X)) {
			if (menu_cursor != menu_labels.size() - 1) {
				menu_cursor = menu_labels.size() - 1;
				PlaySound(game.sndCancel);
			}
		}
		menu_cursor += IsKeyPressed(KEY_DOWN) - IsKeyPressed(KEY_UP);
		menu_cursor = cpml::emod(menu_cursor, menu_labels.size());
		if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP)) {
			PlaySound(game.sndSelect);
		}
		if (IsKeyPressed(KEY_Z)) {
			switch (menu_cursor) {
				case 0: game.next_scene = SCRIPT_SELECT_SCENE; break;
				case 1: game.next_scene = OPTIONS_SCENE;  break;
				case 2: game.quit = true; break;
			}
			if (menu_cursor == 2) {
				PlaySound(game.sndCancel);
			} else {
				PlaySound(game.sndOk);
			}
		}
	}

	void TitleScene::draw(RenderTexture target, float delta)
	{
		BeginTextureMode(target);
		{
			ClearBackground(BLACK);

			for (int i = 0; i < menu_labels.size(); i++) {
				DrawText(
					menu_labels[i].c_str(),
					GAME_W / 3 * 2 - i * 8, GAME_H / 2 + i * 32,
					20, (i == menu_cursor) ? YELLOW : WHITE
				);
			}

			DrawText(
				"F4 - Fullscreen\n\n"
				"F1 - Show Hitboxes\n"
				"F2 - Restart\n"
				"F3 - Debug Overlay\n"
				"F5 - Advance Single Frame\n"
				"F6 - Resume Game Normally\n"
				"F - Fast Forward\n"
				"G - God Mode\n\n"
				"S - Skip Boss Phase\n"
				"L - Get 8 Lives\n"
				"B - Get 8 Bombs\n"
				"P - Get Max Power",
				10, 10,
				10, RED
			);
		}
		EndTextureMode();
	}
}
