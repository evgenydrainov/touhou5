#pragma once

#include <vector>
#include <string>
#include <functional>
#include "InputManager.h"

namespace th
{
	struct MenuButton
	{
		std::string label;
		std::function<void()> on_click;
	};

	class Menu
	{
	public:
		void Update(InputManager& input);

		std::vector<MenuButton> buttons;
		size_t cursor = 0;
	};
}
