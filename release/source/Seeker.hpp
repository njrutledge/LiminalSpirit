//
//  Seeker.hpp
//  LiminalSpirit
//
//  Created by Janice Wei on 3/27/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef __SEEKER_HPP__
#define __SEEKER_HPP__

#include <cugl/cugl.h>
#include "BaseEnemyModel.h"


#pragma mark -
#pragma mark Seeker

extern struct EnemyProperties SEEKER_PROPS;

class Seeker : public BaseEnemyModel {
public:
    /** Inheriting all constructors exactly as is since phantom does not need more initializers*/
    using BaseEnemyModel::BaseEnemyModel;

#pragma mark -
#pragma mark Static Constructors
    /** Allocates a new seeker */
    static std::shared_ptr<Seeker> alloc(const cugl::Vec2& pos, const cugl::Size& size, float scale) {
        std::shared_ptr<Seeker> result = std::make_shared<Seeker>();
        return (result->init(pos, size, scale, SEEKER_PROPS) ? result : nullptr);
    }

    float velScale = 3.8;
    cugl::Vec2 targetPosition;
    bool justAttacked = false;
    bool stop = false;
    float stopTimer = 0;

};

#endif /* Seeker_hpp */
