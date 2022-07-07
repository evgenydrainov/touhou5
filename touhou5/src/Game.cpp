#include "Game.h"

#include "GameplayScene.h"

#include "misc.h"
#include <fmt/format.h>

namespace th
{
	Game::Game() = default;

	Game::~Game() = default;

	void Game::Run()
	{
		window.create(sf::VideoMode(GAME_W, GAME_H), "th");
		window.setVerticalSyncEnabled(true);
		window.setKeyRepeatEnabled(false);

		game_surf.create(GAME_W, GAME_H);

		font.loadFromFile("Oranienbaum-Regular.ttf");

		scene = std::make_unique<GameplayScene>(*this);
		scene->Init();

		sf::Clock clock;
		while (window.isOpen()) {
			sf::Event event;
			while (window.hasFocus() ? window.pollEvent(event) : window.waitEvent(event)) {
				if (event.type == sf::Event::Closed) {
					window.close();
				}
			}

			sf::Time t = clock.restart();
			float delta = std::min(t.asSeconds(), 1.0f / 30.0f) * 60.0f;

			mean_fps_sum += 1.0f / t.asSeconds();
			mean_fps_samples++;
			if (mean_fps_samples >= 100) {
				mean_fps = mean_fps_sum / (float)mean_fps_samples;
				mean_fps_sum = 0.0f;
				mean_fps_samples = 0;
			}

			Update(delta);

			window.clear();
			{
				Render(window, delta);
			}
			window.display();

			time += delta;
			frame++;
		}
	}

	void Game::Update(float delta)
	{
		sf::Clock c;

		input.Update();

		if (next_scene) {
			scene = std::move(next_scene);
			scene->Init();
		}

		scene->Update(delta);

		update_took = c.getElapsedTime().asSeconds() * 1000.0f;
	}

	void Game::Render(sf::RenderTarget& target, float delta)
	{
		sf::Clock c;

		game_surf.clear();
		{
			scene->Render(game_surf, delta);
			static sf::Text t;
			t.setFont(font);
			t.setCharacterSize(16);
			t.setString(fmt::format(
				"{:.2f}fps\n"
				"update {:.2f}ms\n"
				"render {:.2f}ms",
				mean_fps,
				update_took,
				render_took
			));
			t.setPosition(GAME_W, GAME_H);
			AlignText(t, HAlign::Right, VAlign::Bottom);
			game_surf.draw(t);
		}
		game_surf.display();

		target.draw(sf::Sprite(game_surf.getTexture()));

		render_took = c.getElapsedTime().asSeconds() * 1000.0f;
	}
}
