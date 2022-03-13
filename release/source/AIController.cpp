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

// Gets movement for the Lost enemy based on the player position
float AIController::getMovement(shared_ptr<Lost> lost, Vec2 player_pos) {
	//TODO: - check if grounded -> don't move if falling (unless flying enemy)
	// - set states for the enemy -> more interesting ai

	//Check if enemy is already attacking
	if (!lost->isAttacking()) {
		// Check if in range to attack (checking closer than max range so enemy has an actual chance to hit)
		if (player_pos.x <= lost->getPosition().x + lost->getAttackRadius() / 2 
			&& player_pos.x >= lost->getPosition().x - lost->getAttackRadius() / 2 
			&& player_pos.y <= lost->getPosition().y + lost->getAttackRadius() / 4 //Likely exists a better way to check height is similar but this will do for now
			&& player_pos.y >= lost->getPosition().y - lost->getAttackRadius() / 4) {
			lost->setIsAttacking(true);
			return 0; // lost stops moving
		}
		else if (player_pos.x > lost->getPosition().x) {
			return lost->getHorizontalSpeed();
		}
		else {
			return -1 * lost->getHorizontalSpeed();
		}
	}
	else {
		// Check if attack timer should be reset
		if (lost->getAttackCooldown() < lost->getFramesPast()) {
			lost->setIsAttacking(false);
			lost->setFramesPast(0);
		}
		else {
			lost->setFramesPast(lost->getFramesPast() + 1);
		}
		return 0; 
	}
}