//
//  PlayerModel.h
//  Liminal Spirit Game
//
//  This class tracks the state of the player
// 
//


#ifndef __PLAYER_MODEL_H__
#define __PLAYER_MODEL_H__
#include <cugl/cugl.h>
#include <cugl/physics2/CUBoxObstacle.h>
#include <cugl/physics2/CUCapsuleObstacle.h>
#include <cugl/scene2/graph/CUWireNode.h>

#pragma mark - 
#pragma mark Drawing Constants
/** Texture for the enemy*/
#define PLAYER_TEXTURE "player"
/** ID for the sensor*/
#define PLAYER_SENSOR_NAME "playersensor"

#pragma mark -
#pragma mark Physics Constants
/** The factor to multiply by the input */
#define PLAYER_FORCE      20.0f
/** The amount to slow the character down */
#define PLAYER_DAMPING    10.0f
/** The maximum character speed */
#define PLAYER_MAXSPEED   5.0f

#pragma mark - 
#pragma mark Player Model

/**
* The player model for Liminal Spirit
*
*/
class PlayerModel : public cugl::physics2::CapsuleObstacle {

protected:
	/** Health */
	float _health;
	/** Which direction the player is facing */
	bool _faceRight;
	/** The current horizontal movement of the player */
	float _movement;
	/** Whether we are actively jumping*/
	bool _isJumping;
	/** Whether the player's feet is touching the ground*/
	bool _isGrounded;
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
	/** Creates a degenerate Player */
	PlayerModel() : CapsuleObstacle(), _sensorName(PLAYER_SENSOR_NAME) { }

	/**Destroys this Player Model, releasing all resources */
	virtual ~PlayerModel(void) { dispose(); }

	/**Disposes all resources and assets of this Player Model */
	void dispose();
    
    /**Resets player */
    void reset(const cugl::Vec2 pos);

	/** Initializes a new player at origin */
	virtual bool init() override { return init(cugl::Vec2::ZERO, cugl::Size(1, 1), 1.0f); }

	/** Initializes a new player at the given position */
	virtual bool init(const cugl::Vec2 pos) override { return init(pos, cugl::Size(1, 1), 1.0f); }

	/** Initializes a new player at the given position with given size*/
	virtual bool init(const cugl::Vec2 pos, const cugl::Size size) override {
		return init(pos, size, 1.0f);
	}

	/** Base init function (includes scale) */
	virtual bool init(const cugl::Vec2& pos, const cugl::Size& size, float scale);

#pragma mark - 
#pragma mark Static Constructors
	/** Allocates a new player at the origin */
	static std::shared_ptr<PlayerModel> alloc() {
		std::shared_ptr<PlayerModel> result = std::make_shared<PlayerModel>();
		return (result->init() ? result : nullptr);
	}

	static std::shared_ptr<PlayerModel> alloc(const cugl::Vec2& pos) {
		std::shared_ptr<PlayerModel> result = std::make_shared<PlayerModel>();
		return (result->init(pos) ? result : nullptr);
	}

	static std::shared_ptr<PlayerModel> alloc(const cugl::Vec2& pos, const cugl::Size& size) {
		std::shared_ptr<PlayerModel> result = std::make_shared<PlayerModel>();
		return (result->init(pos, size) ? result : nullptr);
	}

	static std::shared_ptr<PlayerModel> alloc(const cugl::Vec2& pos, const cugl::Size& size, float scale) {
		std::shared_ptr<PlayerModel> result = std::make_shared<PlayerModel>();
		return (result->init(pos, size, scale) ? result : nullptr);
	}

#pragma mark -
#pragma mark Scene Node
	/** Returns the scene graph node representing the player*/
	const std::shared_ptr<cugl::scene2::SceneNode>& getSceneNode() const { return _node; }

	/** Sets the scene graph node representing the player */
	void setSceneNode(const std::shared_ptr<cugl::scene2::SceneNode>& node) {
		_node = node;
		_node->setPosition(getPosition() * _drawScale);
	}

#pragma mark - 
#pragma mark Attribute Properties

	/**Returns the health of the player */
	float getHealth() const { return _health; }

	/** Sets the player health */
	void setHealth(float value) { _health = value; }

	/** Returns the horizontal movement of the player*/
	float getMovement() const { return _movement; }

	/** Sets the horizontal movement of the player */
	void setMovement(float value);

	/** Returns true if the player is facing right */
	bool isFacingRight() const { return _faceRight; }
    
    /** Sets whetherthe player is facing right */
    void setFacingRight(bool value) { _faceRight = value; }

	/** Returns true if the player is actively jumping*/
	bool isJumping() const { return _isJumping;  }

	/** Sets whether the player is actively jumping*/
	void setJumping(bool value) { _isJumping = value;  }

	/** Returns true if the player is on the ground*/
	bool isGrounded() const { return _isGrounded;  }

	/** Sets whether the player is on the ground*/
	void setGrounded(bool value) { _isGrounded = value;  }

	/** Set X velocity */
	void setVX(float value) override;


	/**
	 * Returns how much force to apply to get the player moving
	 *
	 * Multiply this by the input to get the movement value.
	 *
	 * @return how much force to apply to get the player moving
	 */
	float getForce() const { return PLAYER_FORCE; }

	/**
	 * Returns ow hard the brakes are applied to get a player to stop moving
	 *
	 * @return ow hard the brakes are applied to get a player to stop moving
	 */
	float getDamping() const { return PLAYER_DAMPING; }

	/**
	 * Returns the upper limit on player left-right movement.
	 *
	 * This does NOT apply to vertical movement.
	 *
	 * @return the upper limit on player left-right movement.
	 */
	float getMaxSpeed() const { return PLAYER_MAXSPEED; }

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

#endif /*  __PLAYER_MODEL_H__ */

