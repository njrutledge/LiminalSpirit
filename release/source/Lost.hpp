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

#define LOST_ATTACK "lost_attack"

#pragma mark - 
#pragma mark Lost

extern struct EnemyProperties LOST_PROPS;

class Lost : public BaseEnemyModel {
	public:
		/** Inheriting all constructors exactly as is since lost does not need more initializers*/
		using BaseEnemyModel::BaseEnemyModel;

#pragma mark - 
#pragma mark Static Constructors
		/** Allocates a new lost */
		static std::shared_ptr<Lost> alloc(const cugl::Vec2& pos, const cugl::Size& realSize, const cugl::Size& size, float scale) {
			std::shared_ptr<Lost> result = std::make_shared<Lost>();
			return (result->init(pos, realSize, size, scale, LOST_PROPS) ? result : nullptr);
		}

};



#endif