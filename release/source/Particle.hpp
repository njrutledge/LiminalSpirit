//
//  Particle.hpp
//  LiminalSpirit
//
//  Created by Alex Lee on 3/24/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
// 
// This class contains basic information and performs operations on particles in the system.
//

#ifndef __PARTICLE_HPP__
#define __PARTICLE_HPP__

#include <cugl/cugl.h>
#include <cugl/physics2/CUCapsuleObstacle.h>

using namespace cugl;
#pragma mark -
#pragma mark Drawing Constants
/** ID for the sensor */
#define PARTICLE_SENSOR_NAME "particlesensor"

#pragma mark -
#pragma mark Particle Model

/** 
* The particle model for Liminal Spirit
*
*/

class Particle : public physics2::CapsuleObstacle {
protected:
	/** Name of sensor */
	std::string _sensorName;
	/** Debug Sensor */
	std::shared_ptr<cugl::scene2::WireNode> _sensorNode;

	/** Scene Graph Node */
	std::shared_ptr<cugl::scene2::SceneNode> _node;
	/** Draw Scale*/
	float _drawScale;

	/** Redraws outline of physics fixtures */
	virtual void resetDebug() override;
public:
	Vec2 position;
	Vec2 velocity;
	Color4 color;
	Size pSize;
	float angle;
	float weight;
	float life;
	float density;

	/** Creates a degenerate Particle */
	Particle() : CapsuleObstacle(), _sensorName(PARTICLE_SENSOR_NAME) {}

	/**Destroys this Particle, releasing all resources */
	virtual ~Particle(void) { dispose(); }

	/** Disposes all resources and assets of this Particle */
	void dispose();

	/** Base init function */
	virtual bool init(const Vec2& pos, const Size&, float scale);

#pragma mark -
#pragma mark Static Constructors
	/** Allocates a particle*/
	static std::shared_ptr<Particle> alloc(const Vec2& pos, const Size& size, float scale) {
		std::shared_ptr<Particle> result = std::make_shared<Particle>();
		return (result->init(pos, size, scale) ? result : nullptr);
	}

#pragma mark -
#pragma mark Scene Node
	/** Returns the scene graph node representing this particle*/
	const std::shared_ptr<scene2::SceneNode>& getScene() const { return _node; }

	/** Sets the scene graph node representing this particle*/
	void setSceneNode(const std::shared_ptr<scene2::SceneNode>& node) {
		_node = node;
		_node->setPosition(getPosition() * _drawScale);
	}

#pragma mark -
#pragma mark Physics Methods
	/**Creates and adds the physics body(s) to the world */
	void createFixtures() override;

	/** Sets the vertical velocity of the enemy */
	void setVX(float value) override;

	/** Releases the fixtures of this body(s) from the world */
	void releaseFixtures() override;
	/** Updates the object's physics state (not game logic) */
	void update(float dt) override;
	/** Applies the force of the body of this glow */
	void applyForce();

};

#endif /* Particle Model*/