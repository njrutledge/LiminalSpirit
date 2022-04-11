//
//  Fantome.hpp
//  LiminalSpirit
//
//  Created by Janice Wei on 3/27/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef __FANTOME_HPP__
#define __FANTOME_HPP__

#include <cugl/cugl.h>
#include "BaseEnemyModel.h"


#pragma mark -
#pragma mark Fantome

extern struct EnemyProperties FANTOME_PROPS;

class Fantome : public BaseEnemyModel {
public:
    /** Inheriting all constructors exactly as is since specter does not need more initializers*/
    using BaseEnemyModel::BaseEnemyModel;

#pragma mark -
#pragma mark Static Constructors
    /** Allocates a new lost */
    static std::shared_ptr<Fantome> alloc(const cugl::Vec2& pos, const cugl::Size& size, float scale) {
        std::shared_ptr<Fantome> result = std::make_shared<Fantome>();
        return (result->init(pos, size, scale, FANTOME_PROPS) ? result : nullptr);
    }

    float velScale = 2.5;
    cugl::Vec2 targetPosition;
    bool justAttacked = false;

};

#endif /* Fantome_hpp */
