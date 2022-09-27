local th = require("touhou")

local sprCirno  = LoadSprite("cirno.png", 1, 0, 0)
local texBullet = LoadTexture("bullet.png")

local DummyNonspell = {
	Type = PHASE_NONSPELL,
	HP = 10000,
	Time = 120,
	Script = function(id)
		while true do
			Shoot(BossGetX(id), BossGetY(id), 3, 270, 0, texBullet, 3, false)

			th.wait(60)
		end
	end
}

local DummySpellCard = {
	Type = PHASE_SPELLCARD,
	Name = "Placeholder Sign ~ Dummy Attack!",
	HP = 10000,
	Time = 120,
	Script = function(id)
		while true do
			Shoot(BossGetX(id), BossGetY(id), 3, 270, 0, texBullet, 3, false)

			th.wait(30)
		end
	end
}

local Dummy = {
	Name = "Dummy",
	Sprite = sprCirno,
	Phases = {DummyNonspell, DummySpellCard, DummyNonspell, DummySpellCard, DummySpellCard},
	Healthbars = {1, 3}
}

local Stage = {
	Script = function()
		th.wait(60)

		CreateBoss(Dummy)
	end
}

return Stage
