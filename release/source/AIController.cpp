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
		return Vec2(getLostMovement(e, player_pos, timestep), -9.8f);
	} 
	else if (name == "Specter") {
		return getSpecterMovement(e, player_pos, timestep);
	}
	else if (name == "Mirror") {
		return getMirrorMovement(dynamic_cast<Mirror*>(e.get()), player_pos, timestep);
	}
	else if (name == "Seeker") {
		return getSeekerMovement(dynamic_pointer_cast<Seeker>(e), player_pos, timestep);
	} 
	else if (name == "Glutton"){
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
            if (std::sqrt(std::pow(player_pos.x - glutton->getPosition().x, 2) + std::pow(player_pos.y - glutton->getPosition().y, 2)) < glutton->getAttackRadius() / 2) {
                if (player_pos.x > glutton->getPosition().x) {
                    return -1 * glutton->getHorizontalSpeed();
                }
                else {
                    return glutton->getHorizontalSpeed();
                }
            }
    }
    return 0;
}

//(abs(player_pos.x - glutton->getPosition().x) < glutton->getAttackRadius() / 3) &&
//(abs(player_pos.y - glutton->getPosition().y) < glutton->getAttackRadius() / 3)

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

Vec2 AIController::getMirrorMovement(Mirror* mirror, cugl::Vec2 player_pos, float timestep) {
	std::shared_ptr<BaseEnemyModel> linkedEnemy = mirror->getLinkedEnemy();
	//mirror should move based off its linked enemy, or pick a new enemy to link to 
	if (linkedEnemy) {
		Vec2 enemy_pos = linkedEnemy->getPosition();
		//see if we are close enough to be in protecting range DEBUG TRUE RN
		if (true || enemy_pos.distance(mirror->getPosition())<=MIRROR_DISTANCE) {
			//get slope of line between linked enemy and player. Mirror should be on this line at MIRROR_DISTANCE
			Vec2 diff = enemy_pos - player_pos;
			Vec2 targetPoint = enemy_pos - (diff * MIRROR_DISTANCE / (player_pos.distance(enemy_pos)));
			diff = targetPoint - mirror->getPosition();
			if (diff.length() > 1) diff.normalize();
			return Vec2(diff.x * mirror->getHorizontalSpeed(), diff.y * mirror->getVerticalSpeed());
			
		}

	}
	else {
		//TODO
	}
	return Vec2();
}
	

Vec2 AIController::getSeekerMovement(shared_ptr<Seeker> seeker, Vec2 player_pos, float timestep) {
    seeker->setTimePast(seeker->getTimePast() + timestep);
//    int flip = 1; // flips y direction
    //Check if enemy is already attacking
    if (!seeker->isAttacking()) {
        if (player_pos.x <= seeker->getPosition().x + seeker->getAttackRadius()/4
            && player_pos.x >= seeker->getPosition().x - seeker->getAttackRadius()/4
            && player_pos.y <= seeker->getPosition().y + seeker->getAttackRadius()/4
            && player_pos.y >= seeker->getPosition().y - seeker->getAttackRadius()/4) {
            if (seeker->getAttackCooldown() < seeker->getTimePast()) {
                seeker->setIsAttacking(true);
                seeker->justAttacked = true;
                seeker->setTimePast(0.0f);
            }
            return Vec2(); // Seeker stops moving
        }

        if (player_pos.distance(seeker->getPosition())>6 && !seeker->getHasSeenPlayer()) {
            if(seeker->targetPosition.distance(seeker->getPosition()) <= 1) {
                seeker->targetPosition = Vec2(0,0);
                while (seeker->targetPosition.x < 2 || seeker->targetPosition.x > 30 || seeker->targetPosition.y < 2 || seeker->targetPosition.y > 22) {
                    float r = 10 + 10 * std::sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
                    float alpha = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * M_PI;
                    seeker->targetPosition = Vec2(r * std::cos(alpha), r * std::sin(alpha)) + seeker->getPosition();
                }
            }
            return movementHelper(seeker->targetPosition, seeker->getPosition(), seeker->getHorizontalSpeed(), seeker->getVerticalSpeed(), seeker->velScale);
        
        }
        
        else if (player_pos.distance(seeker->getPosition())>4) {
            return movementHelper(player_pos, seeker->getPosition(), seeker->getHorizontalSpeed(), seeker->getVerticalSpeed(), 1);
        }
        else {
            return movementHelper(player_pos, seeker->getPosition(), seeker->getHorizontalSpeed(), seeker->getVerticalSpeed(), 3);
        }
    }
    else {
        // Check if attack timer should be reset
        if (seeker->getAttackCooldown() < seeker->getTimePast()) {
            seeker->setIsAttacking(false);
            seeker->setTimePast(0.0f);
            
        }
        seeker->justAttacked = false;
        return Vec2();
    }
}

Vec2 AIController::movementHelper(Vec2 targetPos, Vec2 enemyPos, float horiSpeed, float vertSpeed, float scale) {
    if (abs(targetPos.x - enemyPos.x) < 0.2) {
        if (targetPos.y >= enemyPos.y ) {
            return Vec2(0, vertSpeed) * scale * sqrt(horiSpeed*horiSpeed + vertSpeed*vertSpeed)/vertSpeed;
        }
        else {
            return Vec2(0, -1 * vertSpeed) * scale * sqrt(horiSpeed*horiSpeed + vertSpeed*vertSpeed)/vertSpeed;
        }
    }
    else if (abs(targetPos.y - enemyPos.y)<0.2) {
        if (targetPos.x >= enemyPos.x ) {
            return Vec2(horiSpeed, 0) * scale * sqrt(horiSpeed*horiSpeed + vertSpeed*vertSpeed)/horiSpeed;
        }
        else {
            return Vec2(-1*horiSpeed, 0) * scale * sqrt(horiSpeed*horiSpeed + vertSpeed*vertSpeed)/horiSpeed;
        }
    }

    else if (targetPos.x > enemyPos.x) {
        if (targetPos.y >= enemyPos.y) {
            return Vec2(horiSpeed, vertSpeed) * scale;
        }
        else {
            return Vec2(horiSpeed, -1 * vertSpeed) * scale;
        }
    }
    else {
        if (targetPos.y >= enemyPos.y) {
            return Vec2(-1 * horiSpeed, vertSpeed) * scale;
        }
        else {
            return Vec2(-1 * horiSpeed, -1 * vertSpeed) * scale;
        }
    }
}
