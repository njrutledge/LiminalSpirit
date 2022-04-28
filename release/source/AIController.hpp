//
//  AIController.hpp
//  LiminalSpirit
//
//  Created by Sashimi Software on 3/8/22.
//  Copyright � 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef __AI_CONTROLLER_H__
#define __AI_CONTROLLER_H__

#include <cugl/cugl.h>
#include "BaseEnemyModel.h"
#include "Lost.hpp"
#include "Phantom.hpp"
#include "Mirror.hpp"
#include "Seeker.hpp"

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
	cugl::Vec2 getMovement(shared_ptr<BaseEnemyModel> e, cugl::Vec2 player_pos, float timestep, float bottomwall, float worldwidth);

	/** Takes in a lost enemy and returns its movement based on the pathfinding towards the player's position */
	float getLostMovement(shared_ptr<Lost> lost, cugl::Vec2 player_pos, float timestep);
    
    // Gets movement for the glutton based on player position
    float getGluttonMovement(shared_ptr<BaseEnemyModel> glutton, cugl::Vec2 player_pos, float timestep);

	// Gets movement for the phantom based on player position
	cugl::Vec2 getPhantomMovement(shared_ptr<Phantom> phantom, cugl::Vec2 player_pos, float timestep, float bottomwall, float worldwidth);

	//Gets movement for the mirror
	cugl::Vec2 getMirrorMovement(Mirror* mirror, cugl::Vec2 player_pos, float timestep);

	// Gets movement for the seeker based on player position
	cugl::Vec2 getSeekerMovement(shared_ptr<Seeker> seeker, cugl::Vec2 player_pos, float timestep);
	
	cugl::Vec2 movementHelper(cugl::Vec2 targetPos, cugl::Vec2 enemyPos, float horiSpeed, float vertSpeed, float scale);

	/** Remove any unwanted assets and clean up */
	void dispose() {};
    
	/** Reset AI Controller */
	void reset();

};



#endif
