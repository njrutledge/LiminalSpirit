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

Vec2 AIController::getMovement(shared_ptr<BaseEnemyModel> e, Vec2 player_pos, float timestep) {
	std::string name = e->getName();
	if (name == "Lost") {
		return Vec2(getLostMovement(e, player_pos, timestep), 0.0f);
	} else if (name == "Specter") {
		return getSpecterMovement(e, player_pos, timestep);
	}
	else {
		return Vec2();
	}
}

float AIController::getLostMovement(shared_ptr<BaseEnemyModel> lost, Vec2 player_pos, float timestep) {
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
		if (lost->getAttackCooldown() < lost->getTimePast()) {
			lost->setIsAttacking(false);
			lost->setTimePast(0.0f);
		}
		else {
			lost->setTimePast(lost->getTimePast() + timestep);
		}
		return 0; 
	}
}

Vec2 AIController::getSpecterMovement(shared_ptr<BaseEnemyModel> specter, Vec2 player_pos, float timestep) {
	//TODO: Add vertical movement
	// Use line of sight to determine ranged attacks

	//Check if enemy is already attacking
	if (!specter->isAttacking()) {
		if (player_pos.x <= specter->getPosition().x + specter->getAttackRadius()
			&& player_pos.x >= specter->getPosition().x - specter->getAttackRadius()
			&& player_pos.y <= specter->getPosition().y + specter->getAttackRadius()
			&& player_pos.y >= specter->getPosition().y - specter->getAttackRadius()) {
			specter->setIsAttacking(true);
			return Vec2(); // lost stops moving
		}
		else if (player_pos.x > specter->getPosition().x) {
			return Vec2(specter->getHorizontalSpeed(), 0);
		}
		else {
			return Vec2(- 1 * specter->getHorizontalSpeed(), 0);
		}
	}
	else {
		// Check if attack timer should be reset
		if (specter->getAttackCooldown() < specter->getTimePast()) {
			specter->setIsAttacking(false);
			specter->setTimePast(0.0f);
		}
		else {
			specter->setTimePast(specter->getTimePast() + timestep);
		}
		return Vec2();
	}
}