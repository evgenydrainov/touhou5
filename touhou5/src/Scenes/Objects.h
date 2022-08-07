#pragma once

#include <sol/sol.hpp>
#include <SFML/Graphics.hpp>

namespace th
{
	typedef uint32_t instance_id;

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
		sf::Texture* texture;

		int score;
		int lives;
		int bombs;
		int power;
		int graze;
		int points;

		bool is_focused;

		int character;
		union
		{
			Reimu reimu;
			Marisa marisa;
		};
	};

	constexpr int PLAYER_BULLET_REIMU_CARD = 0;

	struct ReimuCard
	{
		float rotation;
	};

	struct PlayerBullet
	{
		float x;
		float y;
		float spd;
		float dir;
		float acc;
		sf::Texture* texture;
		float radius;
		float dmg;
		int type;
		union
		{
			ReimuCard reimu_card;
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

	constexpr int PICKUP_POINTS = 0;
	constexpr int PICKUP_POWER = 1;

	struct Pickup
	{
		float x;
		float y;
		float vsp;
		float grv;
		float max_vsp;
		float radius;
		int type;
	};
}
