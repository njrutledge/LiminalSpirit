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
#include "Glow.hpp"

#pragma mark - 
#pragma mark Drawing Constants
/** Texture for the enemy*/
#define ENEMY_TEXTURE "enemy"
#define ENEMY_TEXTURE2 "enemy2"
/** ID for the sensor*/
#define ENEMY_SENSOR_NAME "enemysensor"
/** healthbar show time */
#define HEALTH_SHOWTIME 3.0f

#pragma mark -
#pragma mark Enemy Properties
struct EnemyProperties {
	int health;
	float vspeed;
	float hspeed;
	float attackCooldown;
	float attackRadius;
	float density;
    int damage;
	std::string name;
};

#pragma mark - 
#pragma mark Base Enemy Model

/**
* The base enemy model for Liminal Spirit
*
*/
class BaseEnemyModel : public cugl::physics2::CapsuleObstacle {

public:

	enum AttackType {
		p_melee,
		p_dash,
		p_range,
		p_exp_package,
		p_exp,
		e_melee,
		e_range
	};

protected:
	/** Health */
	int _health;

	/** max health of this enemy */
	int _maxhealth;

	/** True if the enemy is on the ground */
	bool _isGrounded;

	/** True if the enemy is currently attacking */
	bool _isAttacking;

	/** True if the enemy has released their attack */
	bool _completedAttack;

	/** The vertical speed of the enemy */
	float _verticalSpeed;

	/** The horizontal speed of the enemy */
	float _horizontalSpeed;

	/** Current time (in ms) since intializing most recent attack */
	float _timePast;

	/** The cooldown for attacking (in seconds) */
	float _attackCooldown;

	/** The attack radius for the enemy*/
	float _attackRadius;

	/** The density of the enemy*/
	float _density;
    
    /** The damage of the enemy */
    int _damage;

	/** True if the enemy has had line of sight of the player */
	bool _hasSeenPlayer;
    
    /** True if the enemy is invincible */
    bool _hasInvincibility;
    
    /** Duration of enemy invincibility */
    float _invincibilityTime;

	/** The type of attack the enemy was last damaged by*/
	AttackType _lastDamagedBy;

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

	/** Pointer to the enemy's Glow*/
	std::shared_ptr<Glow> _glow;

	/** Timer for the enemy's idle animation */
	float _idleTime;

	/** Timer for the enemy's attack animation */
	float _attackAnimationTime;

	/** Redraws outline of physics fixtures */
	virtual void resetDebug() override;

	/** Size of the enemy image */
	cugl::Size _size;

	float _healthTimer;
    
    /** If the enemy is from a spawner, index of the spawner; otherwise, -1.*/
    int _spawnerIndex;

	/** True if the enemy just spawned another enemy (should only apply to spawner), false otherwise */
	bool _spawned;

	/** The amount of damage last taken by the enemy */
	int _lastDamageAmount;
    
    bool _isJumping;
    bool _isFalling;

	bool _playedDamagedParticles;

public:

#pragma mark Hidden Constructors
	/** Creates a degenerate Enemy */
	BaseEnemyModel() : CapsuleObstacle(), _sensorName(ENEMY_SENSOR_NAME) { }

	/**Destroys this Base Enemy Model, releasing all resources */
	virtual ~BaseEnemyModel(void) { dispose(); }

	/**Disposes all resources and assets of this Enemy Model */
	void dispose();

	/** Base init function */
	virtual bool init(const cugl::Vec2& pos, const cugl::Size& rsize, const cugl::Size& size, float scale, EnemyProperties props);

	float getRadius() {
		return _sensorFixture->GetShape()->m_radius;
	}

	float getMirrorDistanceModifier() {
		auto pause = getDimension();
		return getDimension().width;
	}



#pragma mark - 
#pragma mark Static Constructors
	/** Allocates a new enemy, using placeholder values, DO NOT MAKE IN-GAME ENEMIES USING THIS MODEL */
	static std::shared_ptr<BaseEnemyModel> alloc(const cugl::Vec2& pos, const cugl::Size& realSize, const cugl::Size& size, float scale) {
		std::shared_ptr<BaseEnemyModel> result = std::make_shared<BaseEnemyModel>();
		return (result->init(pos, realSize, size, scale, { 10, 1.0f, 1.0f, 600, 1, 1.0f, 1, "base" }) ? result : nullptr);
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

	/** Sets the enemy health, reset healthbar timer */
	void setHealth(int value);
    
    /**Returns the damage of the enemy's attack */
    int getAttackDamage() const { return _damage; }

	/** Returns true if the enemy is on the ground*/
	bool isGrounded() const { return _isGrounded; }

	/** Sets whether the enemy is on the ground*/
	void setGrounded(bool value) { _isGrounded = value; }

	/** Returns true if the enemy is attacking*/
	bool isAttacking() const { return _isAttacking; }

	/** Sets whether the enemy is attacking*/
	void setIsAttacking(bool value) { _isAttacking = value; }

	/** Returns true if the enemy has completed their attack*/
	bool attackIsCompleted() { return _completedAttack; }

	/** Sets whether the enemy has completed their attack */
	void setAttackCompleted(bool value) { _completedAttack = value; }

	/** Returns the vertical speed of the enemy*/
	float getVerticalSpeed() const { return _verticalSpeed; }

	/** Sets the vertical speed of the enemy */
	void setVerticalSpeed(bool value) { _verticalSpeed = value; }

	/** Returns the horizontal speed of the enemy*/
	float getHorizontalSpeed() const { return _horizontalSpeed; }

	/** Sets the horizontal speed of the enemy*/
	void setHorizontalSpeed(bool value) { _horizontalSpeed = value; }

	/** Returns the time since the last attack*/
	float getTimePast() const { return _timePast; }

	/** Time since last attack */
	void setTimePast(float value) { _timePast = value; }

	/** Gets the name of the enemy*/
	std::string getName() { return _enemyName; }

	/** Returns the attack radius of the Lost*/
	float getAttackRadius() { return _attackRadius; }

	/** Returns the attack cooldown */
	float getAttackCooldown() const { return _attackCooldown; }

	/** Returns true if this enemy has seen the player */
	bool getHasSeenPlayer() { return _hasSeenPlayer; }

	/** Sets to true if the enemy has seen the player */
	void setHasSeenPlayer(bool value) { _hasSeenPlayer = value; }
    
    /** Returns true if this enemy has invincibility */
    bool getInvincibility() { return _hasInvincibility; }

    /** Sets to true if the enemy is invincible */
    void setInvincibility(bool value) { _hasInvincibility = value; }
    
    /** Returns amount of invincibility time remaining */
    float getInvincibilityTimer() { return _invincibilityTime; }

	/** Returns the amount of damage last taken */
	int getLastDamageAmount() { return _lastDamageAmount; }

	/** Sets the amount of damage last taken */
	void setLastDamageAmount(int value) { _lastDamageAmount = value; }

	/** Sets the type of attack the enemy was just damaged by*/
	void setLastDamagedBy(AttackType value) { _lastDamagedBy = value; }

	/** Returns type of attack the enemy was last damaged by */
	float getLastDamagedBy() { return _lastDamagedBy; }
    
    /** Sets invincibility time (for after getting hit) */
    void setInvincibilityTimer(float value) { _invincibilityTime = value; }

	/** Returns the pointer to the enemy's glow */
	std::shared_ptr<Glow> getGlow() { return _glow; }

	/** Returns the amount of time remaining for next idle animation frame */
	float getIdleAnimationTimer() { return _idleTime; }

	/** Sets the idle animation time */
	void setIdleAnimationTimer(float value) { _idleTime = value; }

	/** Returns the amount of time remaining for next attack animation frame */
	float getAttackAnimationTimer() { return _attackAnimationTime; }

	/** Sets the attack animation time */
	void setAttackAnimationTimer(float value) { _attackAnimationTime = value; }

	/** Sets the pointer to the enemy's glow */
	void setGlow(std::shared_ptr<Glow> glow) { _glow = glow; }

	/** Sets whether or not the enemy has played their damage particles */
	void setPlayedDamagedParticle(bool didIt) { _playedDamagedParticles = didIt; }

	/** Gets whether or not the enemy has played their damage particles */
	bool getPlayedDamagedParticle() { return _playedDamagedParticles; }

	/** Returns the name of the ground sensor */
	std::string* getSensorName() { return &_sensorName; }
    
	/** Gets the index the next enemy to be spawned from the spawner's list */
    int getSpawnerInd() {return _spawnerIndex; }
    
	/** Sets the index the next enemy to be spawned from the spawner's list */
    void setSpawnerInd(int index) {_spawnerIndex = index; }

	/** Gets the spawned field of the enemy */
	int getSpawned() { return _spawned; }

	/** Sets the spawned field of the enemy */
	void setSpawned(int index) { _spawned = index; }
    
    bool isJumping() {return _isJumping; }
    void setJumping(bool value) {_isJumping = value; }
    bool isFalling() {return _isFalling; }
    void setFalling(bool value) {_isFalling = value; }
    
    /** Gets the desired healthbar scaling for the enemy */
    float getHealthBarScale();

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
	/** Applies the force of the body of this enemy */
	void applyForce();

};

#endif /*  __BASE_ENEMY_MODEL_H__ */
