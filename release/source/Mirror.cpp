//
//  Mirror.cpp
//  Liminal Spirit Game
//
//  This class is the mirroring crystal enemy
// 
//
#include "Mirror.hpp"

EnemyProperties MIRROR_PROPS{
	/** Starting health of the Mirror */
	3,
	/** Vertical speed of the Mirror */
	5.0f,
	/** Horizontal speed of the Mirror*/
	5.0f,
	/** Cooldown for attack(s) in seconds*/
	1,
	/** Attack radius of the Mirror*/
	0.0f,
	/** Density of the Mirror*/
	0.0f,
	/** Name of the Mirror*/
	"Mirror",
};


bool Mirror::init(const cugl::Vec2& pos, const cugl::Size& size, float scale, EnemyProperties props, Mirror::Type type, std::shared_ptr<BaseEnemyModel> enemy) {
	if (BaseEnemyModel::init(pos, size, scale, props)) {
		_linkedEnemy = enemy;
		_type = type;
		return true;
	}
	return false;

}

void Mirror::update(float dt) {
	BaseEnemyModel::update(dt);
	if (_linkedEnemy && _linkedEnemy->getHealth() <= 0) {
		_linkedEnemy = nullptr;
	}
}