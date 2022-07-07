#include "InputManager.h"

namespace th
{
	void InputManager::Update()
	{
		input_state buffer = state;
		state = 0;

		for (size_t i = 0; i < Key::Count; i++) {
			state |= sf::Keyboard::isKeyPressed(keys[i]) << i;
		}

		state_press = state & buffer ^ state;
		state_release = state & buffer ^ buffer;
	}
}
