//
//  BaseEnemyModel.h
//  Liminal Spirit Game
//
//  This class tracks the state of the enemy(s)
// 
//


#ifndef __BASE_ENEMY_MODEL_H__
#define __BASE_ENEMY_MODEL_H__
#include <cugl/cugl.h>
#include <cugl/physics2/CUBoxObstacle.h>
#include <cugl/physics2/CUCapsuleObstacle.h>
#include <cugl/scene2/graph/CUWireNode.h>

#pragma mark - 
#pragma mark Drawing Constants
/** Texture for the enemy*/
#define ENEMY_TEXTURE "enemy"
/** ID for the sensor*/
#define SENSOR_NAME "enemysensor"

#pragma mark -
#pragma mark Enemy Properties
struct EnemyProperties {
	int health;
	float vspeed;
	float hspeed;
	int attackCooldown;
	float attackRadius;
	float density;
	std::string name;
};

#pragma mark - 
#pragma mark Base Enemy Model

/**
* The base enemy model for Liminal Spirit
*
*/
class BaseEnemyModel : public cugl::physics2::CapsuleObstacle {
protected:
	/** Health */
	int _health;

	/** True if the enemy is on the ground */
	bool _isGrounded; 

	/** True if the enemy is currently attacking */
	bool _isAttacking;

	/** The vertical speed of the enemy */
	float _verticalSpeed;

	/** The horizontal speed of the enemy */
	float _horizontalSpeed;

	/** Current time (in frames) since intializing most recent attack */
	int _framesPast;

	/** The cooldown for attacking (in frames) */
	int _attackCooldown;

	/** The attack radius for the enemy*/
	float _attackRadius; 

	/** The density of the enemy*/
	float _density;

	/** Enemy name*/
	std::string _enemyName;

	/** Ground/feet sensor */
	b2Fixture* _sensorFixture;

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

#pragma mark Hidden Constructors
	/** Creates a degenerate Enemy */
	BaseEnemyModel() : CapsuleObstacle(), _sensorName(SENSOR_NAME) { }

	/**Destroys this Base Enemy Model, releasing all resources */
	virtual ~BaseEnemyModel(void) { dispose(); }

	/**Disposes all resources and assets of this Enemy Model */
	void dispose();

	/** Base init function */
	virtual bool init(const cugl::Vec2& pos, const cugl::Size& size, float scale, EnemyProperties props);

	float getRadius() {
		return _sensorFixture->GetShape()->m_radius;
	}

#pragma mark - 
#pragma mark Static Constructors
	/** Allocates a new enemy, using placeholder values, DO NOT MAKE IN-GAME ENEMIES USING THIS MODEL */
	static std::shared_ptr<BaseEnemyModel> alloc(const cugl::Vec2& pos, const cugl::Size& size, float scale) {
		std::shared_ptr<BaseEnemyModel> result = std::make_shared<BaseEnemyModel>();
		return (result->init(pos, size, scale, { 10, 1.0f, 1.0f, 600, 1, 1.0f, "base" }) ? result : nullptr);
	}

#pragma mark -
#pragma mark Scene Node
	/** Returns the scene graph node representing this enemy*/
	const std::shared_ptr<cugl::scene2::SceneNode>& getSceneNode() const { return _node; }

	/** Sets the scene graph node representing this enemy */
	void setSceneNode(const std::shared_ptr<cugl::scene2::SceneNode>& node) {
		_node = node;
		_node->setPosition(getPosition() * _drawScale);
	}

#pragma mark - 
#pragma mark Attribute Properties

	/**Returns the health of the enemy */
	int getHealth() const { return _health; }

	/** Sets the enemy health */
	void setHealth(int value) { _health = value; }

	/** Returns true if the enemy is on the ground*/
	bool isGrounded() const { return _isGrounded; }

	/** Sets whether the enemy is on the ground*/
	void setGrounded(bool value) { _isGrounded = value; }

	/** Returns true if the enemy is attacking*/
	bool isAttacking() const { return _isAttacking; }

	/** Sets whether the enemy is attacking*/
	void setIsAttacking(bool value) { _isAttacking = value; }

	/** Returns the vertical speed of the enemy*/
	float getVerticalSpeed() const { return _verticalSpeed; }

	/** Sets the vertical speed of the enemy */
	void setVerticalSpeed(bool value) { _verticalSpeed = value; }

	/** Returns the horizontal speed of the enemy*/
	float getHorizontalSpeed() const { return _horizontalSpeed; }

	/** Sets the horizontal speed of the enemy*/
	void setHorizontalSpeed(bool value) { _horizontalSpeed = value; }

	/** Returns the frames since the last attack*/
	float getFramesPast() const { return _framesPast; }

	/** Sets the horizontal speed of the enemy*/
	void setFramesPast(int value) { _framesPast = value; }

	/** Gets the name of the enemy*/
	std::string getName() { return _enemyName; }

	/** Returns the attack radius of the Lost*/
	float getAttackRadius() { return _attackRadius; }

	/** Returns the attack cooldown */
	float getAttackCooldown() const { return _attackCooldown; }

	/** Returns the name of the ground sensor */
	std::string* getSensorName() { return &_sensorName; }

#pragma mark - 
#pragma mark Physics Methods
	/**Creates and adds the physics body(s) to the world */
	void createFixtures() override;

	/** Sets the vertical velocity of the enemy */
	void setVX(float value);

	/** Releases the fixtures of this body(s) from the world */
	void releaseFixtures() override;
	/** Updates the object's physics state (not game logic) */
	void update(float dt) override;
	/** Applies the force of the body of this enemy */
	void applyForce();

};

#endif /*  __BASE_ENEMY_MODEL_H__ */
