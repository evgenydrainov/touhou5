#include "Game.h"

#include "Scenes/TitleScene.h"

#include "misc.h"
#include <fmt/format.h>

#if TH_DEBUG
#include "Scenes/GameScene.h"
#endif

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
#if TH_DEBUG
					case sf::Event::KeyPressed: {
						switch (event.key.code) {
							case sf::Keyboard::F1: {
								show_hitboxes ^= true;
								break;
							}
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
#endif
				}
			}

			float t = clock.restart().asSeconds();
			t = std::min(t, 1.0f / 30.0f);

			float delta = t * 60.0f;

			fps_sum += 1.0f / t;
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
		characters[0].name = "Reimu Hakurei";
		characters[0].move_spd = 3.75f;
		characters[0].focus_spd = 1.75f;
		characters[0].radius = 2.0f;
		characters[0].graze_radius = 16.0f;

		{
			sf::VideoMode m = sf::VideoMode::getDesktopMode();
			int xscale = m.width / GAME_W;
			int yscale = m.height / GAME_H;
			int scale = std::min(xscale, yscale);
			scale = std::max(scale, 1);
			window.create(sf::VideoMode(GAME_W * scale, GAME_H * scale), "th");
		}

		window.setVerticalSyncEnabled(true);

		game_surf.create(GAME_W, GAME_H);

		font.loadFromFile("Oranienbaum-Regular.ttf");

		next_scene = std::make_unique<TitleScene>(*this);

		sf::Listener::setGlobalVolume(0.0f);
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

#if TH_DEBUG
		{
			static sf::Text t;
			t.setFont(font);
			t.setCharacterSize(24);
			std::string str = fmt::format(
				"{:.2f}fps\n"
				"update {:.2f}ms\n"
				"render {:.2f}ms\n"
				"time passed {:.2f}\n"
				"frames passed {}\n"
				"delta {}",
				fps_real,
				update_took,
				render_took,
				time,
				frame,
				delta
			);
			if (GameScene* s = dynamic_cast<GameScene*>(scene.get())) {
				str += fmt::format(
					"\n\n"
					"bullets: {}\n"
					"enemies: {}\n"
					"player bullets: {}\n"
					"pickups: {}\n"
					"did physics: {}\n"
					"did co: {}\n"
					"co timer: {}",
					s->bullets.size(),
					s->enemies.size(),
					s->player_bullets.size(),
					s->pickups.size(),
					s->did_physics,
					s->did_co,
					s->co_timer
				);
			}
			t.setString(str);
			window.draw(t);
		}
#endif

		window.display();
	}

	void Game::Update(float delta)
	{
		sf::Clock c;

		input.Update();
		audio.Update();

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
