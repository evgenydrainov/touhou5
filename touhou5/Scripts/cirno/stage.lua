--
--				spellcards hp & kill time
--
--					1500	1500	1500	1500	1500
--	Reimu@full:		0:10	0:10	0:10	0:10	0:10
--

BGCamSetPos(0, 10, 0)
BGCamSetTarget(0, 0, 15)
BGFogSetOrigin(0, 0, 0)
BGFogSetColor(100, 100, 100, 255)
BGFogSetNear(0)
BGFogSetFar(150)
BGFogEnable(true)

local th = require("touhou")



--------------------------------------
-----           CIRNO            -----
--------------------------------------

local sprCirnoIdle  = LoadSprite("cirno idle.png",  1, 0,   0)
local sprCirnoGlide = LoadSprite("cirno glide.png", 1, 0,   0)
local sprCirnoFlap  = LoadSprite("cirno flap.png",  4, .25, 0)

local BULLET_TEX				= LoadTexture("bullet.png")
local BULLET_TEX_YELLOW_FILLED	= LoadTexture("yellow filled bullet.png")
local BULLET_TEX_SMALL			= LoadTexture("small bullet.png")
local BULLET_TEX_2				= LoadTexture("bullet 2.png")
local BULLET_TEX_3				= LoadTexture("bullet 3.png")
local BULLET_TEX_4				= LoadTexture("bullet 4.png")
local BULLET_TEX_5				= LoadTexture("bullet 5.png")
local BULLET_TEX_WHITE			= LoadTexture("white bullet.png")
local PELLET_TEX				= LoadTexture("pellet.png")
local PELLET_TEX_WHITE			= LoadTexture("white pellet.png");
local LAZER_TEX					= LoadTexture("lazer.png")
local BULLET_RAD				= 3
local BULLET_RAD_FILLED			= 4
local BULLET_RAD_SMALL			= 3
local PELLET_RAD				= 2
local BULLET_ROT				= false
local PELLET_ROT				= true

local function wander(id)
	local x = th.random_range(32, PLAY_AREA_W-32)
	local y = th.random_range(32, BOSS_STARTING_Y*2-32)
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
					return th.BossShootAtPlr(id, 4.5, -.08, PELLET_TEX_WHITE, PELLET_RAD, PELLET_ROT)
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

		local function f1()
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
							return th.BossShootAtPlr(id, 2, 0, BULLET_TEX_YELLOW_FILLED, BULLET_RAD_FILLED, BULLET_ROT)
						end)
					end

					th.wait(20)
				end
			end
		end

		local function f2()
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
			wander(id)

			for i = 1, 16 do
				if (i % 2) == 0 then
					th.ShootRadial(8, 360/8, function()
						local b =  th.BossShootAtPlr(id, 4, 0, BULLET_TEX, BULLET_RAD, BULLET_ROT)
						BltSetDir(b, BltGetDir(b) + 360/8 / 2)
						return b
					end)
				else
					th.ShootRadial(8, 360/8, function()
						return th.BossShootAtPlr(id, 2, 0, BULLET_TEX_SMALL, BULLET_RAD_SMALL, BULLET_ROT)
					end)
				end

				th.wait(10)
			end

			wander(id)

			for i = 1, 3 do
				th.ShootRadial(3, 20, function()
					return th.BossShootLazerAtPlr(id, 3.5, 0, LAZER_TEX, 180, 2)
				end)

				th.wait(60)
			end
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
						BossGetX(id),
						BossGetY(id),
						th.random_range(1, 4),
						th.random_range(0, 360), 0,
						th.choose(BULLET_TEX, BULLET_TEX_2, BULLET_TEX_3, BULLET_TEX_4, BULLET_TEX_5),
						BULLET_RAD,
						BULLET_ROT
					)
				end

				th.wait(1)
			end

			th.wait(60)

			for i = 1, #bullets do
				BltSetSpd(bullets[i], 0)
				BltSetTex(bullets[i], BULLET_TEX_WHITE)
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
	Music = "02. mujaki saheno uwagaki.mp3",
	Sprite = sprCirnoIdle,
	Phases = {Nonspell1, IcicleFall, Nonspell2, PerfectFreeze, DiamondBlizzard},
	Healthbars = {1, 3}
}



--------------------------------------
-----          MIDBOSS           -----
--------------------------------------

local sprDaiyousei		= LoadSprite("daiyousei.png", 2, .25, 0)
local KUNAI_TEX_GREEN	= LoadTexture("green kunai.png")
local KUNAI_RAD			= 2
local KUNAI_ROT			= true

local Daiyousei = {
	Sprite = sprDaiyousei,
	Phases = {
		{
			HP = 1000,
			Time = 31,
			Type = PHASE_NONSPELL,
			Script = function(id)
				local homing = {}

				local function tp()
					local x = th.random_range(32, PLAY_AREA_W-32)
					local y = th.random_range(32, BOSS_STARTING_Y*2-32)
					BossSetX(id, x)
					BossSetY(id, y)
				end

				local co = coroutine.create(function()
					while true do
						for j = 1, 2 do
							local n = 48
							for i = 0, n-1 do
								local x = BossGetX(id)
								local y = BossGetY(id)
								local dir
								if j == 1 then
									dir = th.lerp(270+360, 270, i/n)
								else
									dir = th.lerp(270, 270+360, i/n)
								end
								local spd = th.lerp(1.5, 3, i/n)
								local a = th.lerp(1.1, 1.5, i/n)
								Shoot(x, y, spd, dir, 0, KUNAI_TEX_GREEN, KUNAI_RAD, KUNAI_ROT)
								Shoot(x, y, spd*a, dir, 0, KUNAI_TEX_GREEN, KUNAI_RAD, KUNAI_ROT)
								th.wait(1)
							end

							th.wait(100)

							tp()

							th.wait(100)
						end

						for i = 1, 20 do
							th.ShootRadial(3,40,function()
								return th.BossShootAtPlr(id, 2.25, 0, PELLET_TEX, PELLET_RAD, PELLET_ROT)
							end)
							th.ShootRadial(3,40,function()
								homing[#homing+1] = th.BossShootAtPlr(id, 4,     -.1, PELLET_TEX_WHITE, PELLET_RAD, PELLET_ROT)
								homing[#homing+1] = th.BossShootAtPlr(id, 4*1.2, -.1, PELLET_TEX_WHITE, PELLET_RAD, PELLET_ROT)
								return {homing[#homing-1], homing[#homing]}
							end)
							th.wait(10)
						end

						th.wait(100)

						tp()

						th.wait(100)
					end
				end)

				while true do
					coroutine.resume(co)

					for i=#homing,1,-1 do
						if BltGetLifetime(homing[i])>30 then
							BltSetSpd(homing[i],4)
							BltSetAcc(homing[i],0)
							BltSetDir(homing[i],th.BltDirToPlr(homing[i]))
							table.remove(homing,i)
						end
					end

					th.wait(1)
				end
			end
		}
	}
}



---------------------------------------
-----           STAGE             -----
---------------------------------------

local sprSpinner		= LoadSprite("spinner.png", 10, .5,  0)
local sprFairy			= LoadSprite("fairy.png",   8,  .25, 0)
local RICE_TEX_RED		= LoadTexture("red rice.png")
local RICE_RAD			= 2
local RICE_ROT			= true

local Stage = {
	Music = "Hiding you in the cherry trees at night.mp3",
	Script = function()
		th.wait(180)

		local function spawn_spinner()
			local function f(id)
				th.wait(30)
				local r = th.choose(1, 2, 3)
				if r == 1 then
					local dir = th.random_range(0, 360)
					local x = EnmGetX(id)
					local y = EnmGetY(id)
					Shoot(x, y, 2, dir,     0, RICE_TEX_RED, RICE_RAD, RICE_ROT)
					Shoot(x, y, 2, dir+90,  0, RICE_TEX_RED, RICE_RAD, RICE_ROT)
					Shoot(x, y, 2, dir+180, 0, RICE_TEX_RED, RICE_RAD, RICE_ROT)
					Shoot(x, y, 2, dir+270, 0, RICE_TEX_RED, RICE_RAD, RICE_ROT)
				elseif r == 2 then
					local dir = th.random_range(0, 360)
					local x = EnmGetX(id)
					local y = EnmGetY(id)
					Shoot(x, y, 3, dir,     0, KUNAI_TEX_GREEN, KUNAI_RAD, KUNAI_ROT)
					Shoot(x, y, 3, dir+90,  0, KUNAI_TEX_GREEN, KUNAI_RAD, KUNAI_ROT)
					Shoot(x, y, 3, dir+180, 0, KUNAI_TEX_GREEN, KUNAI_RAD, KUNAI_ROT)
					Shoot(x, y, 3, dir+270, 0, KUNAI_TEX_GREEN, KUNAI_RAD, KUNAI_ROT)
				end
				th.wait(30)
				EnmSetAcc(id, -.01)
				while EnmGetSpd(id)>0 do th.wait(1) end
				EnmSetDir(id, -EnmGetDir(id))
				EnmSetAcc(id, .01)
			end

			local x = th.random_range(0, PLAY_AREA_W)
			local y = 0
			return CreateEnemy(x, y, 2, 270 + th.random_range(-30, 30), 0, 10, sprSpinner, f)
		end

		for i = 1, 50 do
			spawn_spinner()
			th.wait(5)
		end

		th.wait(180)

		for j = 1, 4 do
			for i = 1, 18 do
				local x = i * (PLAY_AREA_W / 19)
				if j%2==0 then
					x = PLAY_AREA_W - x
				end
				local y = 0
				CreateEnemy(x, y, 2, th.point_direction(x, y, PlrGetX(), PlrGetY()), 0, 10, sprFairy)
				if i>=12 then
					spawn_spinner()
				end
				th.wait(20)
			end
			th.wait(60)
		end

		th.wait(180)

		---
		--- MIDBOSS
		---

		do
			local dai = CreateMidboss(Daiyousei)

			while BossExists(dai) do
				th.wait(1)
			end
		end

		for i = 1, 250 do
			BGCamSetTarget(0, 0, th.lerp(15, 20, i/250))
			local c = th.lerp(100, 255, i/250)
			BGFogSetColor(c, c, c, 255)

			th.wait(1)
		end

		th.wait(30)

		---
		--- BOSS
		---

		CreateBoss(Cirno)
	end
}

return Stage
