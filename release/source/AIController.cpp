//
//  AIController.cpp
//  LiminalSpirit
//
//  Created by Sashimi Software on 3/8/22.
//  Copyright � 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "AIController.hpp"

using namespace cugl;

AIController::AIController() {
	// Add initialization variables if needed
}

/** Reset AI Controller */
void AIController::reset() {
    // Add reset variables if needed
};

Vec2 AIController::getMovement(shared_ptr<BaseEnemyModel> e, Vec2 player_pos, float timestep) {
	std::string name = e->getName();
	if (name == "Lost") {
		return Vec2(getLostMovement(e, player_pos, timestep), e->getVY());
	} else if (name == "Specter") {
		return getSpecterMovement(e, player_pos, timestep);
    } else if (name == "Glutton"){
        return Vec2(getGluttonMovement(e, player_pos, timestep), e->getVY());
    }
	else {
		return Vec2();
	}
}

float AIController::getGluttonMovement(shared_ptr<BaseEnemyModel> glutton, Vec2 player_pos, float timestep) {
    glutton->setTimePast(glutton->getTimePast() + timestep);
    if (!glutton->isAttacking()) {
        if (glutton->getAttackCooldown() < glutton->getTimePast()
            && rand() % 100 <= 2.5) {
            glutton->setIsAttacking(true);
            glutton->setTimePast(0.0f);
        } else
            if ((abs(player_pos.x - glutton->getPosition().x) < glutton->getAttackRadius())) {
                if (player_pos.x > glutton->getPosition().x) {
                    return -1* glutton->getHorizontalSpeed();
                }
                else {
                    return glutton->getHorizontalSpeed();
                }
            }
    }
    return 0;
}

float AIController::getLostMovement(shared_ptr<BaseEnemyModel> lost, Vec2 player_pos, float timestep) {
	//TODO: - check if grounded -> don't move if falling (unless flying enemy)
	// - set states for the enemy -> more interesting ai
    lost->setTimePast(lost->getTimePast() + timestep);

	//Check if enemy is already attacking
	if (!lost->isAttacking()) {
		// Check if in range to attack (checking closer than max range so enemy has an actual chance to hit)
		if (player_pos.x <= lost->getPosition().x + lost->getAttackRadius() / 2 
			&& player_pos.x >= lost->getPosition().x - lost->getAttackRadius() / 2 
			&& player_pos.y <= lost->getPosition().y + lost->getAttackRadius() / 4 //Likely exists a better way to check height is similar but this will do for now
			&& player_pos.y >= lost->getPosition().y - lost->getAttackRadius() / 4) {
            if (lost->getAttackCooldown() < lost->getTimePast()) {
                lost->setIsAttacking(true);
                lost->setTimePast(0.0f);
                
            }
            return 0; // lost stops moving
			
		}
		else if (abs(player_pos.y - lost->getPosition().y) > 5 && !lost->getHasSeenPlayer()) {
			//This is jank -> will likely use some from of LoS instead once we can detect platforms and walls
			if ((abs(player_pos.x - lost->getPosition().x) < 5)) {
				if (player_pos.x > lost->getPosition().x) {
					return lost->getHorizontalSpeed(); // TODO: Change to behavior
				}
				else {
					return -1* lost->getHorizontalSpeed();
				}
			}
			else {
				return 0; 
			}
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
		return 0; 
	}
}

Vec2 AIController::getSpecterMovement(shared_ptr<BaseEnemyModel> specter, Vec2 player_pos, float timestep) {
	//TODO: 
	// Use line of sight to determine ranged attacks
    specter->setTimePast(specter->getTimePast() + timestep);
//	int flip = 1; // flips y direction

	//Check if enemy is already attacking
	if (!specter->isAttacking()) {
		if (player_pos.x <= specter->getPosition().x + specter->getAttackRadius()
			&& player_pos.x >= specter->getPosition().x - specter->getAttackRadius()
			&& player_pos.y <= specter->getPosition().y + specter->getAttackRadius()
			&& player_pos.y >= specter->getPosition().y - specter->getAttackRadius()) {
            if (specter->getAttackCooldown() < specter->getTimePast()) {
                specter->setIsAttacking(true);
                specter->setTimePast(0.0f);
            }
			return Vec2(); // Specter stops moving
		}
		else if (specter->getVY() == 0) {
			return Vec2(specter->getHorizontalSpeed(), -1 * specter->getVerticalSpeed());
		}
		else if (player_pos.x > specter->getPosition().x) {
			if (player_pos.y >= specter->getPosition().y) {
				return Vec2(specter->getHorizontalSpeed(), specter->getVerticalSpeed());
			}
			else {
				return Vec2(specter->getHorizontalSpeed(), -1 * specter->getVerticalSpeed());
			}
		}
		else {
			if (player_pos.y >= specter->getPosition().y) {
				return Vec2(-1 * specter->getHorizontalSpeed(), specter->getVerticalSpeed());
			}
			else {
				return Vec2(-1 * specter->getHorizontalSpeed(), -1 * specter->getVerticalSpeed());
			}
		}
	} 
	else {
		// Check if attack timer should be reset
		if (specter->getAttackCooldown() < specter->getTimePast()) {
			specter->setIsAttacking(false);
			specter->setTimePast(0.0f);
		}
		
			
		return Vec2();
	}
}
