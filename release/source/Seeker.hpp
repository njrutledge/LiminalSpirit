//
//  Seeker.hpp
//  LiminalSpirit
//
//  Created by Janice Wei on 3/27/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef __SPECTER_HPP__
#define __SPECTER_HPP__

#include <cugl/cugl.h>
#include "BaseEnemyModel.h"


#pragma mark -
#pragma mark Seeker

extern struct EnemyProperties SEEKER_PROPS;

class Seeker : public BaseEnemyModel {
public:
    /** Inheriting all constructors exactly as is since specter does not need more initializers*/
    using BaseEnemyModel::BaseEnemyModel;

#pragma mark -
#pragma mark Static Constructors
    /** Allocates a new lost */
    static std::shared_ptr<Seeker> alloc(const cugl::Vec2& pos, const cugl::Size& size, float scale) {
        std::shared_ptr<Seeker> result = std::make_shared<Seeker>();
        return (result->init(pos, size, scale, SEEKER_PROPS) ? result : nullptr);
    }

};

#endif /* Seeker_hpp */
