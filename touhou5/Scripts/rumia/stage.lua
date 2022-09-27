local th = require('touhou')

local texCirno		= LoadTexture('cirno.png')

local Nonspell1 = {
	HP = 4000,
	Time = 30,
	Type = PHASE_NONSPELL,
	Script = function(id)
		local xstart = BossGetX(id)
		local ystart = BossGetY(id)

		while true do
			th.BossWander(id)

			th.wait(60)

			th.BossLaunchTowardsPoint(id, xstart, ystart, .01)

			th.wait(60)
		end
	end
}

local Rumia = {
	Name = 'Rumia',
	Texture = texCirno,
	Phases = {Nonspell1},
	Healthbars = {1},
	Music = -1
}

local Stage = {
	Music = -1,
	Script = function()
		CreateBoss(Rumia)
	end
}

return Stage
