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

#pragma mark - 
#pragma mark Lost

class Lost : public BaseEnemyModel {
	public:
		/** Inheriting all constructors exactly as is since lost does not need more initializers*/
		using BaseEnemyModel::BaseEnemyModel;

		/** Returns the attack radius of the Lost*/
		float getAttackRadius() { return ATTACK_RADIUS; }

};



#endif