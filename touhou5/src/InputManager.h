#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <cstdint>
#include <cstddef>

namespace th
{
	typedef uint32_t input_state;

	namespace Key
	{
		constexpr input_state Right = 1;
		constexpr input_state Up = 1 << 1;
		constexpr input_state Left = 1 << 2;
		constexpr input_state Down = 1 << 3;
		constexpr input_state Z = 1 << 4;
		constexpr input_state X = 1 << 5;
		constexpr input_state Shift = 1 << 6;
		constexpr size_t Count = 7;
	}

	class InputManager
	{
	public:
		void Update();

		bool Check(input_state key) { return (state & key) != 0; }
		bool CheckPressed(input_state key) { return (state_press & key) != 0; }
		bool CheckReleased(input_state key) { return (state_release & key) != 0; }

		input_state state = 0;
		input_state state_press = 0;
		input_state state_release = 0;

		sf::Keyboard::Key keys[Key::Count] = {
			sf::Keyboard::Right,
			sf::Keyboard::Up,
			sf::Keyboard::Left,
			sf::Keyboard::Down,
			sf::Keyboard::Z,
			sf::Keyboard::X,
			sf::Keyboard::LShift
		};
	};
}
