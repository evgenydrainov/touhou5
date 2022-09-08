#pragma once

#include <raylib.h>
#include <vector>
#include <string>

namespace th
{
	struct Game;

	struct OptionsScene
	{
		Game& game;

		std::vector<std::string> menu_labels = {"Starting Lives", "Master Volume", "Back"};
		int menu_cursor = 0;

		OptionsScene(const OptionsScene&) = delete;
		OptionsScene& operator=(const OptionsScene&) = delete;
		OptionsScene(OptionsScene&&) = delete;
		OptionsScene& operator=(OptionsScene&&) = delete;

		OptionsScene(Game& game);
		~OptionsScene();

		void update(float delta);
		void draw(RenderTexture2D target, float delta);
	};
}
