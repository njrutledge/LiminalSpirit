//
//  Particle.cpp
//  LiminalSpirit
//
//  Created by Alex Lee on 3/24/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//
#include "Particle.hpp"
#include <cugl/scene2/graph/CUPolygonNode.h>
#include <cugl/scene2/graph/CUTexturedNode.h>
#include <cugl/assets/CUAssetManager.h>

using namespace cugl;
#pragma mark - 
#pragma mark Physics Constants
/** Amount to shrink the body fixture vertically*/
#define PARTICLE_VSHRINK 0.95f
/** Amount the shrink the body fixture by horizontally*/
#define PARTICLE_HSHRINK 0.7f
/** */
#define PARTICLE_SSHRINK 0.6f
/**Height of the sensor */
#define SENSOR_HEIGHT 01.f
/** Debug color for sensor */
#define DEBUG_COLOR Color4::RED

/** Initializes a single particle at given position with given size and scale*/
bool Particle::init(const Vec2& pos, const Size& size, float scale) {
	Size nsize = size;
	nsize.width *= PARTICLE_HSHRINK;
	nsize.height *= PARTICLE_VSHRINK;
	_drawScale = scale;
	position = pos;
	velocity = Vec2();
	color = Color4::BLACK;
	pSize = size;
	angle = 0.f;
	weight = 0.f;
	density = 0.f;
	life = 0.f;

	if (CapsuleObstacle::init(pos, nsize)) {
		setDensity(density);
		setFriction(0.0f);
		setFixedRotation(true);
		b2Filter filter = b2Filter();
		filter.categoryBits = 0b0;
		filter.maskBits = 0b000000;
		setFilterData(filter);
		
		return true;
	}
	return false;
}

#pragma mark -
#pragma mark Physics Methods

/** Creates new fixtures for this body */
void Particle::createFixtures() {
	if (_body == nullptr) {
		return;
	}
	CapsuleObstacle::createFixtures();

}

/** Releases the fixtures for this body, reseting the shape */
void Particle::releaseFixtures() {
	if (_body != nullptr) {
		return;
	}

	CapsuleObstacle::releaseFixtures();
}

/** Disposes all resources and assets of this enemy model */
void Particle::dispose() {
	_core = nullptr;
	_node = nullptr;
	_sensorNode = nullptr;
}

/** Sets the vertical movement of the enemy*/
void Particle::setVX(float value) {
	_body->SetLinearVelocity(b2Vec2(value, _body->GetLinearVelocity().y));
}

/** Applies the force to the body of this enemy */

void Particle::applyForce() {
	if (!isEnabled()) {
		return;
	}
}

void Particle::update(float dt) {
	CapsuleObstacle::update(dt);
	if (_node != nullptr) {
		_node->setPosition(getPosition() * _drawScale);
		_node->setAngle(getAngle());
	}
}

#pragma mark -
#pragma mark Scene Graph Methods
/** Redraws outline of physics fixtures*/
void Particle::resetDebug() {
	CapsuleObstacle::resetDebug();
	float w = PARTICLE_SSHRINK * _dimension.width;
	float h = SENSOR_HEIGHT;
	Poly2 poly(Rect(-w / 2.0f, -h / 2.0f, w, h));

	_sensorNode = scene2::WireNode::allocWithTraversal(poly, poly2::Traversal::INTERIOR);
	_sensorNode->setColor(DEBUG_COLOR);
	_sensorNode->setPosition(Vec2(_debug->getContentSize().width / 2.0f, 0.0f));
	_debug->addChild(_sensorNode);
}