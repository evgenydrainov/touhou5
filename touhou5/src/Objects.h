#pragma once

namespace th
{
	typedef uint32_t instance_id;

	struct Player
	{
		float x;
		float y;
		float radius;
		float move_spd;
		float focus_spd;
		int score;
		int lives;
		int bombs;
		int power;
		int graze;
		int points;
		bool is_focused;
	};

	struct Bullet
	{
		instance_id id;
		float x;
		float y;
		float spd;
		float acc;
		float dir;
		float radius;
	};

	struct Enemy
	{
		float x;
		float y;
		float radius;
	};

	struct Boss
	{
		instance_id id;
		float x;
		float y;
		float radius;
	};

	struct Powerup
	{
		float x;
		float y;
		float vsp;
		float grv;
		float max_vsp;
		float radius;
	};
}
