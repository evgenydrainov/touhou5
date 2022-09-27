#pragma once

#include <raylib.h>
#include <string>

namespace th
{
	class Game;

	class ErrorScene
	{
	public:
		ErrorScene(Game& game);

		ErrorScene(const ErrorScene&) = delete;
		ErrorScene& operator=(const ErrorScene&) = delete;
		ErrorScene(ErrorScene&&) = delete;
		ErrorScene& operator=(ErrorScene&&) = delete;

		void update(float delta);
		void draw(RenderTexture2D target, float delta);

	private:
		Game& game;

		std::string msg;
	};
}
