--
--				spellcards hp & kill time
--
--					1500	1500	1500	1500	1500
--	Reimu@full:		0:10	0:10	0:10	0:10	0:10
--

local th = require("touhou")

local sprCirnoIdle  = LoadSprite("cirnoidle.png", 1, 0, 0)
local sprCirnoGlide = LoadSprite("cirnoglide.png", 1, 0, 0)
local sprCirnoFlap  = LoadSprite("cirnoflap.png", 4, .25, 0)
local texBullet     = LoadTexture("bullet.png")
local texPellet     = LoadTexture("pellet.png")

local BULLET_TEX = texBullet
local BULLET_RAD = 3
local BULLET_ROT = false
local PELLET_TEX = texPellet
local PELLET_RAD = 2
local PELLET_ROT = true

local function wander(id)
	local x = th.random_range(0, PLAY_AREA_W)
	local y = th.random_range(0, PLAY_AREA_H / 3)
	local bx = BossGetX(id)
	local by = BossGetY(id)
	x = th.clamp(x, bx-80, bx+80)
	y = th.clamp(y, by-80, by+80)
	th.BossLaunchTowardsPoint(id, x, y, .01)
end

local function go_back(id)
	th.BossLaunchTowardsPoint(id, BOSS_STARTING_X, BOSS_STARTING_Y, .02)
end

local Nonspell1 = {
	HP = 1500,
	Time = 25,
	Type = PHASE_NONSPELL,
	Script = function(id)
		local function shoot_radial_bullets()
			return th.ShootRadial(17, 360/17, function()
				return th.BossShootAtPlr(id, 3.5, 0, BULLET_TEX, BULLET_RAD, BULLET_ROT)
			end)
		end

		while true do
			BossSetSpr(id, sprCirnoFlap)
			for i = 1, 3 do
				for i = 0, 5 do
					th.ShootRadial(7-i, 5, function()
						return th.BossShootAtPlr(id, th.lerp(4, 7.5, i/5), 0, PELLET_TEX, PELLET_RAD, PELLET_ROT)
					end)
				end

				th.wait(60)
			end

			th.wait(15)

			wander(id)
			BossSetSpr(id, sprCirnoGlide)

			th.wait(15)

			for i = 1, 3 do
				shoot_radial_bullets()

				th.wait(15)

				local b = th.ShootRadial(17, 360/17, function()
					return th.BossShootAtPlr(id, 4.5, -.08, PELLET_TEX, PELLET_RAD, PELLET_ROT)
				end)

				th.wait(15)

				shoot_radial_bullets()

				th.wait(25)

				for i = 1, #b do
					BltSetAcc(b[i], 0)
					BltSetSpd(b[i], 5)
					BltSetDir(b[i], th.BltDirToPlr(b[i]))
				end

				th.wait(25)

				if i == 3 then
					go_back(id)
				else
					wander(id)
				end
			end

			th.wait(60)
		end
	end
}

local IcicleFall = {
	HP = 1500,
	Time = 30,
	Type = PHASE_SPELLCARD,
	Name = "Ice Sign \"Icicle Fall\"",
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
							local b = Shoot(x, y, 0, dir, 0, PELLET_TEX, PELLET_RAD, PELLET_ROT)
							local target_x = x + th.lengthdir_x(100 + 90 * j, dir)
							local target_y = y + th.lengthdir_y(100 + 90 * j, dir)
							th.BltLaunchTowardsPoint(b, target_x, target_y, .07)
							bullets[#bullets + 1] = b
						end
					end

					if (i % 3) == 2 then
						th.ShootRadial(5, 20, function()
							return th.BossShootAtPlr(id, 2, 0, BULLET_TEX, BULLET_RAD, BULLET_ROT)
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
						BltSetSpd(b, 2)
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
				error("error in IcicleFall f1: " .. msg)
			end

			result, msg = coroutine.resume(co2)
			if not result then
				error("error in IcicleFall f2: " .. msg)
			end

			th.wait(1)
		end
	end
}

local Nonspell2 = {
	HP = 1500,
	Time = 50,
	Type = PHASE_NONSPELL,
	Script = function(id)
		while true do
			for i = 1, 16 do
				if (i % 2) == 0 then
					th.ShootRadial(8, 360/8, function()
						local b =  th.BossShootAtPlr(id, 4, 0, BULLET_TEX, BULLET_RAD, BULLET_ROT)
						BltSetDir(b, BltGetDir(b) + 360/8 / 2)
						return b
					end)
				else
					th.ShootRadial(8, 360/8, function()
						return th.BossShootAtPlr(id, 2, 0, BULLET_TEX, BULLET_RAD, BULLET_ROT)
					end)
				end

				th.wait(10)
			end

			th.wait(60)
		end
	end
}

local PerfectFreeze = {
	HP = 1500,
	Time = 40,
	Type = PHASE_SPELLCARD,
	Name = "Freeze Sign \"Perfect Freeze\"",
	Script = function(id)
		while true do
			local bullets = {}

			wander(id)

			for i = 1, 100 do
				for i = 1, 2 do
					bullets[#bullets+1] = Shoot(
						BossGetX(id), BossGetY(id),
						th.random_range(1, 4), th.random_range(0, 360), 0,
						BULLET_TEX, BULLET_RAD, BULLET_ROT
					)
				end

				th.wait(1)
			end

			th.wait(60)

			for i = 1, #bullets do
				BltSetSpd(bullets[i], 0)
			end

			th.wait(60)

			wander(id)

			for i = 1, 5 do
				for i = 0, 4 do
					th.ShootRadial(4, 30, function()
						return th.BossShootAtPlr(id, th.lerp(2, 6, i/4), 0, BULLET_TEX, BULLET_RAD, BULLET_ROT)
					end)
				end
				th.wait(10)
			end

			th.wait(60)

			for i = 1, #bullets do
				local b = bullets[i]
				BltSetSpd(b, 0)
				BltSetDir(b, th.random_range(0, 360))
				BltSetAcc(b, th.random_range(.01, .015))
			end

			th.wait(180)
		end
	end
}

local DiamondBlizzard = {
	HP = 1500,
	Time = 33,
	Type = PHASE_SPELLCARD,
	Name = "Snow Sign \"Diamond Blizzard\"",
	Script = function(id)
		while true do
			for i = 1, 30 do
				local bullets = {}

				local x = BossGetX(id) + th.random_range(-50, 50)
				local y = BossGetY(id) + th.random_range(-50, 50)

				local n = th.random_range(4, 6)
				for i = 1, n do
					bullets[i] = Shoot(x, y, th.random_range(4, 5), th.random_range(0, 360), 0, PELLET_TEX, PELLET_RAD, PELLET_ROT)
				end

				th.wait(4)

				for i = 1, n do
					BltSetSpd(bullets[i], th.random_range(1, 2))
				end
			end

			wander(id)
		end
	end
}

local Cirno = {
	Name = "Cirno",
	Sprite = sprCirnoIdle,
	Phases = {Nonspell1, IcicleFall, Nonspell2, PerfectFreeze, DiamondBlizzard},
	Healthbars = {1, 3}
}

local Stage = {
	Music = "02. mujaki saheno uwagaki.mp3",
	Script = function()
		CreateBoss(Cirno)
	end
}

return Stage
