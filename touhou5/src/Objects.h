#pragma once

#include <sol/sol.hpp>
#include <SFML/Graphics.hpp>

namespace th
{
	typedef uint32_t instance_id;

	struct Character
	{
		std::string name;
		float move_spd;
		float focus_spd;
		float radius;
		float graze_radius;
		sf::Texture* texture;
	};

	struct Reimu
	{
		float fire_timer;
		int fire_queue;
	};

	struct Marisa {};

	struct Player
	{
		float x;
		float y;
		float hsp;
		float vsp;
		size_t character_id;
		int score;
		int lives;
		int bombs;
		int power;
		int graze;
		int points;
		bool is_focused;
		union
		{
			Reimu reimu;
			Marisa marisa;
		};
	};

	struct Bullet
	{
		instance_id id;
		float x;
		float y;
		float spd;
		float dir;
		float acc;
		float radius;
		sf::Texture* texture;
		float dmg;
		bool rotate;
	};

	struct Enemy
	{
		instance_id id;
		float x;
		float y;
		float spd;
		float dir;
		float acc;
		float radius;
		sf::Texture* texture;
		float hp;
		sol::thread co_runner;
		sol::coroutine co;
	};

	struct BossPhase
	{
		sol::coroutine script;
		float time;
		float hp;
	};

	struct Boss
	{
		float x;
		float y;
		float spd;
		float dir;
		float acc;
		float radius;
		sf::Texture* texture;
		float hp;
		sol::thread co_runner;
		sol::coroutine co;
		float timer;
		std::string name;
		std::vector<BossPhase> phases;
		size_t phase_index;
	};

	enum class PowerupType
	{
		Points,
		Power
	};

	struct Powerup
	{
		float x;
		float y;
		float vsp;
		float grv;
		float max_vsp;
		float radius;
		PowerupType type;
	};
}
