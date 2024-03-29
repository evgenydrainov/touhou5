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

	inline float angle_difference(float dest, float src)
	{
		float res = dest - src;
		res = angle_wrap(res + 180.0f) - 180.0f;
		return res;
	}

	inline bool circle_vs_rotated_rect(float circle_x, float circle_y, float circle_radius, float rect_center_x, float rect_center_y, float rect_w, float rect_h, float rect_dir)
	{
		float dx = circle_x - rect_center_x;
		float dy = circle_y - rect_center_y;

		float x_rotated = rect_center_x - (dx * dsin(rect_dir)) - (dy * dcos(rect_dir));
		float y_rotated = rect_center_y + (dx * dcos(rect_dir)) - (dy * dsin(rect_dir));

		float x_closest = std::clamp(x_rotated, rect_center_x - rect_w / 2.0f, rect_center_x + rect_w / 2.0f);
		float y_closest = std::clamp(y_rotated, rect_center_y - rect_h / 2.0f, rect_center_y + rect_h / 2.0f);

		dx = x_closest - x_rotated;
		dy = y_closest - y_rotated;

		return (dx * dx + dy * dy) < (circle_radius * circle_radius);
	}
}
