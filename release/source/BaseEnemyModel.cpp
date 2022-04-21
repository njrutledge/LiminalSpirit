//
//  BaseEnemyModel.cpp
//  Liminal Spirit Game
//
//  This class tracks the state of the enemy(s)
// 
//

#include "BaseEnemyModel.h"
#include <cugl/scene2/graph/CUPolygonNode.h>
#include <cugl/scene2/graph/CUTexturedNode.h>
#include <cugl/assets/CUAssetManager.h>

#pragma mark - 
#pragma mark Physics Constants
/** Amount to shrink the body fixture vertically*/
#define ENEMY_VSHRINK 0.95f
/** Amount the shrink the body fixture by horizontally*/
#define ENEMY_HSHRINK 0.7f
/** */
#define ENEMY_SSHRINK 0.6f
/**Height of the sensor */
#define SENSOR_HEIGHT 01.f
/** Debug color for sensor */
#define DEBUG_COLOR Color4::RED
/** Color of HealthBar back */
#define HEALTHBACK_COLOR Color4(160,160,220)
/** Color of HealthBar health */
#define HEALTH_COLOR Color4(200,240,200)


using namespace cugl;

#pragma mark - 
#pragma mark Constructors

/** Initializes the enemy at the given position, with size, scale, health and horizontal/vertical speed. */
bool BaseEnemyModel::init(const cugl::Vec2& pos, const cugl::Size& realSize, const cugl::Size& size, float scale, EnemyProperties props) {
	_size = realSize;
	Size nsize = size;
	nsize.width *= ENEMY_HSHRINK;
	nsize.height *= ENEMY_VSHRINK;
	_drawScale = scale;
	_health = props.health;
	_maxhealth = props.health;
	_verticalSpeed = props.vspeed;
	_horizontalSpeed = props.hspeed;
	_attackCooldown = props.attackCooldown;
	_attackRadius = props.attackRadius;
	_timePast = 0.0f;
	_enemyName = props.name;
	_density = props.density;
    _spawnerIndex = -1;

	if (CapsuleObstacle::init(pos, nsize)) {
		setDensity(_density);
		setFriction(0.0f);
		setFixedRotation(true);
		b2Filter filter = b2Filter();
		filter.categoryBits = 0b10;
		filter.maskBits = 0b011100;
		setFilterData(filter);
		return true;
	}
	return false;
}

#pragma mark - 
#pragma mark Physics Methods

/** Creates new fixtures for this body */
void BaseEnemyModel::createFixtures() {
	if (_body == nullptr) {
		return;
	}
	CapsuleObstacle::createFixtures();
	b2FixtureDef sensorDef;
	sensorDef.density = _density;
	sensorDef.isSensor = true;

	// Sensor dimensions
	b2Vec2 corners[4];
	corners[0].x = -ENEMY_SSHRINK * getWidth() / 2.0f;
	corners[0].y = (-getHeight() + SENSOR_HEIGHT) / 2.0f;
	corners[1].x = -ENEMY_SSHRINK * getWidth() / 2.0f;
	corners[1].y = (-getHeight() - SENSOR_HEIGHT) / 2.0f;
	corners[2].x = ENEMY_SSHRINK * getWidth() / 2.0f;
	corners[2].y = (-getHeight() - SENSOR_HEIGHT) / 2.0f;
	corners[3].x = ENEMY_SSHRINK * getWidth() / 2.0f;
	corners[3].y = (-getHeight() + SENSOR_HEIGHT) / 2.0f;

	b2PolygonShape sensorShape;
	sensorShape.Set(corners, 4);

	sensorDef.shape = &sensorShape;
	sensorDef.userData.pointer = reinterpret_cast<uintptr_t>(getSensorName());
	_sensorFixture = _body->CreateFixture(&sensorDef);

	

}

/** Releases the fixtures for this body, reseting the shape */
void BaseEnemyModel::releaseFixtures() {
	if (_body != nullptr) {
		return;
	}

	CapsuleObstacle::releaseFixtures();
	if (_sensorFixture != nullptr) {
		_body->DestroyFixture(_sensorFixture);
		_sensorFixture = nullptr;
	}
}

/** Disposes all resources and assets of this enemy model */
void BaseEnemyModel::dispose() {
	_core = nullptr;
	_node->removeAllChildren();
	_node = nullptr;
	_sensorNode = nullptr;
}

/** Sets the vertical movement of the enemy*/
void BaseEnemyModel::setVX(float value) {
	_body->SetLinearVelocity(b2Vec2(value, _body->GetLinearVelocity().y));
}

/** Applies the force to the body of this enemy */

void BaseEnemyModel::applyForce() {
	if (!isEnabled()) {
		return;
	}
}

void BaseEnemyModel::update(float dt) {
	CapsuleObstacle::update(dt);
	if (_node != nullptr) {
		_node->setPosition(getPosition() * _drawScale);
		_node->setAngle(getAngle());
		//update healthbar
		if (_healthTimer > 0) {
			if (scene2::PolygonNode* foundHealthBar = dynamic_cast<scene2::PolygonNode*>(_node->getChildByName("healthbar").get())) {
				foundHealthBar->setPolygon((Rect(0, 0, _health / 2.0f / _node->getScaleX(), .1 / _node->getScaleY()) * _drawScale));
				cugl::Poly2 poly = foundHealthBar->getPolygon();
				//foundHealthBar->setPosition(Vec2(_size.width / 2, _size.height) - Vec2((_maxhealth - _health)/2/_node->getScaleX() * _drawScale, 0));
				foundHealthBar->setPosition(Vec2(_size.width / 2, _size.height) - Vec2((_maxhealth - _health) / 4.0 / _node->getScaleX() *_drawScale,0));
			}
			else {
				//add health bars
				std::shared_ptr<scene2::PolygonNode> healthBarBack = scene2::PolygonNode::allocWithPoly(Rect(0, 0, _maxhealth / 2.0f/ _node->getScaleX(), .1/_node->getScaleY()) * _drawScale);
				healthBarBack->setColor(HEALTHBACK_COLOR);
				healthBarBack->setAnchor(.5, 0);
				healthBarBack->setPosition(Vec2(_size.width / 2, _size.height));
				healthBarBack->setPriority(2);
				_node->addChildWithName(healthBarBack, "healthbarback");

				std::shared_ptr<scene2::PolygonNode> healthBar = scene2::PolygonNode::allocWithPoly(Rect(0, 0, _health / 2.0f / _node->getScaleX(), .1/ _node->getScaleY()) * _drawScale);
				healthBar->setColor(HEALTH_COLOR);
				healthBar->setAnchor(.5, 0);
				healthBar->setPosition(Vec2(_size.width / 2, _size.height));
				healthBarBack->setPriority(3);
				_node->addChildWithName(healthBar, "healthbar");
			}
			_healthTimer -= dt;
		}
		else {
			_node->removeChildByName("healthbar");
			_node->removeChildByName("healthbarback");
		}
	}

}

#pragma mark -
#pragma mark Scene Graph Methods
/** Redraws outline of physics fixtures*/
void BaseEnemyModel::resetDebug() {
	CapsuleObstacle::resetDebug();
	float w = ENEMY_SSHRINK * _dimension.width;
	float h = SENSOR_HEIGHT;
	Poly2 poly(Rect(-w / 2.0f, -h / 2.0f, w, h));

	_sensorNode = scene2::WireNode::allocWithTraversal(poly, poly2::Traversal::INTERIOR);
	_sensorNode->setColor(DEBUG_COLOR);
	_sensorNode->setPosition(Vec2(_debug->getContentSize().width / 2.0f, 0.0f));
	_debug->addChild(_sensorNode);
}
