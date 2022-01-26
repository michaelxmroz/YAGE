#pragma once
#include <cstdint>

struct Vector3
{
	float x, y, z;
};

struct Vector2
{
	float x, y;
};

struct Vertex
{
	Vector3 m_pos;
	Vector2 m_uv;
};