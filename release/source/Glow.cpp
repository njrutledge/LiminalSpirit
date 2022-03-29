//
//  Glow.cpp
//  Liminal Spirit Game
//
//  This class tracks the glow effect
// 
//

#include "Glow.hpp"
#include <cugl/scene2/graph/CUPolygonNode.h>
#include <cugl/scene2/graph/CUTexturedNode.h>
#include <cugl/assets/CUAssetManager.h>

using namespace cugl;
#pragma mark - 
#pragma mark Physics Constants
/** Amount to shrink the body fixture vertically*/
#define GLOW_VSHRINK 0.95f
/** Amount the shrink the body fixture by horizontally*/
#define GLOW_HSHRINK 0.7f
/** */
#define GLOW_SSHRINK 0.6f
/**Height of the sensor */
#define SENSOR_HEIGHT 01.f
/** Debug color for sensor */
#define DEBUG_COLOR Color4::RED

#pragma mark - 
#pragma mark Constructors
/** Initializes the Glow. */
bool Glow::init(const cugl::Vec2& pos, const cugl::Size& size, float scale) {
	Size nsize = size;
	nsize.width *= GLOW_HSHRINK;
	nsize.height *= GLOW_VSHRINK;
	_drawScale = scale;

	if (CapsuleObstacle::init(pos, nsize)) {
		setDensity(0);
		setFriction(0.0f);
		setFixedRotation(true);

		return true;
	}
	return false;
}

#pragma mark - 
#pragma mark Physics Methods

/** Creates new fixtures for this body */
void Glow::createFixtures() {
	if (_body == nullptr) {
		return;
	}
	CapsuleObstacle::createFixtures();

}

/** Releases the fixtures for this body, reseting the shape */
void Glow::releaseFixtures() {
	if (_body != nullptr) {
		return;
	}

	CapsuleObstacle::releaseFixtures();
}

/** Disposes all resources and assets of this enemy model */
void Glow::dispose() {
	_core = nullptr;
	_node = nullptr;
	_sensorNode = nullptr;
}

/** Sets the vertical movement of the enemy*/
void Glow::setVX(float value) {
	_body->SetLinearVelocity(b2Vec2(value, _body->GetLinearVelocity().y));
}

/** Applies the force to the body of this enemy */

void Glow::applyForce() {
	if (!isEnabled()) {
		return;
	}
}

void Glow::update(float dt) {
	CapsuleObstacle::update(dt);
	if (_node != nullptr) {
		_node->setPosition(getPosition() * _drawScale);
		_node->setAngle(getAngle());
	}
}

#pragma mark -
#pragma mark Scene Graph Methods
/** Redraws outline of physics fixtures*/
void Glow::resetDebug() {
	CapsuleObstacle::resetDebug();
	float w = GLOW_SSHRINK * _dimension.width;
	float h = SENSOR_HEIGHT;
	Poly2 poly(Rect(-w / 2.0f, -h / 2.0f, w, h));

	_sensorNode = scene2::WireNode::allocWithTraversal(poly, poly2::Traversal::INTERIOR);
	_sensorNode->setColor(DEBUG_COLOR);
	_sensorNode->setPosition(Vec2(_debug->getContentSize().width / 2.0f, 0.0f));
	_debug->addChild(_sensorNode);
}
