#ifndef __LS_COLLISION_CONTROLLER_H__
#define __LS_COLLISION_CONTROLLER_H__
#include <cugl/cugl.h>
#include "AttackController.hpp"
#include <box2d/b2_contact.h>
#include "BaseEnemyModel.h"
#include "PlayerModel.h"

class CollisionController {
public:
	/** Creates a new collision Controller */
	CollisionController() {}

	/**Deletes the collision controller */
	~CollisionController() {}


	void beginContact(b2Contact* contact, std::shared_ptr<PlayerModel> player, std::shared_ptr<AttackController> AC);

	void endContact(b2Contact* contact, std::shared_ptr<PlayerModel> player);

private:
	/** handle collision between enemy and an obstacle */
	void handleEnemyCollision(BaseEnemyModel* enemy, cugl::physics2::Obstacle* bd, string* fd, std::shared_ptr<AttackController> AC);

	/**handle collision between player and an obstacle */
	void handlePlayerCollision(PlayerModel* player, cugl::physics2::Obstacle* bd, std::string* fd);
};

#endif /* __LS_COLLISION_CONTROLLER_H__ */