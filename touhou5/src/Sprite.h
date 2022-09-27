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
}
