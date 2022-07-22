#include "Game.h"

#include "TitleScene.h"

#include "misc.h"
#include <fmt/format.h>

namespace th
{
	Game::Game() = default;

	Game::~Game() = default;

	void Game::Run()
	{
		Init();

		sf::Clock clock;
		while (window.isOpen()) {
			do_frame = !frame_advance;

			sf::Event event;
			while (window.hasFocus() ? window.pollEvent(event) : window.waitEvent(event)) {
				switch (event.type) {
					case sf::Event::Closed: {
						window.close();
						break;
					}
					case sf::Event::KeyPressed: {
#if TH_DEBUG
						if (event.key.code == sf::Keyboard::F1) debug ^= true;
#endif
						if (!debug) break;
						switch (event.key.code) {
							// restart
							case sf::Keyboard::F2: {
								next_scene = std::make_unique<TitleScene>(*this);
								break;
							}
							case sf::Keyboard::F5: {
								frame_advance = true;
								do_frame = true;
								break;
							}
							case sf::Keyboard::F6: {
								frame_advance = false;
								break;
							}
						}
						break;
					}
				}
			}

			sf::Time t = clock.restart();
			float delta = std::min(t.asSeconds(), 1.0f / 30.0f) * 60.0f;

			fps_sum += 1.0f / t.asSeconds();
			fps_samples++;
			fps_clock += delta;
			if (fps_clock >= 60.0f) {
				fps = fps_sum / (float)fps_samples;
				fps_sum = 0.0f;
				fps_samples = 0;
				fps_clock = 0.0f;
			}

			Tick(delta);

			time += delta;
			frame++;
		}
	}

	void Game::Init()
	{
		window.create(sf::VideoMode(GAME_W, GAME_H), "th");
		window.setVerticalSyncEnabled(true);

		game_surf.create(GAME_W, GAME_H);

		font.loadFromFile("Oranienbaum-Regular.ttf");

		next_scene = std::make_unique<TitleScene>(*this);
	}

	void Game::Tick(float delta)
	{
		sf::Clock c;

		Update(delta);

		window.clear();
		{
			Render(window, delta);
		}

		fps_real = 1.0f / c.getElapsedTime().asSeconds();

		if (debug) {
			static sf::Text t;
			t.setFont(font);
			t.setCharacterSize(24);
			t.setString(fmt::format(
				"{:.2f}fps\n"
				"update {:.2f}ms\n"
				"render {:.2f}ms\n"
				"time passed {:.2f}\n"
				"frames passed {}\n",
				fps_real,
				update_took,
				render_took,
				time,
				frame
			));
			window.draw(t);
		}

		window.display();
	}

	void Game::Update(float delta)
	{
		sf::Clock c;

		input.Update();

		if (next_scene) {
			scene = std::move(next_scene);
			scene->Init();
		}

		if (do_frame) {
			scene->Update(delta);
		}

		update_took = c.getElapsedTime().asSeconds() * 1000.0f;
	}

	void Game::Render(sf::RenderTarget& target, float delta)
	{
		sf::Clock c;

		if (do_frame) {
			game_surf.clear();

			scene->Render(game_surf, delta);

			{
				static sf::Text t;
				t.setFont(font);
				t.setCharacterSize(16);
				t.setString(fmt::format("{:.2f}fps", fps));
				t.setPosition(GAME_W, GAME_H);
				AlignText(t, HAlign::Right, VAlign::Bottom);
				game_surf.draw(t);
			}

			game_surf.display();
		}

		{
			sf::Vector2f to(target.getSize());
			sf::Vector2f from(game_surf.getSize());
			target.setView(sf::View(to / 2.0f, to));
			float scale = std::min(to.x / from.x, to.y / from.y);
			sf::Sprite s;
			s.setTexture(game_surf.getTexture());
			s.setScale(scale, scale);
			s.setPosition(to / 2.0f);
			s.setOrigin(from / 2.0f);
			target.draw(s);
		}

		render_took = c.getElapsedTime().asSeconds() * 1000.0f;
	}
}
