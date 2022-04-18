//
//  spawner.cpp
//  LiminalSpirit
//
//  Created by Chenyu Wei on 4/11/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "spawner.hpp"

EnemyProperties SPAWNER_PROPS{
    /** Starting health of the spawner */
    10,
    /** Vertical speed of the spawner */
    0.0f,
    /** Horizontal speed of the spawner*/
    0.0f,
    /** Cooldown for attack(s) in seconds*/
    2,
    /** Attack radius of the spawner*/
    0.0f,
    /** Density of the spawner*/
    1.0f,
    /** Name of the spawner*/
    "Spawner",
};
