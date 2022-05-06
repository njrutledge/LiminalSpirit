//
//  Spawner.cpp
//  LiminalSpirit
//
//  Created by Chenyu Wei on 4/18/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "Spawner.hpp"

EnemyProperties SPAWNER_PROPS{
    /** Starting health of the spawner */
    360,
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
    /** Damage of the spawner */
    9001,
    /** Name of the spawner*/
    "Spawner",
};
