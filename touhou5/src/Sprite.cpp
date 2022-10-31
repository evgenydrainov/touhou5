#include "Sprite.h"

namespace th
{
	Sprite LoadSprite(const char* fname, int frame_count, float anim_spd, int loop_frame)
	{
		Sprite res = {};
		res.tex = LoadTexture(fname);
		res.frame_count = frame_count;
		res.anim_spd = anim_spd;
		res.loop_frame = loop_frame;
		return res;
	}

	void UnloadSprite(Sprite spr)
	{
		UnloadTexture(spr.tex);
	}
}
