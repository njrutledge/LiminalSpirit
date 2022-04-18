#ifndef __LS_COLLISION_CONTROLLER_H__
#define __LS_COLLISION_CONTROLLER_H__
#include <cugl/cugl.h>
#include "AttackController.hpp"
#include <box2d/b2_contact.h>
#include "BaseEnemyModel.h"
#include "Mirror.hpp"
#include "Spawner.hpp"
#include "PlayerModel.h"

class CollisionController {
public:
    /** Creates a new collision Controller */
    CollisionController() {_spawner_killed = 0;}

    /**Deletes the collision controller */
    ~CollisionController() {}


    void beginContact(b2Contact* contact, std::shared_ptr<AttackController> AC, float timer);

    void endContact(b2Contact* contact);
    
    float getSpawnerKilled() {return _spawner_killed;};

private:
    /** if a spawner exists, 1 if it is killed, 0 if it is not. It is 0 if spawner does not exits. */
    float _spawner_killed;
    /** handle collision between enemy and an obstacle */
    void handleEnemyCollision(BaseEnemyModel* enemy, cugl::physics2::Obstacle* bd, string* fd, std::shared_ptr<AttackController> AC, float timer);

    /**handle collision between player and an obstacle */
    void handlePlayerCollision(PlayerModel* player, cugl::physics2::Obstacle* bd, std::string* fd);

    /** handle collision between attack and a non-enemy obstacle (i.e, if bd is an enemy, nothing will happen) */
    void handleAttackCollision(AttackController::Attack* attack1, cugl::physics2::Obstacle* bd, std::string* fd, std::shared_ptr<AttackController> AC, float timer);
};

#endif /* __LS_COLLISION_CONTROLLER_H__ */
