local th = require('touhou')

local texCirno = LoadTexture('cirno.png')
local texBullet = LoadTexture('bullet.png')

local function EnemyShootTarget(id, spd, acc, texture, radius)
	local x = EnemyGetX(id)
	local y = EnemyGetY(id)
	local t = EnemyGetTarget(id)
	local dir = th.point_direction(x, y, PlayerGetX(t), PlayerGetY(t))
	return CreateBullet(x, y, spd, acc, dir, texture, radius)
end

local Enemy1 = {
	HP = 100,
	Texture = texCirno,
	Script = function(id)
		EnemySetSpd(id, 1)
		EnemySetDir(id, 270)
		th.wait(60)
		EnemySetSpd(id, 0)
		for i = 1, 3 do
			local x = EnemyGetX(id)
			local y = EnemyGetY(id)
			local t = EnemyGetTarget(id)
			local dir = th.point_direction(x, y, PlayerGetX(t), PlayerGetY(t))
			CreateBullet(x, y, 5, 0, dir, texBullet, 3)
			th.wait(10)
		end
		th.wait(60)
		EnemySetSpd(id, 5)
		EnemySetDir(id, 90)
	end
}

local Enemy2 = {
	HP = 100,
	Texture = texCirno,
	Script = function(id)
		EnemySetSpd(id, 1)
		EnemySetDir(id, 270)
		while true do
			EnemyShootTarget(id, 5, 0, texBullet, 3)
			th.wait(60)
		end
	end
}

local Nonspell1 = {
	HP = 1000,
	Time = 30,
	Script = function(id)
		
	end
}

local Boss = {
	Name = 'Name',
	Texture = sprCirno,
	Phases = {Nonspell1}
}

local Stage = {
	Script = function()
		th.wait(30)
		
		CreateEnemy(Enemy2, 50, 0)
	end
}

return Stage
