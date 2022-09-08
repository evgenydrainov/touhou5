#pragma once

#include <raylib.h>
#include <vector>
#include <string>

namespace th
{
	struct Game;

	struct TitleScene
	{
		Game& game;

		std::vector<std::string> menu_labels = {"Play", "Options", "Quit to Desktop"};
		int menu_cursor = 0;

		TitleScene(const TitleScene&) = delete;
		TitleScene& operator=(const TitleScene&) = delete;
		TitleScene(TitleScene&&) = delete;
		TitleScene& operator=(TitleScene&&) = delete;

		TitleScene(Game& game);

		void update(float delta);
		void draw(RenderTexture target, float delta);
	};
}
