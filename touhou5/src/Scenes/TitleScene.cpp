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
		menu_cursor += (game.key_pressed == KEY_DOWN) - (game.key_pressed == KEY_UP);
		menu_cursor = cpml::emod(menu_cursor, menu_labels.size());
		if ((game.key_pressed == KEY_DOWN) - (game.key_pressed == KEY_UP)) {
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
				"F4 - Fullscreen\n"
				"\n"
				"Anywhere\n"
				"  F1 - Show Hitboxes\n"
				"  F2 - Restart\n"
				"  F3 - Debug Overlay\n"
				"  F5 - Advance Single Frame\n"
				"  F6 - Resume Game Normally\n"
				"  F - Fast Forward\n"
				"  I - Invincibility\n"
				"\n"
				"In-Game\n"
				"  S - Skip Boss Phase\n"
				"  [Shift/Alt +] L - Get/Lose Lives\n"
				"  [Shift/Alt +] B - Get/Lose Bombs\n"
				"  [Shift/Ctrl/Alt +] P - Get/Lose Power\n"
				"  [Shift/Ctrl/Alt +] . - Get/Lose Points\n"
				"  [Shift/Ctrl/Alt +] G - Get/Lose Graze",
				0, 0,
				10, RED
			);
		}
		EndTextureMode();
	}
}
