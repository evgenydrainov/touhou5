local S = {}

function S.dsin(x) return math.sin(math.rad(x)) end

function S.dcos(x) return math.cos(math.rad(x)) end

function S.lengthdir_x(l, d) return l*S.dcos(d) end

function S.lengthdir_y(l, d) return -l*S.dsin(d) end

function S.clamp(x, min, max) return math.min(math.max(v, min), max) end

function S.approach(a, b, n) return a + S.clamp(b-a, -n, n) end

function S.sqr(x) return x*x end

function S.point_direction(x1, y1, x2, y2) return math.deg(math.atan(y1-y2, x2-x1)) end

function S.point_distance(x1, y1, x2, y2) return math.sqrt(S.sqr(x2-x1) + S.sqr(y2-y1)) end

function S.lerp(a, b, f) return a + (b-a)*f end

function S.floor_to(x, n) return math.floor(x/n) * n end

function S.round_to(x, n) return S.round(x/n) * n end

function S.ceil_to(x, n) return math.ceil(x/n) * n end

function S.emod(a, b) return (a%b + b) % b end

function S.wait(t)
	while t>0 do
		coroutine.yield()
		t=t-1
	end
end

return S
