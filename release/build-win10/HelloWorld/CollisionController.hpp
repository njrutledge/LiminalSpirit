#ifndef __LS_COLLISION_CONTROLLER_H__
#define __LS_COLLISION_CONTROLLER_H__
#include <cugl/cugl.h>
#include "AttackController.hpp"
#include "BaseEnemyModel.h"

class CollisionController {
public:
	/** Creates a new collision Controller */
	CollisionController() {}

	/**Deletes the collision controller */
	~CollisionController() {}

	/** returns true if there is a attack-enemy collision. If there is a collision, this also resolves it*/
	bool resolveCollision(AttackController& aset, BaseEnemyModel& enemy);
};

#endif /* __LS_COLLISION_CONTROLLER_H__ */