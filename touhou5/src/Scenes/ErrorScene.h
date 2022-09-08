#pragma once

#include <raylib.h>
#include <string>

namespace th
{
	struct Game;

	struct ErrorScene
	{
		Game& game;

		std::string msg;

		ErrorScene(const ErrorScene&) = delete;
		ErrorScene& operator=(const ErrorScene&) = delete;
		ErrorScene(ErrorScene&&) = delete;
		ErrorScene& operator=(ErrorScene&&) = delete;

		ErrorScene(Game& game);

		void update(float delta);
		void draw(RenderTexture2D target, float delta);
	};
}
