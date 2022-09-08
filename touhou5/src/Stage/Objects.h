#pragma once

#include <sol/sol.hpp>

namespace th
{
	typedef uint32_t instance_id;

	struct Reimu
	{
		float fire_timer;
		int fire_queue;
	};

	struct Marisa {};

	enum PlayerStates
	{
		PLAYER_STATE_NORMAL,
		PLAYER_STATE_DYING,
		PLAYER_STATE_APPEARING
	};

	constexpr float PLAYER_DEATH_TIME = 20.0f;
	constexpr float PLAYER_RESPAWN_IFRAMES = 60.0f;

	struct Player
	{
		float x;
		float y;
		float hsp;
		float vsp;

		float hitbox_alpha;
		float timer;
		float iframes;
		char state;
		bool is_focused;

		union
		{
			Reimu reimu;
			Marisa marisa;
		};
	};

	enum PlayerBullets
	{
		PLAYER_BULLET_REIMU_CARD
	};

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
		float radius;
		float dmg;
		char type;
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
		int texture_id;
		float lifetime;
		bool rotate;
		bool grazed;
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
		int texture_id;
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

	enum BossStates
	{
		BOSS_STATE_WAITING,
		BOSS_STATE_NORMAL
	};

	const float BOSS_WAIT_TIME = 60.0f;

	struct Boss
	{
		instance_id id;
		float x;
		float y;
		float spd;
		float dir;
		float acc;
		float radius;
		int texture_id;
		float hp;
		sol::thread co_runner;
		sol::coroutine co;
		float timer;
		std::string name;
		std::vector<BossPhase> phases;
		size_t phase_index;
		float wait_timer;
		char state;
	};

	enum Pickups
	{
		PICKUP_POINTS,
		PICKUP_POWER,
		PICKUP_BIGP,
		PICKUP_SCORE,
		PICKUP_MAX_POWER
	};

	struct Pickup
	{
		float x;
		float y;
		float vsp;
		float grv;
		float max_vsp;
		float radius;
		char type;
	};
}
