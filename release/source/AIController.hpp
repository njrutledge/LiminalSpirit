//
//  AIController.hpp
//  LiminalSpirit
//
//  Created by Sashimi Software on 3/8/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef __AI_CONTROLLER_H__
#define __AI_CONTROLLER_H__

#include <cugl/cugl.h>
#include "BaseEnemyModel.h"

class AIController {
	enum States {
		Finding,
		Attacking,
		Stopped,
	};
public:
	/** Creates a basic empty AI Controller*/
	AIController();

	/** Takes in an enemy and returns its movement based on the pathfinding towards the player's position */
	float getMovement(shared_ptr<BaseEnemyModel> e, cugl::Vec2 player_pos);

	/** Remove any unwanted assets and clean up */
	void dispose() {};

};



#endif