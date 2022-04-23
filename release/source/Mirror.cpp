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

bool Mirror::init(const cugl::Vec2& pos, const cugl::Size& realSize, const cugl::Size& size, float scale, EnemyProperties props, Mirror::Type type, std::shared_ptr<BaseEnemyModel> enemy) {
	
	if (BaseEnemyModel::init(pos, realSize, size, scale, props)) {
		_linkedEnemy = enemy;
		_type = type;
		//okay its a lot of magic numbers but its all just relative positions
		_shard1Positions[0] = cugl::Vec2(_size.width / 6, _size.height * 6 / 10);
		_shard1Positions[1] = cugl::Vec2(_size.width / 6, _size.height /2);
		_shard1Positions[2] = cugl::Vec2(_size.width / 6, _size.height * 4 / 10);
		_shard1Positions[3] = cugl::Vec2(_size.width / 6, _size.height / 2);
		_shard1Index = rand() % 4;

		_shard2Positions[0] = cugl::Vec2(_size.width * 7 / 8, _size.height * 4 / 6);
		_shard2Positions[1] = cugl::Vec2(_size.width * 6 / 8, _size.height * 3 / 6);
		_shard2Positions[2] = cugl::Vec2(_size.width * 6 / 8, _size.height * 4 / 6);
		_shard2Positions[3] = cugl::Vec2(_size.width * 7 / 8, _size.height * 5 / 6);
		_shard2Index = rand() % 4;

		_shard3Positions[0] = cugl::Vec2(_size.width * 5 / 8, _size.height / 6);
		_shard3Positions[1] = cugl::Vec2(_size.width * 6 / 8, _size.height / 6);
		_shard3Positions[2] = cugl::Vec2(_size.width * 7 / 8, _size.height / 6);
		_shard3Positions[3] = cugl::Vec2(_size.width * 6 / 8, _size.height / 6);
		_shard3Index = rand() % 4;


		return true;
	}
	return false;

}

void Mirror::update(float dt) {
	BaseEnemyModel::update(dt);
	if (_linkedEnemy && _linkedEnemy->getHealth() <= 0) {
		_linkedEnemy = nullptr;
	}
	updateAnimations(dt);

}
void Mirror::updateAnimations(float dt) {
	//attack animation
	if (_showAttack) {
		//update timer
		_attackTime += dt;
		if (_node->getChildByName("attack")) {
			int nextFrame = -1;
			if (_attackTime > .1f) {
				nextFrame = _attackSprite->getFrame() + 1;
				CULog("nextFrame: %d", nextFrame);
				if (nextFrame < 3) {
					_attackSprite->setFrame(nextFrame);
					setAttackAnimationTimer(0);
				}
				else {
					_attackSprite->setFrame(0);
					showAttack(false);
					_node->removeChildByName("attack");
				}
			}
		}
		else {
			_attackSprite->setPosition(cugl::Vec2(_size.width / 2, _size.height / 2));
			_attackSprite->setScale(5.0f);
			_attackSprite->setPriority(3.1);
			_node->addChildWithName(_attackSprite, "attack");
			//_node->swapChild(_node->getChildByName("attack"), _node->getChild(0));
			showAttack(true);
			setAttackAnimationTimer(0);
			updateAnimations(dt);//so as to not miss the first animation on adding, and to not duplicate code
		}
	}
	//see if shard1 does not exists, if so, no shard exists and add all three shards
	if (!_node->getChildByName("shard1")) {
		//shard 1
		_shard1->setPosition(cugl::Vec2(_size.width / 6, _size.height / 2));// *_drawScale * 15);
		_shard1->setScale(.75f);
		_node->addChildWithName(_shard1, "shard1");
		_shard1Time = 0;
		//shard2
		_shard2->setPosition(cugl::Vec2(_size.width * 7 / 8, _size.height * 2 / 3));// *_drawScale * 15);
		_shard2->setScale(.75f);
		_node->addChildWithName(_shard2, "shard2");
		_shard2Time = 0;
		//shard3
		_shard3->setPosition(cugl::Vec2(_size.width * 5 / 8, _size.height / 6));// *_drawScale * 15);
		_shard3->setScale(.75f);
		_node->addChildWithName(_shard3, "shard3");
		_shard3Time = 0;
	}
	_shard1Time += dt;
	_shard2Time += dt;
	_shard3Time += dt;
	updateShard(&_shard1Time, &_shard1Index, _shard1, _shard1Positions, 0.3f);
	updateShard(&_shard2Time, &_shard2Index, _shard2, _shard2Positions, 0.4f);
	updateShard(&_shard3Time, &_shard3Index, _shard3, _shard3Positions, 0.5f);



}

void Mirror::updateShard(float* shardTime, int* shardIndex, std::shared_ptr<cugl::scene2::PolygonNode> shard, cugl::Vec2 shardPositions[4], float time) {
	if (*shardTime > time) {
		*shardIndex = ((*shardIndex + 1) % 4);
		shard->setPosition((shardPositions[*shardIndex]));
		*shardTime = 0;
	}
	else {
		cugl::Vec2 pos1 = shardPositions[*shardIndex];
		cugl::Vec2 pos2 = shardPositions[(*shardIndex + 1) % 4];
		float t = *shardTime / time;
		cugl::Vec2 diff = pos2 - pos1;
		shard->setPosition((shardPositions[*shardIndex] + (diff * t)));

	}
}