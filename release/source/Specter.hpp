//
//  Specter.hpp
//  Liminal Spirit Game
//
//  This class is the flying ranged enemy
// 
//

#ifndef __SPECTER_HPP__
#define __SPECTER_HPP__

#include <cugl/cugl.h>
#include "BaseEnemyModel.h"


#pragma mark - 
#pragma mark Specter

extern struct EnemyProperties SPECTER_PROPS;

class Specter : public BaseEnemyModel {
public:
	/** Inheriting all constructors exactly as is since specter does not need more initializers*/
	using BaseEnemyModel::BaseEnemyModel;

#pragma mark - 
#pragma mark Static Constructors
	/** Allocates a new lost */
	static std::shared_ptr<Specter> alloc(const cugl::Vec2& pos, const cugl::Size& size, float scale) {
		std::shared_ptr<Specter> result = std::make_shared<Specter>();
		return (result->init(pos, size, scale, SPECTER_PROPS) ? result : nullptr);
	}

};



#endif