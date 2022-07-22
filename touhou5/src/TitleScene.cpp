#include "TitleScene.h"

#include "GameplayScene.h"

namespace th
{
	void TitleScene::Init()
	{
		for (const auto& e : std::filesystem::directory_iterator(game.scripts_path)) {
			if (!e.is_directory()) continue;
			if (e.path().filename().string()[0] == '.') continue;

			menu.buttons.emplace_back(e.path().filename().string(), [this, path = e.path()]()
			{
				game.next_scene = std::make_unique<GameplayScene>(game, path);
			});
		}

		label.setFont(game.font);
		label.setCharacterSize(16);
		label.setString("Touhou 69\nEmbodiment of Sussy Baka\n\nPress Z to Start");
		label.setPosition(100.0f, 100.0f);
	}

	void TitleScene::Update(float delta)
	{
		menu.Update(input);
	}

	void TitleScene::Render(sf::RenderTarget& target, float delta)
	{
		target.draw(label);

		float x = 100.0f;
		float y = 212.0f;
		for (size_t i = 0; i < menu.buttons.size(); i++) {
			sf::Text t;
			t.setFont(game.font);
			t.setCharacterSize(16);
			t.setString(menu.buttons[i].label);
			t.setPosition(x, y);
			y += 16.0f;
			if (i == menu.cursor) {
				t.setFillColor(sf::Color::Yellow);
			}
			target.draw(t);
		}
	}
}
