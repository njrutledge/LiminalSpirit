//
//  Particle.hpp
//  LiminalSpirit
//
//  Created by Alex Lee on 3/24/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
// 
// This class contains basic information and performs operations on particles in the system.
//

#ifndef __PARTICLE_HPP__
#define __PARTICLE_HPP__

#include <cugl/cugl.h>

using namespace cugl;


class Particle {
public:
	Vec2 position;
	Vec2 velocity;
	Color4 color;
	float size;
	float angle;
	float weight;
	float life;
};

#endif