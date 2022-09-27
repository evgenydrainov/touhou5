#pragma once

#include <raylib.h>
#include <vector>
#include <string>

namespace th
{
	class Game;

	class TitleScene
	{
	public:
		TitleScene(Game& game);

		TitleScene(const TitleScene&) = delete;
		TitleScene& operator=(const TitleScene&) = delete;
		TitleScene(TitleScene&&) = delete;
		TitleScene& operator=(TitleScene&&) = delete;

		void update(float delta);
		void draw(RenderTexture target, float delta);

	private:
		Game& game;

		std::vector<std::string> menu_labels = {"Play", "Options", "Quit to Desktop"};
		int menu_cursor = 0;
	};
}
