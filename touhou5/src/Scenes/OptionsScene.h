#pragma once

#include <raylib.h>
#include <vector>
#include <string>

namespace th
{
	class Game;

	class OptionsScene
	{
	public:
		OptionsScene(Game& game);
		~OptionsScene();

		OptionsScene(const OptionsScene&) = delete;
		OptionsScene& operator=(const OptionsScene&) = delete;
		OptionsScene(OptionsScene&&) = delete;
		OptionsScene& operator=(OptionsScene&&) = delete;

		void update(float delta);
		void draw(RenderTexture2D target, float delta);

	private:
		Game& game;

		std::vector<std::string> menu_labels = {"Starting Lives", "Master Volume", "Back"};
		int menu_cursor = 0;
	};
}
