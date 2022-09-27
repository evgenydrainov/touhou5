#pragma once

#include <raylib.h>
#include <filesystem>

namespace th
{
	class Game;

	class ScriptSelectScene
	{
	public:
		ScriptSelectScene(Game& game);

		ScriptSelectScene(const ScriptSelectScene&) = delete;
		ScriptSelectScene& operator=(const ScriptSelectScene&) = delete;
		ScriptSelectScene(ScriptSelectScene&&) = delete;
		ScriptSelectScene& operator=(ScriptSelectScene&&) = delete;

		void update(float delta);
		void draw(RenderTexture2D target, float delta);

	private:
		Game& game;

		std::vector<std::filesystem::path> paths;
		std::vector<std::string> menu_labels;
		int menu_cursor = 0;
	};
}
