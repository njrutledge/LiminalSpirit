//
//  Lost.hpp
//  Liminal Spirit Game
//
//  This class is the basic grounded enemy
// 
//

#ifndef __LOST_HPP__
#define __LOST_HPP__

#include <cugl/cugl.h>
#include "BaseEnemyModel.h"

#pragma mark -
#pragma mark Attack Constants
/** The range from the center of the Lost to the end of its attack hitbox */
#define ATTACK_RADIUS 5.0f // needs to be adjusted based on attack hitbox

/** Starting health of the lost */
#define MAX_HEALTH 10

/** Vertical speed of the lost */
#define VERTICAL_SPEED 0.0f

/** Horizontal speed of the lost*/
#define HORIZONTAL_SPEED 4.0f

/** Cooldown for attack(s) in frames*/
#define ATTACK_COOLDOWN 360

#pragma mark - 
#pragma mark Lost

class Lost : public BaseEnemyModel {
	public:
		/** Inheriting all constructors exactly as is since lost does not need more initializers*/
		using BaseEnemyModel::BaseEnemyModel;

		/** Returns the attack radius of the Lost*/
		float getAttackRadius() { return ATTACK_RADIUS; }

#pragma mark - 
#pragma mark Static Constructors
		/** Allocates a new lost */
		static std::shared_ptr<Lost> alloc(const cugl::Vec2& pos, const cugl::Size& size, float scale) {
			std::shared_ptr<Lost> result = std::make_shared<Lost>();
			return (result->init(pos, size, scale, MAX_HEALTH, VERTICAL_SPEED, HORIZONTAL_SPEED, ATTACK_COOLDOWN) ? result : nullptr);
		}

};



#endif