local th = require('touhou')

local texCirno		= LoadTexture('cirno.png')
local texBullet		= LoadTexture('bullet.png')
local texPellet		= LoadTexture('pellet.png')

local Nonspell1 = {
	HP = 4000,
	Time = 25,
	Type = PHASE_NONSPELL,
	Script = function(id)
		local function shoot_radial_bullets()
			return th.ShootRadial(17, 360/17, function()
				return th.BossShootAtPlr(id, 4, 0, texBullet, 3, false)
			end)
		end

		while true do
			for i = 1, 3 do
				for i = 0, 5 do
					th.ShootRadial(2+i, 5, function()
						return th.BossShootAtPlr(id, th.lerp(7.5, 4, i/5), 0, texPellet, 3, true)
					end)
				end

				th.wait(60)
			end

			th.wait(30)

			for i = 1, 3 do
				shoot_radial_bullets()

				th.wait(10)

				local b = th.ShootRadial(17, 360/17, function()
					return th.BossShootAtPlr(id, 5, -.1, texPellet, 3, true)
				end)

				th.wait(10)

				shoot_radial_bullets()

				th.wait(20)

				for i = 1, #b do
					BltSetAcc(b[i], 0)
					BltSetSpd(b[i], 5)
					BltSetDir(b[i], th.BltDirToPlr(b[i]))
				end

				th.wait(20)
			end

			th.wait(60)
		end
	end
}

local function BltLaunchTowardsPoint(id, target_x, target_y, acc)
	acc = math.abs(acc)
	local x = BltGetX(id)
	local y = BltGetY(id)
	local dist = th.point_distance(x, y, target_x, target_y)
	BltSetSpd(id, math.sqrt(dist * acc * 2))
	BltSetAcc(id, -acc)
	BltSetDir(id, th.point_direction(x, y, target_x, target_y))
end

local IcicleFall = {
	HP = 4000,
	Time = 30,
	Type = PHASE_SPELLCARD,
	Name = 'Ice Sign "Icicle Fall"',
	Script = function(id)
		local bullets = {}

		local f1 = function()
			while true do
				for i = 0, 10 do
					local off = 100 - 5 * i

					for side = 0, 1 do
						local dir = (side == 0) and (270 - off) or (270 + off)

						for j = 0, 2 do
							local x = BossGetX(id)
							local y = BossGetY(id)
							local b = Shoot(x, y, 0, dir, 0, texPellet, 2, true)
							local target_x = x + th.lengthdir_x(100 + 90 * j, dir)
							local target_y = y + th.lengthdir_y(100 + 90 * j, dir)
							BltLaunchTowardsPoint(b, target_x, target_y, .07)
							bullets[#bullets + 1] = b
						end
					end

					if (i % 3) == 2 then
						th.ShootRadial(5, 20, function()
							return th.BossShootAtPlr(id, 2, 0, texBullet, 2, false)
						end)
					end

					th.wait(20)
				end
			end
		end

		local f2 = function()
			while true do
				for i = #bullets, 1, -1 do
					local b = bullets[i]
					if not BltExists(b) then
						table.remove(bullets, i)
					elseif BltGetLifetime(b) > 50 then
						BltSetSpd(b, 1.5)
						BltSetAcc(b, 0)
						local dir = BltGetDir(b)
						if (90 <= dir) and (dir < 270) then
							BltSetDir(b, dir + 90)
						else
							BltSetDir(b, dir - 90)
						end
						table.remove(bullets, i)
					end
				end

				th.wait(1)
			end
		end

		local co1 = coroutine.create(f1)
		local co2 = coroutine.create(f2)

		while true do
			local result, msg

			result, msg = coroutine.resume(co1)
			if not result then
				error('error in IcicleFall f1: '..msg)
			end

			result, msg = coroutine.resume(co2)
			if not result then
				error('error in IcicleFall f2: '..msg)
			end

			th.wait(1)
		end
	end
}

local Nonspell2 = {
	HP = 4000,
	Time = 50,
	Type = PHASE_NONSPELL,
	Script = function(id)
		while true do
			for i = 1, 16 do
				if (i % 2) == 0 then
					th.ShootRadial(8, 360/8, function()
						local b =  th.BossShootAtPlr(id, 5, 0, texBullet, 2, false)
						BltSetDir(b, BltGetDir(b) + 360/8 / 2)
						return b
					end)
				else
					th.ShootRadial(8, 360/8, function()
						return th.BossShootAtPlr(id, 2, 0, texBullet, 2, false)
					end)
				end

				th.wait(10)
			end

			th.wait(60)
		end
	end
}

local function random(a, b)
	return a + (b - a) * math.random()
end

local PerfectFreeze = {
	HP = 4000,
	Time = 40,
	Type = PHASE_SPELLCARD,
	Name = 'Freeze Sign "Perfect Freeze"',
	Script = function(id)
		while true do
			local bullets = {}

			for i = 1, 100 do
				for i = 1, 2 do
					bullets[#bullets+1] = Shoot(
						BossGetX(id), BossGetY(id),
						random(1, 4), random(0, 360), 0,
						texBullet, 2, false
					)
				end

				th.wait(1)
			end

			th.wait(60)

			for i = 1, #bullets do
				BltSetSpd(bullets[i], 0)
			end

			th.wait(60)

			for i = 1, 6 do
				for i = 1, 3 do
					th.ShootRadial(5, 20, function()
						return th.BossShootAtPlr(id, 4 + i/2, 0, texBullet, 2, true)
					end)
				end
				th.wait(10)
			end

			th.wait(60)

			for i = 1, #bullets do
				local b = bullets[i]
				BltSetSpd(b, 0)
				BltSetDir(b, random(0, 360))
				BltSetAcc(b, random(.01, .015))
			end

			th.wait(180)
		end
	end
}

local DiamondBlizzard = {
	HP = 4000,
	Time = 33,
	Type = PHASE_SPELLCARD,
	Name = 'Snow Sign "Diamond Blizzard"',
	Script = function(id)
		while true do
			local bullets = {}

			local x = BossGetX(id) + random(-50, 50)
			local y = BossGetY(id) + random(-50, 50)

			local n = random(4, 6)
			for i = 1, n do
				bullets[i] = Shoot(x, y, random(4, 5), random(0, 360), 0, texPellet, 2, true)
			end

			th.wait(4)

			for i = 1, n do
				BltSetSpd(bullets[i], random(1, 2))
			end
		end
	end
}

local Cirno = {
	Name = 'Cirno',
	Texture = texCirno,
	Phases = {Nonspell1, IcicleFall, Nonspell2, PerfectFreeze, DiamondBlizzard},
	Healthbars = {1, 3},
	Music = -1
}

local Stage = {
	Music = '02. mujaki saheno uwagaki.mp3',
	Script = function()
		CreateBoss(Cirno)
	end
}

return Stage
