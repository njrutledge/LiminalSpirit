#include "CollisionController.hpp"
#include <box2d/b2_contact.h>
#include "PlayerModel.h"
#include "AttackController.hpp"
using namespace cugl;

void CollisionController::init(std::shared_ptr<SoundController> sound) {
    _sound = sound;
}

/**
    * Processes the start of a collision
    *
    * This method is called when we first get a collision between two objects.
    *
    * @param  contact  The two bodies that collided
    * @param  player   The player pointer
    */
void CollisionController::beginContact(b2Contact* contact, std::shared_ptr<AttackController> AC, float timer) {
    //setup
    b2Fixture* fix1 = contact->GetFixtureA();
    b2Fixture* fix2 = contact->GetFixtureB();

    b2Body* body1 = fix1->GetBody();
    b2Body* body2 = fix2->GetBody();

    std::string* fd1 = reinterpret_cast<std::string*>(fix1->GetUserData().pointer);
    std::string* fd2 = reinterpret_cast<std::string*>(fix2->GetUserData().pointer);

    physics2::Obstacle* bd1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
    physics2::Obstacle* bd2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);



    if (AttackController::Attack* attack = dynamic_cast<AttackController::Attack*>(bd1)) {
        handleAttackCollision(attack, bd2, fd2, AC, timer);
    }
    else if (AttackController::Attack* attack = dynamic_cast<AttackController::Attack*>(bd2)) {
        handleAttackCollision(attack, bd1, fd1, AC, timer);
    }

    //handle enemy collision
    if (BaseEnemyModel* enemy = dynamic_cast<BaseEnemyModel*>(bd1)) {
        handleEnemyCollision(enemy, bd2, fd2, AC, timer);
    }
    else if (BaseEnemyModel* enemy = dynamic_cast<BaseEnemyModel*>(bd2)) {
        handleEnemyCollision(enemy, bd1, fd1, AC, timer);
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
void CollisionController::handleEnemyCollision(BaseEnemyModel* enemy, physics2::Obstacle* bd, std::string* fd, std::shared_ptr<AttackController> AC, float timer) {
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
                    cugl::Vec2 linvel = attack->getVel();
                    
                    switch (mirror->getType()) {
                    case Mirror::Type::square:
                        //just reflect the attack
                        AC->createAttack(attack->getPosition(), attack->getRadius()*MIRROR_AMPLIFY, attack->getMaxAge(),
                            attack->getDamage(), AttackController::Type::e_range,
                            linvel.rotate(M_PI), timer, attack->getAttackID(), attack->getFrames(), false);
                        break;
                    case Mirror::Type::triangle:
                        //reflect three back at you
                        linvel.rotate(4 * M_PI / 6);
                        angle_change = M_PI / 6.0f;
                        for (int i = 0; i < 3; i++) {
                            AC->createAttack(attack->getPosition(), attack->getRadius(), attack->getMaxAge(),
                                attack->getDamage(), AttackController::Type::e_range,
                                linvel.rotate(angle_change)*.66f, timer, attack->getAttackID(), attack->getFrames(), false);
                        }
                        break;
                    case Mirror::Type::circle:
                        //bullet hell all around!
                        angle_change = M_PI / 4;
                        for (float i = 0; i < 8; i++) {
                            AC->createAttack(attack->getPosition(), attack->getRadius(), attack->getMaxAge(),
                                attack->getDamage(), AttackController::Type::e_range,
                                linvel.rotate(angle_change)*.5f, timer, attack->getAttackID(), attack->getFrames(), false);
                        }
                        break;
                    }
                    //set to show the attack animation
                    mirror->showAttack(true);
                    
                }
                else if (attack->getType() == AttackController::Type::p_melee ||
                         attack->getType() == AttackController::Type::p_dash) {
                    if (!attack->hasHitEnemy(mirror)) {
                        if (mirror->getHealth() > 0) {
                            mirror->setHealth(mirror->getHealth() - attack->getDamage());
                            mirror->setHurt();
                        }
                        //mirror->setLastMelee(attack, timer)
                        attack->hitEnemy(mirror);
                        //mirror->setInvincibility(true);
                        mirror->setInvincibilityTimer(0.1f);
                        CULog("NEW ATTACK~~~~~~~~~~~~~~~~~~");
                        if (mirror->getHealth() <= 0) {
                            mirror->markRemoved(true);
                        }
                        _sound->play_player_sound(SoundController::playerSType::slashHit);
                    }
                    else {
                        CULog("SAME ATTACK");
                    }
                }
            }
            else{
                if (!attack->hasHitEnemy(enemy)) {
                    enemy->setHealth(enemy->getHealth() - attack->getDamage());
                    CULog("HEALTH SET");
                    if (attack->getDamage() > 0) {
                        //enemy->setInvincibility(true);
                        enemy->setInvincibilityTimer(0.2f);
                        enemy->setPlayedDamagedParticle(false);
                        enemy->setLastDamagedBy(mapToBaseAttackType(attack->getType()));
                    }
                    else {
                        CULog("NO DAMAGE ATTACK???");
                    }
                    if (attack->getType() == AttackController::Type::p_melee ||
                        attack->getType() == AttackController::Type::p_dash) {
                        //enemy->setLastMelee(attack, timer);
                        attack->hitEnemy(enemy);
                    }
                    if (enemy->getHealth() <= 0) {
                        if (Spawner* spawner = dynamic_cast<Spawner*>(enemy)) {
                            _spawner_killed = spawner->getIndex();
                        }
                        else if (enemy->getSpawnerInd() != -1) {
                            _name_of_killed_spawner_enemy = enemy->getName();
                            _index_spawner = enemy->getSpawnerInd();
                        }
                        enemy->markRemoved(true);
                    }
                }
                else {
                    CULog("SAME ATTACK? timer = %f", timer);                    
                }


                if (attack->getType() == AttackController::p_exp_package) {
                    AC->createAttack(attack->getPosition() /*cugl::Vec2(bd->getPosition().x, bd->getPosition().y)*/, 3, 0.1, 4, AttackController::p_exp, cugl::Vec2::ZERO, timer, PLAYER_RANGE, PLAYER_RANGE_FRAMES);
                }
                switch (attack->getType()) {
                    case AttackController::p_range:
                        _sound->play_player_sound(SoundController::playerSType::shootHit);
                        attack->setInactive();
                        break;
                    case AttackController::p_exp_package:
                        attack->setInactive();
                        break;
                    case AttackController::p_melee:
                        _sound->play_player_sound(SoundController::playerSType::slashHit);
                        break;
                default:
                    break;
                }
            }
        }
        else if (*(attack->getSensorName()) == "enemyattacksensor" && attack->isSplitable() && attack->getType() == AttackController::Type::e_range) {
            if (Mirror* mirror = dynamic_cast<Mirror*>(enemy)) {
                attack->setInactive();
                float angle_change;
                cugl::Vec2 linvel = attack->getVel();

                switch (mirror->getType()) {
                case Mirror::Type::square:
                    //just amplify the attack
                    AC->createAttack(attack->getPosition(), attack->getRadius()*MIRROR_AMPLIFY, attack->getMaxAge(),
                        attack->getDamage()*MIRROR_AMPLIFY, AttackController::Type::e_range,
                        linvel, timer, attack->getAttackID(), attack->getFrames(), false);
                    break;
                case Mirror::Type::triangle:
                    //split into three
                    linvel.rotate(-2*M_PI / 6);
                    angle_change = M_PI / 6.0f;
                    for (int i = 0; i < 3; i++) {
                        AC->createAttack(attack->getPosition(), attack->getRadius(), attack->getMaxAge(),
                            attack->getDamage(), AttackController::Type::e_range,
                            linvel.rotate(angle_change)*.66f, timer, attack->getAttackID(), attack->getFrames(), false);
                    }
                    break;
                case Mirror::Type::circle:
                    //bullet hell all around!
                    angle_change = M_PI / 4;
                    for (float i = 0; i < 8; i++) {
                        AC->createAttack(attack->getPosition(), attack->getRadius(), attack->getMaxAge(),
                            attack->getDamage(), AttackController::Type::e_range,
                            linvel.rotate(angle_change)*.5f, timer, attack->getAttackID(), attack->getFrames(), false);
                    }
                    break;
                }
                //set to show the attack animation
                mirror->showAttack(true);
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
            if (!player->isInvincible()) {
                player->setHealth(player->getHealth() - attack->getDamage());
                player->setIsInvincible(true);
                player->setIsStunned(true);
                player->setInvincibilityTimer(0.2f);
                _sound->play_player_sound(SoundController::playerSType::hurt);
            }
            attack->setInactive();
            if (player->getHealth() <= 0) {
                player->markRemoved(true);
            }
        }
    }
}

/**
* helper to converting from attack controller attack type to base enemy model attack type
*/
BaseEnemyModel::AttackType CollisionController::mapToBaseAttackType(AttackController::Type attackType) {
    if (attackType == AttackController::Type::e_melee) {
        return BaseEnemyModel::AttackType::e_melee;
    }
    else if (attackType == AttackController::Type::e_range) {
        return BaseEnemyModel::AttackType::e_range;
    }
    else if (attackType == AttackController::Type::p_dash) {
        return BaseEnemyModel::AttackType::p_dash;
    }
    else if (attackType == AttackController::Type::p_exp) {
        return BaseEnemyModel::AttackType::p_exp;
    }
    else if (attackType == AttackController::Type::p_exp_package) {
        return BaseEnemyModel::AttackType::p_exp_package;
    }
    else if (attackType == AttackController::Type::p_range) {
        return BaseEnemyModel::AttackType::p_range;
    }
    else {
        return BaseEnemyModel::AttackType::p_melee;
    }
}


void CollisionController::handleAttackCollision(AttackController::Attack* attack, physics2::Obstacle* bd, std::string* fd, std::shared_ptr<AttackController> AC, float timer) {
    if (!attack->isActive()) {
        return;
    }

    if (AttackController::Attack* attack2 = dynamic_cast<AttackController::Attack*>(bd)) {
        if (!attack2->isActive()) {
            return;
        }
        //both attacks, see if cancellation can occur
        if ((attack->getType() == AttackController::p_range && attack2->getType() == AttackController::e_range) || (attack->getType() == AttackController::e_range && attack2->getType() == AttackController::p_range)) {
            //both different range attacks, cancel both
            //attack->setInactive();
            //attack2->setInactive();
        }
        else if ((attack->getType() == AttackController::p_melee && attack2->getType() == AttackController::e_melee) || (attack->getType() == AttackController::e_melee && attack2->getType() == AttackController::p_melee)) {
            attack->setInactive();
            attack2->setInactive();
            //TODO: stun both player and enemy?
        }
    }
    else if ((bd && (bd->getName() == "platform" || bd->getName().find("wall")!= std::string::npos)) && attack->getType() == AttackController::p_exp_package) {
        AC->createAttack(attack->getPosition(), 3, 0.1, 4, AttackController::p_exp, cugl::Vec2::ZERO, timer, PLAYER_RANGE, PLAYER_RANGE_FRAMES);
            attack->setInactive();
    }
    if (bd && bd->getName() == "bottomwall") {
        int breaking = 1;
    }
}

/**
 * Callback method for the end of a collision
 *
 * This method is called when two objects cease to touch.
 */
void CollisionController::endContact(b2Contact* contact) {
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
