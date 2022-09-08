// Cirno's Perfect Math Library

#pragma once

#include <cmath>
#include <algorithm>
#include <numbers>

namespace cpml
{
	constexpr float pi = std::numbers::pi_v<float>;
	constexpr float sqrt2 = std::numbers::sqrt2_v<float>;

	constexpr float sqr(float x)
	{
		return x * x;
	}

	constexpr int emod(int a, int b)
	{
		return (a % b + b) % b;
	}

	constexpr float approach(float start, float end, float shift)
	{
		return start + std::clamp(end - start, -shift, shift);
	}

	constexpr float deg2rad(float degrees)
	{
		return degrees * pi / 180.0f;
	}

	constexpr float rad2deg(float radians)
	{
		return radians * 180.0f / pi;
	}

	constexpr bool circle_vs_circle(float x1, float y1, float r1, float x2, float y2, float r2)
	{
		return sqr(x2 - x1) + sqr(y2 - y1) < sqr(r1 + r2);
	}

	inline float dcos(float degrees)
	{
		return std::cos(deg2rad(degrees));
	}

	inline float dsin(float degrees)
	{
		return std::sin(deg2rad(degrees));
	}

	inline float point_direction(float x1, float y1, float x2, float y2)
	{
		return rad2deg(std::atan2(y1 - y2, x2 - x1));
	}

	inline float point_distance(float x1, float y1, float x2, float y2)
	{
		return std::sqrt(sqr(x2 - x1) + sqr(y2 - y1));
	}

	inline float lengthdir_x(float len, float dir)
	{
		return len * dcos(dir);
	}

	inline float lengthdir_y(float len, float dir)
	{
		return len * -dsin(dir);
	}

	inline float angle_wrap(float deg)
	{
		deg = fmodf(deg, 360.0f);
		if (deg < 0.0f) {
			deg += 360.0f;
		}
		return deg;
	}
}
