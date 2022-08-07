#pragma once

#include "Game.h"

namespace th
{
	class Scene
	{
	public:
		Scene(Game& game) :
			game(game),
			input(game.input),
			audio(game.audio)
		{}

		virtual ~Scene() = default;

		virtual void Init() {}
		virtual void Update(float delta) {}
		virtual void Render(sf::RenderTarget& target, float delta) {}

		Game& game;
		const Input& input;
		Audio& audio;
	};
}
