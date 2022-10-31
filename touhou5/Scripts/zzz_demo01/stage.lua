local th = require('touhou')

local texPellet = LoadTexture('pellet.png')

local Stage = {
	Script = function()
		local dir, acc = 0, 0
		while true do
			local x, y = 384/2, 448/4
			th.ShootRadial(4, 360/4, function()
				return Shoot(x, y, 1, th.point_direction(x, y, PlrGetX(), PlrGetY()) + dir, 0, texPellet, 2, true)
			end)
			acc = acc + .05
			dir = dir + acc
			th.wait(1)
		end
	end
}

return Stage
