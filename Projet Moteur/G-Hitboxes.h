#pragma once
#include <DirectXCollision.h>

using namespace DirectX;

struct CollisionBox
{
	BoundingOrientedBox obox;
	ContainmentType collision;
};


