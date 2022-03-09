//
//  AIController.cpp
//  LiminalSpirit
//
//  Created by Sashimi Software on 3/8/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "AIController.hpp"

using namespace cugl;

AIController::AIController() {
	// Add initialization variables if needed
}

float AIController::getMovement(shared_ptr<BaseEnemyModel> e, Vec2 player_pos) {
	// Basic for testing purposes
	if (player_pos.x > e->getPosition().x) {
		return 1.0f;
	}
	else {
		return -1.0f;
	}
}