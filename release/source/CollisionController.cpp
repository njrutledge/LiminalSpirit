#include "CollisionController.hpp"

using namespace cugl;
//TEST
bool CollisionController::resolveCollision(AttackController& ac, BaseEnemyModel& enemy) {
	bool collision = false;
	auto it = ac._current.begin();
	while (it != ac._current.end()) {
		std::shared_ptr<AttackController::Attack> attack = *it;

		Vec2 norm = attack->getPosition() - Vec2(enemy.getBody()->GetPosition().x, enemy.getBody()->GetPosition().y);
		float distance = norm.length();
		float impactDistance = attack->getRadius() + enemy.getRadius();
		if (distance < impactDistance) {
			collision = true;
			enemy.setHealth(enemy.getHealth() - attack->getDamage());
			if (enemy.getHealth() <= 0) {
				enemy.markRemoved(true);
			}
		}
		++it;
	}
	return collision;
}