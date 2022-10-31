---@class touhoulib
local th = {}



---@param x number
---@return number
function th.dsin(x) return math.sin(math.rad(x)) end

---@param x number
---@return number
function th.dcos(x) return math.cos(math.rad(x)) end

---@param l number
---@param d number
---@return number
function th.lengthdir_x(l, d) return l*th.dcos(d) end

---@param l number
---@param d number
---@return number
function th.lengthdir_y(l, d) return l*-th.dsin(d) end

---@param x number
---@param min number
---@param max number
---@return number
function th.clamp(x, min, max) return math.min(math.max(x, min), max) end

---@param a number
---@param b number
---@param n number
---@return number
function th.approach(a, b, n) return a + th.clamp(b-a, -n, n) end

---@param x number
---@return number
function th.sqr(x) return x*x end

---@param x1 number
---@param y1 number
---@param x2 number
---@param y2 number
---@return number
function th.point_direction(x1, y1, x2, y2) return math.deg(math.atan(y1-y2, x2-x1)) end

---@param x1 number
---@param y1 number
---@param x2 number
---@param y2 number
---@return number
function th.point_distance(x1, y1, x2, y2) return math.sqrt(th.sqr(x2-x1) + th.sqr(y2-y1)) end

---@param a number
---@param b number
---@param f number
---@return number
function th.lerp(a, b, f) return a + (b-a)*f end

---@param x number
---@return number
function th.round(x) return x + (2^52 + 2^51) - (2^52 + 2^51) end

---@param x number
---@param n number
---@return number
function th.floor_to(x, n) return math.floor(x/n) * n end

---@param x number
---@param n number
---@return number
function th.round_to(x, n) return th.round(x/n) * n end

---@param x number
---@param n number
---@return number
function th.ceil_to(x, n) return math.ceil(x/n) * n end

---@param a number
---@param b number
---@return number
function th.wrap(a, b) return (a%b + b) % b end

---@param a number
---@param b number
---@return number
function th.random_range(a, b)
	return a + (b-a) * Random()
end

---@param ... any
---@return any
function th.choose(...)
	local arg = {...}
	local i = math.floor(Random() * #arg) + 1
	return arg[i]
end



---@param t integer
function th.wait(t)
	while t > 0 do
		coroutine.yield()
		t = t - 1
	end
end



---@param id instance_id
---@param spd number
---@param acc number
---@param texture texture_handle | -1
---@param radius number
---@param rotate boolean
---@return instance_id
function th.BossShootAtPlr(id, spd, acc, texture, radius, rotate)
	local x = BossGetX(id)
	local y = BossGetY(id)
	local dir = th.point_direction(x, y, PlrGetX(), PlrGetY())
	return Shoot(x, y, spd, dir, acc, texture, radius, rotate)
end

---@param id instance_id
---@param spd number
---@param acc number
---@param texture texture_handle | -1
---@param length number
---@param thickness number
---@return instance_id
function th.BossShootLazerAtPlr(id, spd, acc, texture, length, thickness)
	local x = BossGetX(id)
	local y = BossGetY(id)
	local dir = th.point_direction(x, y, PlrGetX(), PlrGetY())
	return ShootLazer(x, y, spd, dir, acc, texture, length, thickness)
end

---@param id instance_id
---@param spd number
---@param acc number
---@param texture texture_handle | -1
---@param radius number
---@return instance_id
function th.BossShootRectAtPlr(id, spd, acc, texture, radius)
	local x = BossGetX(id)
	local y = BossGetY(id)
	local dir = th.point_direction(x, y, PlrGetX(), PlrGetY())
	return ShootRect(x, y, spd, dir, acc, texture, radius)
end

---@param count integer
---@param dir_diff number
---@param lambda fun(): instance_id | array<instance_id>
---@return array<instance_id>
function th.ShootRadial(count, dir_diff, lambda)
	local blts = {}
	for i = 0, count-1 do
		local f = -(count - 1) / 2 + i
		local res = lambda()
		if type(res)=="table" then
			for i = 1, #res do
				blts[#blts+1] = res[i]
				BltSetDir(blts[#blts], BltGetDir(blts[#blts]) + dir_diff * f)
			end
		else
			blts[#blts+1] = res
			BltSetDir(blts[#blts], BltGetDir(blts[#blts]) + dir_diff * f)
		end
	end
	return blts
end

---@param b instance_id
---@return number
function th.BltDirToPlr(b)
	return th.point_direction(BltGetX(b), BltGetY(b), PlrGetX(), PlrGetY())
end

---@param id instance_id
---@param target_x number
---@param target_y number
---@param acc number
function th.BossLaunchTowardsPoint(id, target_x, target_y, acc)
	acc = math.abs(acc)
	local x = BossGetX(id)
	local y = BossGetY(id)
	local dist = th.point_distance(x, y, target_x, target_y)
	BossSetSpd(id, math.sqrt(dist * acc * 2))
	BossSetAcc(id, -acc)
	BossSetDir(id, th.point_direction(x, y, target_x, target_y))
end

---@param id instance_id
function th.BossWander(id)
	local dir = th.random_range(0, 360)
	local target_x = BossGetX(id) + th.lengthdir_x(50, dir)
	local target_y = BossGetY(id) + th.lengthdir_y(50, dir)
	th.BossLaunchTowardsPoint(id, target_x, target_y, .01)
end

---@param id instance_id
---@param target_x number
---@param target_y number
---@param acc number
function th.BltLaunchTowardsPoint(id, target_x, target_y, acc)
	acc = math.abs(acc)
	local x = BltGetX(id)
	local y = BltGetY(id)
	local dist = th.point_distance(x, y, target_x, target_y)
	BltSetSpd(id, math.sqrt(dist * acc * 2))
	BltSetAcc(id, -acc)
	BltSetDir(id, th.point_direction(x, y, target_x, target_y))
end



return th
