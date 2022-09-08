local th = require('touhou')

local texCirno		= LoadTexture('cirno.png')

local function BossLaunchTowardsPoint(id, target_x, target_y, acc)
	acc = math.abs(acc)
	local x = BossGetX(id)
	local y = BossGetY(id)
	local dist = th.point_distance(x, y, target_x, target_y)
	BossSetSpd(id, math.sqrt(dist * acc * 2))
	BossSetAcc(id, -acc)
	BossSetDir(id, th.point_direction(x, y, target_x, target_y))
end

local function random(a, b)
	return a + (b - a) * math.random()
end

local function launch_wherever(id)
	local dir = random(0, 360)
	local target_x = BossGetX(id) + th.lengthdir_x(50, dir)
	local target_y = BossGetY(id) + th.lengthdir_y(50, dir)
	BossLaunchTowardsPoint(id, target_x, target_y, .01)
end

local Nonspell1 = {
	HP = 4000,
	Time = 30,
	Type = PHASE_NONSPELL,
	Script = function(id)
		local xstart = BossGetX(id)
		local ystart = BossGetY(id)

		launch_wherever(id)
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
