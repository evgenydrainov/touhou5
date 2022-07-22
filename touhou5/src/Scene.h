#pragma once

#include "Game.h"

namespace th
{
	class Scene
	{
	public:
		Scene(Game& game) : game(game), input(game.input) {}
		virtual ~Scene() = default;

		virtual void Init() {}
		virtual void Update(float delta) {}
		virtual void Render(sf::RenderTarget& target, float delta) {}

		Game& game;
		InputManager& input;
	};
}
