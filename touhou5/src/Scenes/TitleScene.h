#pragma once

#include "Scene.h"
#include "Menu.h"

namespace th
{
	class ScriptButton : public MenuButton
	{
	public:
		ScriptButton(Game& game, const std::filesystem::path& path);

		void OnPressed() override;

		std::filesystem::path path;
	};

	class TitleScene : public Scene
	{
	public:
		TitleScene(Game& game) :
			Scene(game),
			menu(game)
		{}

		void Init() override;
		void Update(float delta) override;
		void Render(sf::RenderTarget& target, float delta) override;

		Menu menu;
		sf::Text label;
	};
}
