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
#include "Lost.hpp"

class AIController {
	enum States {
		Finding,
		Attacking,
		Stopped,
	};
public:
	/** Creates a basic empty AI Controller*/
	AIController();

	/** Degenerate way to get movement (for base enemy class) used for when AI is not implemented for an enemy yet */
	float getMovement(shared_ptr<BaseEnemyModel> e, cugl::Vec2 player_pos);

	/** Takes in a lost enemy and returns its movement based on the pathfinding towards the player's position */
	float getLostMovement(shared_ptr<BaseEnemyModel> lost, cugl::Vec2 player_pos);

	/** Remove any unwanted assets and clean up */
	void dispose() {};

};



#endif