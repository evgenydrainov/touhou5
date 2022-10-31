local th = require("touhou")

local Stage = {
	Script = function()
		print("enable hitboxes!")

		th.wait(60)

		print("bullets test...")

		th.wait(60)

		for i = 1, 2 do
			for i = -2, 2 do
				Shoot(BOSS_STARTING_X, BOSS_STARTING_Y, 1, 270 + i * 10, 0, -1, 3, false)

				th.wait(10)
			end
		end

		print("lazers test...")

		th.wait(60)

		for i = 1, 2 do
			for i = -2, 2 do
				ShootLazer(BOSS_STARTING_X, BOSS_STARTING_Y, 1, 270 + i * 10, 0, -1, 50, 10)

				th.wait(10)
			end

			th.wait(30)
		end

		print("rect bullets test...")

		th.wait(60)

		for r = 3, 6, 3 do
			for i = -2, 2 do
				ShootRect(BOSS_STARTING_X, BOSS_STARTING_Y, 1, 270 + i * 10, 0, -1, r)

				th.wait(10)
			end
		end
	end
}

return Stage
