#pragma once

#include "Scene.h"

namespace th
{
	class ErrorScene : public Scene
	{
	public:
		ErrorScene(Game& game, const std::string& what) :
			Scene(game),
			what(what)
		{}

		void Init() override;
		void Update(float delta) override;
		void Render(sf::RenderTarget& target, float delta) override;

		std::string what;
		sf::Text text;
	};
}
