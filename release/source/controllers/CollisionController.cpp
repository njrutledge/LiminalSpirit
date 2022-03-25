#include "CollisionController.hpp"
#include <box2d/b2_contact.h>
#include "models/PlayerModel.h"
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
void CollisionController::beginContact(b2Contact *contact, std::shared_ptr<PlayerModel> player, std::shared_ptr<AttackController> AC)
{
	// setup
	b2Fixture *fix1 = contact->GetFixtureA();
	b2Fixture *fix2 = contact->GetFixtureB();

	b2Body *body1 = fix1->GetBody();
	b2Body *body2 = fix2->GetBody();

	std::string *fd1 = reinterpret_cast<std::string *>(fix1->GetUserData().pointer);
	std::string *fd2 = reinterpret_cast<std::string *>(fix2->GetUserData().pointer);

	physics2::Obstacle *bd1 = reinterpret_cast<physics2::Obstacle *>(body1->GetUserData().pointer);
	physics2::Obstacle *bd2 = reinterpret_cast<physics2::Obstacle *>(body2->GetUserData().pointer);

	// See if we have landed on the ground.
	// THIS NEED TO BE CHANGED
	if ((player->getSensorName() == fd2 && (bd1->getName() == "floor" || bd1->getName() == "platform")) ||
			(player->getSensorName() == fd1 && (bd2->getName() == "floor" || bd2->getName() == "platform")))
	{

		// player->setGrounded(true);
	}

	if ((player->getSensorName() == fd2 && (bd1->getName() == "leftwall" || bd1->getName() == "rightwall")) ||
			(player->getSensorName() == fd1 && (bd2->getName() == "leftwall" || bd2->getName() == "rightwall")))
	{
		// player->setSensor(false);
	}

	// handle enemy collision
	if (BaseEnemyModel *enemy = dynamic_cast<BaseEnemyModel *>(bd1))
	{
		handleEnemyCollision(enemy, bd2, fd2, AC);
	}
	else if (BaseEnemyModel *enemy = dynamic_cast<BaseEnemyModel *>(bd2))
	{
		handleEnemyCollision(enemy, bd1, fd1, AC);
	}

	// handlePlayerCollision
	if (PlayerModel *player = dynamic_cast<PlayerModel *>(bd1))
	{
		handlePlayerCollision(player, bd2, fd2);
	}
	else if (PlayerModel *player = dynamic_cast<PlayerModel *>(bd1))
	{
		handlePlayerCollision(player, bd1, fd1);
	}
}

/**
 * helper method for handling beginning of enemy collision
 */
void CollisionController::handleEnemyCollision(BaseEnemyModel *enemy, physics2::Obstacle *bd, std::string *fd, std::shared_ptr<AttackController> AC)
{
	if (AttackController::Attack *attack = dynamic_cast<AttackController::Attack *>(bd))
	{
		// TODO: Make "playerattacksensor" a constant somewhere
		if (*(attack->getSensorName()) == "playerattacksensor")
		{
			enemy->setHealth(enemy->getHealth() - attack->getDamage());
			if (enemy->getHealth() <= 0)
			{
				enemy->markRemoved(true);
			}
			if (attack->getType() == AttackController::p_exp_package)
			{
				AC->createAttack(cugl::Vec2(bd->getPosition().x, bd->getPosition().y), 3, 0.1, 9000, AttackController::p_exp, cugl::Vec2::ZERO);
			}
			switch (attack->getType())
			{
			case AttackController::p_range:
			case AttackController::p_exp_package:
				attack->setInactive();
				break;
			default:
				break;
			}
		}
	}
}

/**
 * helper method for handling beginning of player collision
 */
void CollisionController::handlePlayerCollision(PlayerModel *player, physics2::Obstacle *bd, std::string *fd)
{
	if (AttackController::Attack *attack = dynamic_cast<AttackController::Attack *>(bd))
	{
		// TODO: Make "enemyattacksensor" a constant somewhere
		if (*(attack->getSensorName()) == "enemyattacksensor")
		{
			player->setHealth(player->getHealth() - attack->getDamage());
			attack->setInactive();
			if (player->getHealth() <= 0)
			{
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
void CollisionController::endContact(b2Contact *contact, std::shared_ptr<PlayerModel> player)
{
	// setup
	b2Fixture *fix1 = contact->GetFixtureA();
	b2Fixture *fix2 = contact->GetFixtureB();

	b2Body *body1 = fix1->GetBody();
	b2Body *body2 = fix2->GetBody();

	std::string *fd1 = reinterpret_cast<std::string *>(fix1->GetUserData().pointer);
	std::string *fd2 = reinterpret_cast<std::string *>(fix2->GetUserData().pointer);

	physics2::Obstacle *bd1 = reinterpret_cast<physics2::Obstacle *>(body1->GetUserData().pointer);
	physics2::Obstacle *bd2 = reinterpret_cast<physics2::Obstacle *>(body2->GetUserData().pointer);

	// See if we have left the ground
	// THIS NEEDS TO BE CHANGED
	if ((player->getSensorName() == fd2 && (bd1->getName() == "floor" || bd1->getName() == "platform")) ||
			(player->getSensorName() == fd1 && (bd2->getName() == "floor" || bd2->getName() == "platform")))
	{

		// player->setGrounded(false);
	}
	/* if (*fd1 == "attacksensor") {
		if (*fd2 == "enemysensor") {
			CULog("ATTACK end");
		}
	}
	else if (*fd2 == "attacksensor") {
		if (*fd1 == "enemysensor") {
			CULog("ATTACK end");
		}
	}
	*/
}
