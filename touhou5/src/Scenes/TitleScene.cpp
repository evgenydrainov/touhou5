#include "TitleScene.h"

#include "GameScene.h"

namespace th
{
	ScriptButton::ScriptButton(Game& game, const std::filesystem::path& path) :
		MenuButton(game),
		path(path)
	{
		label = path.filename().string();
	}

	void ScriptButton::OnPressed()
	{
		game.next_scene = std::make_unique<GameScene>(game, path);
	}

	void TitleScene::Init()
	{
		std::error_code err;
		for (const auto& e : std::filesystem::directory_iterator(game.scripts_path, err)) {
			if (!e.is_directory()) continue;
			if (e.path().filename().string()[0] == '.') continue;
			menu.entities.emplace_back(std::make_unique<ScriptButton>(game, e.path()));
		}

		label.setFont(game.font);
		label.setCharacterSize(16);
		label.setString("Touhou 69\nEmbodiment of Sussy Baka\n\nPress Z to Start");
		label.setPosition(100.0f, 100.0f);
	}

	void TitleScene::Update(float delta)
	{
		menu.Update();
	}

	void TitleScene::Render(sf::RenderTarget& target, float delta)
	{
		target.draw(label);

		float x = 100.0f;
		float y = 212.0f;
		for (size_t i = 0; i < menu.entities.size(); i++) {
			sf::Text t;
			t.setFont(game.font);
			t.setCharacterSize(16);
			t.setString(menu.entities[i]->label);
			t.setPosition(x, y);
			y += 16.0f;
			if (i == menu.cursor) {
				t.setFillColor(sf::Color::Yellow);
			}
			target.draw(t);
		}
	}
}
