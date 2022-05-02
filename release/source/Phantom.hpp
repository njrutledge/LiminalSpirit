//
//  Phantom.hpp
//  Liminal Spirit Game
//
//  This class is the flying ranged enemy
// 
//

#ifndef __PHANTOM_HPP__
#define __PHANTOM_HPP__

#include <cugl/cugl.h>
#include "BaseEnemyModel.h"

#define PHANTOM_ATTACK "phantom_projectile"
#define PHANTOM_FRAMES 2

#pragma mark - 
#pragma mark Phantom

extern struct EnemyProperties PHANTOM_PROPS;

class Phantom : public BaseEnemyModel {
public:
	/** Inheriting all constructors exactly as is since phantom does not need more initializers*/
	using BaseEnemyModel::BaseEnemyModel;

#pragma mark - 
#pragma mark Static Constructors
	/** Allocates a new lost */
	static std::shared_ptr<Phantom> alloc(const cugl::Vec2& pos, const cugl::Size& realSize, const cugl::Size& size, float scale) {
		std::shared_ptr<Phantom> result = std::make_shared<Phantom>();
		return (result->init(pos, realSize, size, scale, PHANTOM_PROPS) ? result : nullptr);
	}
    cugl::Vec2 targetPosition = getPosition();

};



#endif
