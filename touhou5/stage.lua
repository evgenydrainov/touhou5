local th = require('touhou')

local Nonspell1 = {
	Type = 0,
	HP = 1000,
	Time = 60,
	Script = function(id)
		
	end
}

local Boss = {
	Name = 'Name',
	Phases = {Nonspell1},
	Healthbars = {{1}},
	V = ''
}

local Stage = {
	Script = function(id)
		print('hi')
		
		th.wait(10)
		
		--local shoot = function()
		--	local b = {}
		--	
		--	local i
		--	for i = 0, 9 do
		--		b[#b+1] = CreateBullet(384/2, 448/2, 0, .1, i/10*360, 3)
		--	end
		--	
		--	th.wait(10)
		--	
		--	for i = 1, #b do
		--		BulletSetAcc(b[i], 0)
		--		BulletSetDir(b[i], BulletGetDir(b[i]) + 90)
		--	end
		--end
		--
		--local i
		--for i = 1, 5 do
		--	shoot()
		--	th.wait(10)
		--end
		--
		--th.wait(10)
		
		CreateBoss(Boss, 384/2, 448/2, 10)
		
		th.wait(10)
		
		print('end')
	end
}

return Stage
