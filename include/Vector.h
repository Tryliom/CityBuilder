#pragma once

#include <cstdlib>

struct Vector2F
{
	float X, Y;

	Vector2F operator+(Vector2F other) const
	{
		return {X + other.X, Y + other.Y};
	}

	Vector2F operator-(Vector2F other) const
	{
		return {X - other.X, Y - other.Y};
	}

	Vector2F operator*(float other) const
	{
		return {X * other, Y * other};
	}

	Vector2F operator/(float other) const
	{
		return {X / other, Y / other};
	}

	Vector2F& operator+=(Vector2F other)
	{
		X += other.X;
		Y += other.Y;
		return *this;
	}

	Vector2F& operator-=(Vector2F other)
	{
		X -= other.X;
		Y -= other.Y;
		return *this;
	}

	Vector2F& operator*=(float other)
	{
		X *= other;
		Y *= other;
		return *this;
	}

	Vector2F& operator/=(float other)
	{
		X /= other;
		Y /= other;
		return *this;
	}

	Vector2F operator*(Vector2F other) const
	{
		return {X * other.X, Y * other.Y};
	}

	Vector2F& operator*=(Vector2F other)
	{
		X *= other.X;
		Y *= other.Y;
		return *this;
	}

	bool operator==(Vector2F other) const
	{
		return X == other.X && Y == other.Y;
	}

	bool operator!=(Vector2F other) const
	{
		return X != other.X || Y != other.Y;
	}

	[[nodiscard]] float Length() const
	{
		return sqrtf(X * X + Y * Y);
	}

	[[nodiscard]] Vector2F Normalized() const
	{
		float length = Length();
		return {X / length, Y / length};
	}

	[[nodiscard]] Vector2F Normal() const
	{
		return {-Y, X};
	}
};