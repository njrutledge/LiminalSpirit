#ifndef __LS_COLLISION_CONTROLLER_H__
#define __LS_COLLISION_CONTROLLER_H__
#include <cugl/cugl.h>
#include "AttackController.hpp"
#include <box2d/b2_contact.h>
#include "BaseEnemyModel.h"
#include "Mirror.hpp"
#include "Spawner.hpp"
#include "PlayerModel.h"
#include "SoundController.hpp"

class CollisionController {
public:
    /** Creates a new collision Controller */
    CollisionController() {_spawner_killed = -1; _index_spawner = -1;}
    
    void init(std::shared_ptr<SoundController> sound);

    /**Deletes the collision controller */
    ~CollisionController() {}


    void beginContact(b2Contact* contact, std::shared_ptr<AttackController> AC, float timer);

    void endContact(b2Contact* contact);
    
    int getSpawnerKilled() {return _spawner_killed;};
    void setSpawnerKilled(int v) { _spawner_killed = v;};
    
    string getSpawnerEnemyName() {return _name_of_killed_spawner_enemy;};
    
    int getIndexSpawner() {return _index_spawner;};
    void setIndexSpawner(int v) { _index_spawner = v;};

private:
    std::shared_ptr<SoundController> _sound;
    /** the index of spawner that is been killed */
    int _spawner_killed;
    string _name_of_killed_spawner_enemy;
    int _index_spawner;
    /** handle collision between enemy and an obstacle */
    void handleEnemyCollision(BaseEnemyModel* enemy, cugl::physics2::Obstacle* bd, string* fd, std::shared_ptr<AttackController> AC, float timer);

    /**handle collision between player and an obstacle */
    void handlePlayerCollision(PlayerModel* player, cugl::physics2::Obstacle* bd, std::string* fd);

    /** handle collision between attack and a non-enemy obstacle (i.e, if bd is an enemy, nothing will happen) */
    void handleAttackCollision(AttackController::Attack* attack1, std::string* fd1, cugl::physics2::Obstacle* bd, std::string* fd2, std::shared_ptr<AttackController> AC, float timer);
    
    /** determine the amount of damage an enemy is going to take from a particular attack */
    int getDamageDealt(AttackController::Attack* attack, BaseEnemyModel* enemy);

    /** helper to converting from attack controller attack type to base enemy model attack type*/
    BaseEnemyModel::AttackType mapToBaseAttackType(AttackController::Type attackType);
};

#endif /* __LS_COLLISION_CONTROLLER_H__ */
