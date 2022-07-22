local th = require('touhou')

local texCirno = LoadTexture('cirno.png')
local texBullet = LoadTexture('bullet.png')

local Nonspell1 = {
	HP = 4000,
	Time = 25,
	Script = function()
		local function shoot_radial_bullets()
			return th.ShootRadial(17, 360/17, function()
				return th.BossShootAtPlr(4, 0, texBullet, 3, false)
			end)
		end

		while true do
			for i = 1, 3 do
				for i = 0, 5 do
					th.ShootRadial(2+i, 5, function()
						return th.BossShootAtPlr(th.lerp(7.5, 4, i/5), 0, texBullet, 3, false)
					end)
				end

				th.wait(60)
			end

			th.wait(30)

			for i = 1, 3 do
				shoot_radial_bullets()

				th.wait(10)

				local b = th.ShootRadial(17, 360/17, function()
					return th.BossShootAtPlr(5, -.1, texBullet, 3, false)
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

local IcicleFall = {
	HP = 4000,
	Time = 30,
	Script = function()

	end
}

local Cirno = {
	Name = 'Cirno',
	Texture = texCirno,
	Phases = {Nonspell1, IcicleFall}
}

local Stage = {
	Script = function()
		CreateBoss(Cirno)
	end
}

return Stage
