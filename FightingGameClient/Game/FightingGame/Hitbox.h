#pragma once
#include <glm/glm.hpp>

struct Hitbox
{
	enum HitboxLayer
	{
		Push = 1,
		Hit = 2,
		Hurt = 4
	};
	
	Hitbox() = default;

	Hitbox(float width, float height, glm::vec3 pos, HitboxLayer layer) :
		width_(width), height_(height), pos_(pos), layer_(layer)	{}
	float width_;
	float height_;
	glm::vec3 pos_; //BASED ON IF FIGHTER IS ON RIGHT SIDE.  POS IS FLIPPED IF THE FIGHTER IS ON THE LEFT.  Always assign this value assuming the right side
	std::vector<Hitbox> children_;
	HitboxLayer layer_ = Push;
};
