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
#pragma mark Base Enemy Model

/**
* The base enemy model for Liminal Spirit
*
*/
class BaseEnemyModel : public cugl::physics2::CapsuleObstacle {

protected:
	/** Health */
	int _health;
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

	/** Initializes a new enemy at origin */
	virtual bool init() override { return init(cugl::Vec2::ZERO, cugl::Size(1, 1), 1.0f); }

	/** Initializes a new enemy at the given position */
	virtual bool init(const cugl::Vec2 pos) override { return init(pos, cugl::Size(1, 1), 1.0f); }

	/** Initializes a new enemy at the given position with given size*/
	virtual bool init(const cugl::Vec2 pos, const cugl::Size size) override {
		return init(pos, size, 1.0f);
	}

	/** Base init function */
	virtual bool init(const cugl::Vec2& pos, const cugl::Size& size, float scale);

#pragma mark - 
#pragma mark Static Constructors
	/** Allocates a new enemy at the origin */
	static std::shared_ptr<BaseEnemyModel> alloc() {
		std::shared_ptr<BaseEnemyModel> result = std::make_shared<BaseEnemyModel>();
		return (result->init() ? result : nullptr);
	}

	static std::shared_ptr<BaseEnemyModel> alloc(const cugl::Vec2& pos) {
		std::shared_ptr<BaseEnemyModel> result = std::make_shared<BaseEnemyModel>();
		return (result->init(pos) ? result : nullptr);
	}

	static std::shared_ptr<BaseEnemyModel> alloc(const cugl::Vec2& pos, const cugl::Size& size) {
		std::shared_ptr<BaseEnemyModel> result = std::make_shared<BaseEnemyModel>();
		return (result->init(pos, size) ? result : nullptr);
	}

	static std::shared_ptr<BaseEnemyModel> alloc(const cugl::Vec2& pos, const cugl::Size& size, float scale) {
		std::shared_ptr<BaseEnemyModel> result = std::make_shared<BaseEnemyModel>();
		return (result->init(pos, size, scale) ? result : nullptr);
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

	/** Returns the name of the ground sensor */
	std::string* getSensorName() { return &_sensorName; }

#pragma mark - 
#pragma mark Physics Methods
	/**Creates and adds the physics body(s) to the world */
	void createFixtures() override;

	/** Releases the fixtures of this body(s) from the world */
	void releaseFixtures() override;
	/** Updates the object's physics state (not game logic) */
	void update(float dt) override;
	/** Applies the force of the body of this enemy */
	void applyForce();

};

#endif /*  __BASE_ENEMY_MODEL_H__ */
