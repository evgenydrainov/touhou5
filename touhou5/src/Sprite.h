#pragma once

#include <raylib.h>

namespace th
{
	struct Sprite
	{
		Texture2D tex;
		int frame_count;
		float anim_spd;
		int loop_frame;
	};

	Sprite LoadSprite(const char* fname, int frame_count, float anim_spd, int loop_frame);

	void UnloadSprite(Sprite spr);
}
