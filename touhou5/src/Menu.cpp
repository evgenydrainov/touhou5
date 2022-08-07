#include "Menu.h"

namespace th
{
	void Menu::Update()
	{
		if (!entities.empty()) {
			if (input.CheckPressed(Key::Down)) {
				++cursor;
			}

			if (input.CheckPressed(Key::Up)) {
				if (cursor != 0) --cursor;
			}

			cursor = std::min(cursor, entities.size() - 1);

			if (MenuButton* b = dynamic_cast<MenuButton*>(entities[cursor].get())) {
				if (input.CheckPressed(Key::Z)) {
					b->on ^= true;
					b->OnPressed();
				}
			} else if (MenuSlider* s = dynamic_cast<MenuSlider*>(entities[cursor].get())) {
				if (input.CheckPressed(Key::Left | Key::Right)) {
					if (input.CheckPressed(Key::Right)) ++s->step;
					if (input.CheckPressed(Key::Left)) --s->step;
					s->step = std::clamp(s->step, 0, s->steps);
					s->value = s->from + (s->to - s->from) * ((float)s->step / (float)s->steps);
					s->OnValueChanged();
				}
			}
		}
	}
}
