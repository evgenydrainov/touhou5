#pragma once

#include "Scene.h"
#include "Menu.h"

namespace th
{
	class TitleScene : public Scene
	{
	public:
		TitleScene(Game& game) : Scene(game) {}

		void Init() override;
		void Update(float delta) override;
		void Render(sf::RenderTarget& target, float delta) override;

		Menu menu;
		sf::Text label;
	};
}
