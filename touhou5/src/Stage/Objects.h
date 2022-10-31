#pragma once

#include "Sprite.h"
#include <sol/sol.hpp>

namespace th
{
	typedef uint32_t instance_id;

	struct Reimu
	{
		float fire_timer;
		int fire_queue;
		float orb_x[2];
		float orb_y[2];
	};

	struct Marisa {};

	enum PlayerStates
	{
		PLAYER_STATE_NORMAL,
		PLAYER_STATE_DYING,
		PLAYER_STATE_APPEARING
	};

	constexpr float PLAYER_DEATH_TIME = 20.0f;
	constexpr float PLAYER_RESPAWN_IFRAMES = 120.0f;

	struct Player
	{
		float x;
		float y;
		float hsp;
		float vsp;

		float hitbox_alpha;
		float timer;
		float iframes;
		Sprite spr;
		float frame_index;
		float xscale;
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
		PLAYER_BULLET_REIMU_CARD,
		PLAYER_BULLET_REIMU_ORB_SHOT
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

	enum Projectiles
	{
		PROJECTILE_BULLET,
		PROJECTILE_LAZER,
		PROJECTILE_RECT
	};

	// rename to projectile
	struct Bullet
	{
		instance_id id;
		float x;
		float y;
		float spd;
		float dir;
		float acc;

		union
		{
			struct
			{
				float radius;
				bool rotate;
			};

			struct
			{
				float length;
				float thickness;
			};
		};

		int texture_id;
		float lifetime;
		bool grazed;
		char type;
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
		int sprite_id;
		float frame_index;
		float hp;
		std::unique_ptr<sol::thread> co_runner;
		std::unique_ptr<sol::coroutine> co;
	};

	enum BossPhases
	{
		PHASE_NONSPELL,
		PHASE_SPELLCARD
	};

	struct BossPhase
	{
		sol::coroutine script;
		float time;
		float hp;
		char type;
		std::string name;
	};

	constexpr float BOSS_WAIT_TIME = 100.0f;

	struct Boss
	{
		instance_id id;
		float x;
		float y;
		float spd;
		float dir;
		float acc;
		float radius;
		int sprite_id;
		float frame_index;
		float xscale;
		float hp;
		sol::thread co_runner;
		sol::coroutine co;
		float timer;
		std::string name;
		std::vector<BossPhase> phases;
		size_t phase_index;
		float wait_timer;
		char wait_flag;
		bool midboss;
	};

	enum Pickups
	{
		PICKUP_POWER,
		PICKUP_POINT,
		PICKUP_BIG_POWER,
		PICKUP_BOMB,
		PICKUP_FULL_POWER,
		PICKUP_1UP,
		PICKUP_SCORE
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
