//
//  Glow.hpp
//  Liminal Spirit Game
//
//  This class tracks creates the glow effects and alters them depending on situation
// 
//


#ifndef __GLOW_HPP__
#define __GLOW_HPP__
#include <cugl/cugl.h>
#include <cugl/physics2/CUBoxObstacle.h>
#include <cugl/physics2/CUCapsuleObstacle.h>
#include <cugl/scene2/graph/CUWireNode.h>

#pragma mark - 
#pragma mark Drawing Constants
/** Texture for the glow*/
#define GLOW_TEXTURE "whiteGrad"
/** ID for the sensor*/
#define GLOW_SENSOR_NAME "glowsensor"

#pragma mark - 
#pragma mark Glow Model
/**
* The glow model for Liminal Spirit
*
*/
class Glow : public cugl::physics2::CapsuleObstacle {
public:
	enum MeleeState {
		first,
		h1_right,
		h2_right,
		h3_right,
		h1_left,
		h2_left,
		h3_left,
		cool,
        jump_attack
	};

protected:
	/** Name of sensor */
	std::string _sensorName;
	/** Debug Sensor */
	std::shared_ptr<cugl::scene2::WireNode> _sensorNode;

	/** Scene Graph Node */
	std::shared_ptr<cugl::scene2::SceneNode> _node;
	/** Draw Scale*/
	float _drawScale;

	/** Time passed since glow initialized */
	float _glowTime;

	/** Time passed since last animation frame */
	float _animeTime;

	/** Last Type of attack used*/
	MeleeState _attackType;
    
    /** Angle of last projectile in degrees*/
    float _attackAngle;

	/** Redraws outline of physics fixtures */
	virtual void resetDebug() override;

public:
	/** Creates a degenerate Glow */
	Glow() : CapsuleObstacle(), _sensorName(GLOW_SENSOR_NAME) { }

	/**Destroys this Glow, releasing all resources */
	virtual ~Glow(void) { dispose(); }

	/**Disposes all resources and assets of this Glow */
	void dispose();

	/** Base init function */
	virtual bool init(const cugl::Vec2& pos, const cugl::Size& size, float scale);

#pragma mark - 
#pragma mark Static Constructors
	/** Allocates a glow*/
	static std::shared_ptr<Glow> alloc(const cugl::Vec2& pos, const cugl::Size& size, float scale) {
		std::shared_ptr<Glow> result = std::make_shared<Glow>();
		return (result->init(pos, size, scale) ? result : nullptr);
	}

#pragma mark -
#pragma mark Scene Node
	/** Returns the scene graph node representing this glow*/
	const std::shared_ptr<cugl::scene2::SceneNode>& getSceneNode() const { return _node; }

	/** Sets the scene graph node representing this glow */
	void setSceneNode(const std::shared_ptr<cugl::scene2::SceneNode>& node) {
		_node = node;
		_node->setPosition(getPosition() * _drawScale);
	}

#pragma mark - 
#pragma mark Attribute Properties
	/** Returns the name of the ground sensor */
	std::string* getSensorName() { return &_sensorName; }

	/** Returns the amount of time past since the glow was initialized */
	float getGlowTimer() { return _glowTime; }

	/** Sets the amount of time past since the glow was initialized */
	void setGlowTimer(float value) { _glowTime = value; }

	/** Returns last type of attack used by melee */
	MeleeState getLastType() { return _attackType; }

	/** Sets last type of attack used by melee */
	void setLastType(MeleeState type) { _attackType = type; }
    
    /** Returns angle of last attack */
    float getAttackAngle() { return _attackAngle; }

    /** Sets angle of last attack */
    void setAttackAngle(float angle) { _attackAngle = angle; }

	/** Returns the amount of time past since the last animation frame played */
	float getAnimeTimer() { return _animeTime; }

	/** Sets the amount of time past since the last animation frame played */
	void setAnimeTimer(float value) { _animeTime = value; }
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
#endif /* Glow Model*/
