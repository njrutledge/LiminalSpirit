//
//  Spawner.hpp
//  LiminalSpirit
//
//  Created by Chenyu Wei on 4/18/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef Spawner_hpp
#define Spawner_hpp

#include <stdio.h>
#include <cugl/cugl.h>
#include "BaseEnemyModel.h"

#pragma mark -
#pragma mark Spawner

extern struct EnemyProperties SPAWNER_PROPS;

class Spawner : public BaseEnemyModel {
    private:
        int _index;
    public:
        using BaseEnemyModel::BaseEnemyModel;

#pragma mark -
#pragma mark Static Constructors
        /** Allocates a new spawner */
        static std::shared_ptr<Spawner> alloc(const cugl::Vec2& pos, const cugl::Size& realSize, const cugl::Size& size, float scale) {
            std::shared_ptr<Spawner> result = std::make_shared<Spawner>();
            return (result->init(pos, realSize, size, scale, SPAWNER_PROPS) ? result : nullptr);
        }

    int getIndex() {return _index;};
    void setIndex(int index) {_index = index;};
};

#endif /* Spawner_hpp */
