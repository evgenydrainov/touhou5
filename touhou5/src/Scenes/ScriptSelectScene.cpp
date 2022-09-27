#include "ScriptSelectScene.h"

#include "Game.h"

#include "cpml.h"
#include <fmt/format.h>

namespace th
{
	ScriptSelectScene::ScriptSelectScene(Game& game) : game(game)
	{
		std::error_code err;
		for (const auto& e : std::filesystem::directory_iterator(game.scripts_path, err)) {
			if (!e.is_directory()) continue;
			if (e.path().filename().string()[0] == '.') continue;
			paths.emplace_back(e.path());
			menu_labels.emplace_back(fmt::format("Play {}", e.path().filename().string()));
		}
		menu_labels.emplace_back("Back");
	}

	void ScriptSelectScene::update(float delta)
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
		if (IsKeyPressed(KEY_Z)) {
			if (menu_cursor < menu_labels.size() - 1) {
				game.character_id = 0;
				game.script_path = paths[menu_cursor];
				game.next_scene = GAME_SCENE;
				PlaySound(game.sndOk);
			} else {
				game.next_scene = TITLE_SCENE;
				PlaySound(game.sndCancel);
			}
		}
	}

	void ScriptSelectScene::draw(RenderTexture2D target, float delta)
	{
		BeginTextureMode(target);
		{
			ClearBackground(BLACK);

			int menu_x = GAME_W / 8;
			int menu_y = GAME_H / 10;

			for (int i = 0; i < menu_labels.size(); i++) {
				DrawText(
					menu_labels[i].c_str(),
					menu_x, menu_y + i * 20,
					20, (i == menu_cursor) ? YELLOW : WHITE
				);
			}
		}
		EndTextureMode();
	}
}
