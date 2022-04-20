//
//  Glutton.hpp
//  LiminalSpirit
//
//  Created by Jonathan Gomez on 3/25/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef Glutton_hpp
#define Glutton_hpp

#include <stdio.h>

#include <cugl/cugl.h>
#include "BaseEnemyModel.h"

#define GLUTTON_ATTACK "glutton_projectile"
#define GLUTTON_FRAMES 4

#pragma mark -
#pragma mark Glutton

extern struct EnemyProperties GLUTTON_PROPS;

class Glutton : public BaseEnemyModel {
    public:
        /** Inheriting all constructors exactly as is since glutton does not need more initializers*/
        using BaseEnemyModel::BaseEnemyModel;

#pragma mark -
#pragma mark Static Constructors
        /** Allocates a new Glutton */
        static std::shared_ptr<Glutton> alloc(const cugl::Vec2& pos, const cugl::Size& realSize, const cugl::Size& size, float scale) {
            std::shared_ptr<Glutton> result = std::make_shared<Glutton>();
            return (result->init(pos, realSize, size, scale, GLUTTON_PROPS) ? result : nullptr);
        }

};

#endif /* Glutton_hpp */
