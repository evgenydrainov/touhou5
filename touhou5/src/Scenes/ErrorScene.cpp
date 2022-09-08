#include "ErrorScene.h"

#include "Game.h"

#include "raylibx.h"
#include <iostream>

namespace th
{
	ErrorScene::ErrorScene(Game& game) : game(game)
	{
		std::cout << "[TOUHOU ERROR] " << game.error_msg << '\n';

		msg = game.error_msg + "\n\nPress Z to go back";
	}

	void ErrorScene::update(float delta)
	{
		if (IsKeyPressed(KEY_Z)) {
			game.next_scene = TITLE_SCENE;
			PlaySound(game.sndSelect);
		}
	}

	void ErrorScene::draw(RenderTexture2D target, float delta)
	{
		BeginTextureMode(target);
		{
			ClearBackground(RAYWHITE);

			DrawTextBoxed(GetFontDefault(),
						  msg.c_str(),
						  {0.0f, 0.0f, float(GAME_W), float(GAME_H)},
						  10.0f,
						  1.0f,
						  true,
						  RED);
		}
		EndTextureMode();
	}
}
