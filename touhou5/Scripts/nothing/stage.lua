local th = require('touhou')

local Stage = {
	Music = 'weed.mod',
	Script = function()
		for i = 1, 10 do
			print('Nothing!')

			th.wait(60 * 60)
		end
	end
}

return Stage
