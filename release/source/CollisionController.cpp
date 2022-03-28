#include "CollisionController.hpp"
#include <box2d/b2_contact.h>
#include "PlayerModel.h"
#include "AttackController.hpp"
using namespace cugl;

/**
	* Processes the start of a collision
	*
	* This method is called when we first get a collision between two objects.
	*
	* @param  contact  The two bodies that collided
	* @param  player   The player pointer
	*/
void CollisionController::beginContact(b2Contact* contact, std::shared_ptr<PlayerModel> player, std::shared_ptr<AttackController> AC) {
	//setup
	b2Fixture* fix1 = contact->GetFixtureA();
	b2Fixture* fix2 = contact->GetFixtureB();

	b2Body* body1 = fix1->GetBody();
	b2Body* body2 = fix2->GetBody();

	std::string* fd1 = reinterpret_cast<std::string*>(fix1->GetUserData().pointer);
	std::string* fd2 = reinterpret_cast<std::string*>(fix2->GetUserData().pointer);

	physics2::Obstacle* bd1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
	physics2::Obstacle* bd2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);

	//handle enemy collision
	if (BaseEnemyModel* enemy = dynamic_cast<BaseEnemyModel*>(bd1)) {
		handleEnemyCollision(enemy, bd2, fd2, AC);
	}
	else if (BaseEnemyModel* enemy = dynamic_cast<BaseEnemyModel*>(bd2)) {
		handleEnemyCollision(enemy, bd1, fd1, AC);
	}

	//handlePlayerCollision
	if (PlayerModel* player = dynamic_cast<PlayerModel*>(bd1)) {
		handlePlayerCollision(player, bd2, fd2);
	}
	else if (PlayerModel* player = dynamic_cast<PlayerModel*>(bd1)) {
		handlePlayerCollision(player, bd1, fd1);

	}
}


/** 
* helper method for handling beginning of enemy collision
*/
void CollisionController::handleEnemyCollision(BaseEnemyModel* enemy, physics2::Obstacle* bd, std::string* fd, std::shared_ptr<AttackController> AC) {
	if (AttackController::Attack* attack = dynamic_cast<AttackController::Attack*>(bd)) {
		if (!enemy) {
			//makes the compiler happy
			return;
		}
		if (!attack->isActive()) {
			return;
		}
		//TODO: Make "playerattacksensor" a constant somewhere
		if (*(attack->getSensorName()) == "playerattacksensor") {
			if (Mirror* mirror = dynamic_cast<Mirror*>(enemy)) {
				if (attack->getType() == AttackController::p_range) {
					//attack->markRemoved();
					attack->setInactive();
					float angle_change;
					cugl::Vec2 linvel = attack->getLinearVelocity().normalize();
					
					switch (mirror->getType()) {
					case Mirror::Type::square:
						//just reflect the attack
						AC->createAttack(attack->getPosition(), attack->getRadius()*MIRROR_AMPLIFY, attack->getMaxAge(),
							attack->getDamage(), AttackController::Type::e_range,
							linvel.rotate(M_PI), false);
						break;
					case Mirror::Type::triangle:
						//reflect three back at you
						linvel.rotate(4 * M_PI / 6);
						angle_change = M_PI / 6.0f;
						for (int i = 0; i < 3; i++) {
							AC->createAttack(attack->getPosition(), attack->getRadius(), attack->getMaxAge(),
								attack->getDamage(), AttackController::Type::e_range,
								linvel.rotate(angle_change)*.66f, false);
						}
						break;
					case Mirror::Type::circle:
						//bullet hell all around!
						angle_change = M_PI / 4;
						for (float i = 0; i < 8; i++) {
							AC->createAttack(attack->getPosition(), attack->getRadius(), attack->getMaxAge(),
								attack->getDamage(), AttackController::Type::e_range,
								linvel.rotate(angle_change)*.5f, false);
						}
						break;
					}
					
				}
				else if (attack->getType() == AttackController::Type::p_melee) {
					mirror->setHealth(mirror->getHealth() - attack->getDamage());
					if (mirror->getHealth() <= 0) {
						mirror->markRemoved(true);
					}
				}
			}
			else{
				enemy->setHealth(enemy->getHealth() - attack->getDamage());
				if (enemy->getHealth() <= 0) {
					enemy->markRemoved(true);
				}
				if (attack->getType() == AttackController::p_exp_package) {
					AC->createAttack(cugl::Vec2(bd->getPosition().x, bd->getPosition().y), 3, 0.1, 9000, AttackController::p_exp, cugl::Vec2::ZERO);
				}
				switch (attack->getType()) {
				case AttackController::p_range:
				case AttackController::p_exp_package:
					attack->setInactive();
					break;
				default:
					break;
				}
			}
		}
		else if (*(attack->getSensorName()) == "enemyattacksensor" && attack->isSplitable()) {
			if (Mirror* mirror = dynamic_cast<Mirror*>(enemy)) {
				attack->setInactive();
				float angle_change;
				cugl::Vec2 linvel = attack->getLinearVelocity().normalize();

				switch (mirror->getType()) {
				case Mirror::Type::square:
					//just amplify the attack
					AC->createAttack(attack->getPosition(), attack->getRadius()*MIRROR_AMPLIFY, attack->getMaxAge(),
						attack->getDamage()*MIRROR_AMPLIFY, AttackController::Type::e_range,
						linvel, false);
					break;
				case Mirror::Type::triangle:
					//split into three
					linvel.rotate(-2*M_PI / 6);
					angle_change = M_PI / 6.0f;
					for (int i = 0; i < 3; i++) {
						AC->createAttack(attack->getPosition(), attack->getRadius(), attack->getMaxAge(),
							attack->getDamage(), AttackController::Type::e_range,
							linvel.rotate(angle_change)*.66f, false);
					}
					break;
				case Mirror::Type::circle:
					//bullet hell all around!
					angle_change = M_PI / 4;
					for (float i = 0; i < 8; i++) {
						AC->createAttack(attack->getPosition(), attack->getRadius(), attack->getMaxAge(),
							attack->getDamage(), AttackController::Type::e_range,
							linvel.rotate(angle_change)*.5f, false);
					}
					break;
				}
			}
		}
	}
}

/**
* helper method for handling beginning of player collision
*/
void CollisionController::handlePlayerCollision(PlayerModel* player, physics2::Obstacle* bd, std::string* fd) {
	if (AttackController::Attack* attack = dynamic_cast<AttackController::Attack*>(bd)) {
		//TODO: Make "enemyattacksensor" a constant somewhere
		if (*(attack->getSensorName()) == "enemyattacksensor") {
			player->setHealth(player->getHealth() - attack->getDamage());
            attack->setInactive();
			if (player->getHealth() <= 0) {
				player->markRemoved(true);
			}
		}
	}
}

/**
 * Callback method for the end of a collision
 *
 * This method is called when two objects cease to touch.
 */
void CollisionController::endContact(b2Contact* contact, std::shared_ptr<PlayerModel> player) {
	//setup
	b2Fixture* fix1 = contact->GetFixtureA();
	b2Fixture* fix2 = contact->GetFixtureB();

	b2Body* body1 = fix1->GetBody();
	b2Body* body2 = fix2->GetBody();

	std::string* fd1 = reinterpret_cast<std::string*>(fix1->GetUserData().pointer);
	std::string* fd2 = reinterpret_cast<std::string*>(fix2->GetUserData().pointer);

	physics2::Obstacle* bd1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
	physics2::Obstacle* bd2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);
	//currently nothing still?
}
