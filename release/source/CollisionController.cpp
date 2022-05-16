#include "CollisionController.hpp"
#include <box2d/b2_contact.h>
#include "PlayerModel.h"
#include "AttackController.hpp"
using namespace cugl;

#define MAX_STALEING 0.5

void CollisionController::init(std::shared_ptr<SoundController> sound) {
    _sound = sound;
    
    _mCoolReduction = 0;
    _rCoolReduction = 0;
    
    _stale = 5;
    
    std::shared_ptr<JsonReader> reader = JsonReader::alloc(Application::get()->getSaveDirectory() + "savedGame.json");
    std::shared_ptr<JsonValue> save = reader->readJson();
    std::shared_ptr<JsonValue> progress = save->get("progress");
    _unlock_count = progress->get("unlock_count")->asInt();
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

    if (fd1 && *fd1 == "playerattacksensorhoming") {
        int breaking = 1;
    }
    if (fd2 && *fd2 == "playerattacksensorhoming") {
        int breaking = 2;
    }


    if (AttackController::Attack* attack = dynamic_cast<AttackController::Attack*>(bd1)) {
        string test = fd1 ? *fd1 : "NULLLLLLL";
        handleAttackCollision(attack, fd1, bd2, fd2, AC, timer);
    }
    else if (AttackController::Attack* attack = dynamic_cast<AttackController::Attack*>(bd2)) {
        string test = fd2 ? *fd2 : "NULLLLLLL";
        handleAttackCollision(attack, fd2, bd1, fd1, AC, timer);
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
        if (!fd) {
            //TODO:: IDK BRUH NO FIXTURE WHAT DO
           int breaking = 1;
           return;
        }
        //TODO: Make "playerattacksensor" a constant somewhere
        if (*fd == "playerattacksensor") {
            if (Mirror* mirror = dynamic_cast<Mirror*>(enemy)) {
                if (attack->getType() == AttackController::p_range) {
                    //attack->markRemoved();
                    attack->setInactive();
                    float angle_change;
                    cugl::Vec2 linvel = attack->getVel();
                    int angle = 60;
                    float randAngle = (rand() % angle) - angle / 2.0f;
                    randAngle = randAngle * M_PI / 180.0f;
                    switch (mirror->getType()) {
                    case Mirror::Type::square:
                        //just reflect the attack
                        AC->createAttack(attack->getPosition(), attack->getRadius()*MIRROR_SQUARE_SCALE, attack->getMaxAge(),
                            mirror->getAttackDamage(), AttackController::Type::e_range,
                            linvel.rotate(M_PI+randAngle), timer, attack->getAttackID(), attack->getFrames(), false);
                        break;
                    case Mirror::Type::triangle:
                        //reflect three back at you
                        linvel.rotate(4 * M_PI / 6 + randAngle);
                        angle_change = M_PI / 6.0f;
                        for (int i = 0; i < 3; i++) {
                            AC->createAttack(attack->getPosition(), attack->getRadius()*MIRROR_TRI_SCALE, attack->getMaxAge(),
                                mirror->getAttackDamage()*MIRROR_TRI_AMP, AttackController::Type::e_range,
                                linvel.rotate(angle_change)*MIRROR_TRI_AMP, timer, attack->getAttackID(), attack->getFrames(), false);
                        }
                        break;
                    case Mirror::Type::circle:
                        //bullet hell all around!
                        angle_change = M_PI / 4;
                        for (float i = 0; i < 8; i++) {
                            AC->createAttack(attack->getPosition(), attack->getRadius()*MIRROR_CIRC_SCALE, attack->getMaxAge(),
                                mirror->getAttackDamage()*MIRROR_CIRC_AMP, AttackController::Type::e_range,
                                linvel.rotate(angle_change)*MIRROR_CIRC_AMP, timer, attack->getAttackID(), attack->getFrames(), false);
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
                        //CULog("NEW ATTACK~~~~~~~~~~~~~~~~~~");
                        if (mirror->getHealth() <= 0) {
                            mirror->markRemoved(true);
                        }
                        
                        if (attack->getType() == AttackController::Type::p_melee) {
                            _sound->play_player_sound(SoundController::playerSType::slashHit);
                            _rCoolReduction += 1;
                            _stall = true;
                        } else if (attack->getType() == AttackController::Type::p_dash) {
                            _sound->play_player_sound(SoundController::playerSType::slashDashHit);
                        }
                    }
                    else {
                        //CULog("SAME ATTACK");
                    }
                }
            }
            else{
                if (!attack->hasHitEnemy(enemy)) {
                    int damage = getDamageDealt(attack, enemy);
                    switch (_unlock_count) {
                        case 3:
                        case 4:
                            damage *= 2;
                            break;
                        case 5:
                            damage *= 3;
                        default:
                            break;
                    }
                    enemy->setHealth(enemy->getHealth() - damage);
                    //CULog("HEALTH SET");
                    if (damage > 0) {
                        //enemy->setInvincibility(true);
                        enemy->setInvincibilityTimer(0.2f);
                        enemy->setPlayedDamagedParticle(false);
                        enemy->setLastDamagedBy(mapToBaseAttackType(attack->getType()));
                    }
                    else {
                        //CULog("NO DAMAGE ATTACK???");
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
                    switch (attack->getType()) {
                        case AttackController::p_range:
                            _sound->play_player_sound(SoundController::playerSType::shootHit);
                            _mCoolReduction += 1;
                            _stale = clampi(_stale + 1, 0, 10);
                            attack->setInactive();
                            break;
                        case AttackController::p_melee:
                            _sound->play_player_sound(SoundController::playerSType::slashHit);
                            _rCoolReduction += 1;
                            _stall = true;
                            _stale = clampi(_stale - 1, 0, 10);
                            break;
                        case AttackController::p_dash:
                            _sound->play_player_sound(SoundController::playerSType::slashDashHit);
                    default:
                        break;
                    }
                }
                else {
                    //CULog("SAME ATTACK? timer = %f", timer);                    
                }


                if (attack->getType() == AttackController::p_exp_package) {
                    AC->createAttack(attack->getPosition() /*cugl::Vec2(bd->getPosition().x, bd->getPosition().y)*/, 5, 0.8, 30, AttackController::p_exp, cugl::Vec2::ZERO, timer, PLAYER_RANGE, PLAYER_EXP_FRAMES);
                    _sound->play_player_sound(SoundController::playerSType::explosion);
                    attack->setInactive();
                }
               
            }
        }
        else if (*fd == "enemyattacksensor" && attack->isSplitable() && attack->getType() == AttackController::Type::e_range) {
            if (Mirror* mirror = dynamic_cast<Mirror*>(enemy)) {
                attack->setInactive();
                float angle_change;
                cugl::Vec2 linvel = attack->getVel();
                switch (mirror->getType()) {
                case Mirror::Type::square:
                    //just amplify the attack
                    AC->createAttack(attack->getPosition(), attack->getRadius()*MIRROR_SQUARE_SCALE, attack->getMaxAge(),
                        attack->getDamage()*MIRROR_SQUARE_AMP, AttackController::Type::e_range,
                        linvel, timer, attack->getAttackID(), attack->getFrames(), false);
                    break;
                case Mirror::Type::triangle:
                    //split into three
                    linvel.rotate(-2*M_PI / 6);
                    angle_change = M_PI / 6.0f;
                    for (int i = 0; i < 3; i++) {
                        AC->createAttack(attack->getPosition(), attack->getRadius()*MIRROR_TRI_SCALE, attack->getMaxAge(),
                            attack->getDamage()*MIRROR_TRI_AMP, AttackController::Type::e_range,
                            linvel.rotate(angle_change)*MIRROR_TRI_AMP, timer, attack->getAttackID(), attack->getFrames(), false);
                    }
                    break;
                case Mirror::Type::circle:
                    //bullet hell all around!
                    angle_change = M_PI / 4;
                    for (float i = 0; i < 8; i++) {
                        AC->createAttack(attack->getPosition(), attack->getRadius()*MIRROR_CIRC_SCALE, attack->getMaxAge(),
                            attack->getDamage()*MIRROR_CIRC_AMP, AttackController::Type::e_range,
                            linvel.rotate(angle_change)*MIRROR_CIRC_AMP, timer, attack->getAttackID(), attack->getFrames(), false);
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
        if (!attack->isActive()) {
            return;
        }
        //TODO: Make "enemyattacksensor" a constant somewhere
        if (*(attack->getSensorName()) == "enemyattacksensor") {
            if (!player->isInvincible()) {
                player->setHealth(player->getHealth() - attack->getDamage());
                player->setIsInvincible(true);
                player->setIsStunned(true);
                player->setInvincibilityTimer(0.8f);
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


void CollisionController::handleAttackCollision(AttackController::Attack* attack, std::string* fd1, physics2::Obstacle* bd, std::string* fd2, std::shared_ptr<AttackController> AC, float timer) {
    if (!attack->isActive()) {
        return;
    }
    if (fd1 && *fd1 == "playerattacksensorhoming") {
        if (BaseEnemyModel* enemy = dynamic_cast<BaseEnemyModel*>(bd)) {
            attack->setHomingEnemy(enemy);
            //CULog("%f, %f", attack->getLinearVelocity().x, attack->getLinearVelocity().y);

        }

    }else if (AttackController::Attack* attack2 = dynamic_cast<AttackController::Attack*>(bd)) {
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
    else if ((bd && (bd->getName().find("wall")!= std::string::npos))){
        string text = fd1 ? *fd1 : "NULLLL";
        switch(attack->getType()) {
        case AttackController::p_exp_package:
            AC->createAttack(attack->getPosition(), 3, 0.15, 30, AttackController::p_exp, cugl::Vec2::ZERO, timer, PLAYER_RANGE, PLAYER_EXP_FRAMES);
                _sound->play_player_sound(SoundController::playerSType::explosion);
            attack->setInactive();
            break;
        case AttackController::p_range:
        case AttackController::e_range:
            attack->setInactive();
            break;
        }
    }
}

/** determine the amount of damage an enemy is going to take from a particular attack */
int CollisionController::getDamageDealt(AttackController::Attack* attack, BaseEnemyModel* enemy){
    float mMult = 1.0f;
    float rMult = 1.0f;
    if (_stale < 5) {
        mMult -= (5 - _stale) * (MAX_STALEING / 5.0f);
        rMult += (5 - _stale) * (MAX_STALEING / 5.0f);
    } else if (_stale > 5) {
        mMult += (_stale - 5) * (MAX_STALEING / 5.0f);
        rMult -= (_stale - 5) * (MAX_STALEING / 5.0f);
    }
    switch (attack->getType()){
        case AttackController::p_range:
            if (enemy->getName() == "Glutton"){
                return attack->getDamage() / 2 * rMult;
            } else if (enemy->getName() == "Seeker") {
                return attack->getDamage() * 2 * rMult;
            } else {
                return attack->getDamage() * rMult;
            }
        case AttackController::p_exp:
            if (enemy->getName() == "Glutton"){
                return attack->getDamage() / 2;
            } else if (enemy->getName() == "Seeker") {
                return attack->getDamage() * 2;
            } else {
                return attack->getDamage();
            }
        case AttackController::p_dash:
            return attack->getDamage();
        default:
            return attack->getDamage() * mMult;
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

void CollisionController::reset() {
    _mCoolReduction = 0;
    _rCoolReduction = 0;
    
    _stale = 5;
    
    _stall = false;
    
}
