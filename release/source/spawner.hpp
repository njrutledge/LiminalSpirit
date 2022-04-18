//
//  spawner.hpp
//  LiminalSpirit
//
//  Created by Chenyu Wei on 4/11/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef spawner_hpp
#define spawner_hpp

#include <stdio.h>

#include <cugl/cugl.h>
#include "BaseEnemyModel.h"

#pragma mark -
#pragma mark Spawner

extern struct EnemyProperties SPAWNER_PROPS;

class Spawner : public BaseEnemyModel {
    private:
        float _timer;
    public:
        using BaseEnemyModel::BaseEnemyModel;

#pragma mark -
#pragma mark Static Constructors
        /** Allocates a new spawner */
        static std::shared_ptr<Spawner> alloc(const cugl::Vec2& pos, const cugl::Size& size, float scale) {
            std::shared_ptr<Spawner> result = std::make_shared<Spawner>();
            return (result->init(pos, size, scale, SPAWNER_PROPS) ? result : nullptr);
        }
    
    float getTimer() {return _timer;};
    void setTimer(float time) {_timer = time;};
};
#endif /* spawner_hpp */
