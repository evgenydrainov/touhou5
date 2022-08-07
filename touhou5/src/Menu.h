#pragma once

#include "Game.h"

namespace th
{
	class MenuEntity
	{
	public:
		MenuEntity(Game& game) :
			game(game),
			input(game.input),
			audio(game.audio)
		{}

		virtual ~MenuEntity() = default;

		Game& game;
		const Input& input;
		Audio& audio;

		std::string label;
	};

	class MenuButton : public MenuEntity
	{
	public:
		MenuButton(Game& game) :
			MenuEntity(game)
		{}

		virtual void OnPressed() {}

		bool on = false;
	};

	class MenuSlider : public MenuEntity
	{
	public:
		MenuSlider(Game& game) :
			MenuEntity(game)
		{}

		virtual void OnValueChanged() {}

		float from = 0.0f;
		float to = 0.0f;
		int steps = 1;
		int step = 0;
		float value = 0.0f;
	};

	class Menu
	{
	public:
		Menu(Game& game) :
			game(game),
			input(game.input),
			audio(game.audio)
		{}

		void Update();

		Game& game;
		const Input& input;
		Audio& audio;

		std::vector<std::unique_ptr<MenuEntity>> entities;
		size_t cursor = 0;
	};
}
