#include "Menu.h"

namespace th
{
	void Menu::Update(InputManager& input)
	{
		if (!buttons.empty()) {
			if (input.CheckPressed(Key::Down)) {
				if (cursor < buttons.size() - 1) cursor++;
			}

			if (input.CheckPressed(Key::Up)) {
				if (cursor != 0) cursor--;
			}

			if (input.CheckPressed(Key::Z)) {
				if (buttons[cursor].on_click) {
					buttons[cursor].on_click();
				}
			}
		}
	}
}
