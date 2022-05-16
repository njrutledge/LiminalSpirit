//
//  PlayerModel.cpp
//  Liminal Spirit Game
//
//  This class tracks the state of the player
// 
//
#include "PlayerModel.h"
#include <cugl/scene2/graph/CUPolygonNode.h>
#include <cugl/scene2/graph/CUTexturedNode.h>
#include <cugl/assets/CUAssetManager.h>

#define SIGNUM(x)  ((x > 0) - (x < 0))

#pragma mark -
#pragma mark Physics Constants
/** Amount to shrink the body fixture vertically*/
#define PLAYER_VSHRINK 0.95f
/** Amount the shrink the body fixture by horizontally*/
#define PLAYER_HSHRINK 0.7f
/** */
#define PLAYER_SSHRINK 0.6f
/**Height of the sensor */
#define SENSOR_HEIGHT 01.f
/**Density of character*/
#define PLAYER_DENSITY 3.0f
/** Impulse of the player jump */
#define PLAYER_JUMP 240.0f
/** Debug color for sensor */
#define DEBUG_COLOR Color4::RED


#define PLAYER_MASS 10.0f

using namespace cugl;

/** Initializes the player at the given position*/
bool PlayerModel::init(const cugl::Vec2& pos, const cugl::Size& size, float scale) {
    Size nsize = size;
    nsize.width *= PLAYER_HSHRINK;
    nsize.height *= PLAYER_VSHRINK;
    _drawScale = scale;

    if (CapsuleObstacle::init(pos, nsize)) {
        setDensity(PLAYER_DENSITY);
        setFriction(0.0f);      // HE WILL STICK TO WALLS IF YOU FORGET
        setFixedRotation(true); // OTHERWISE, HE IS A WEEBLE WOBBLE
        b2Filter filter = b2Filter();
        filter.categoryBits = 0b1;
        filter.maskBits = 0b101100;
        setFilterData(filter);
        // Gameplay attributes
        _health = PLAYER_HEALTH;
        _faceRight = true;
        _isInvincible = false;
        _isStunned = false;
        _isGrounded = true;
        _isFloored = false;
        _isJumping = false;
        _isMovingUp = false;
        _hasJustLanded = false;
        _isDashing = false;
        _rangedAttackRight = true;
        _isStunned = false;
        _movement = 0;
        _walkTime = 0;
        _idleTime = 0;
        _jumpTime = 0;
        _invincibilityTime = 0;
        _postStunInvincibilityTimer = 0;

        return true;
    }
    return false;
}


#pragma mark -
#pragma mark Attribute Properties

/**
 * Sets left/right movement of this character.
 *
 * This is the result of input times player force.
 *
 * @param value left/right movement of this character.
 */
void PlayerModel::setMovement(float value) {
    _movement = value;
    bool face = _movement > 0;
    if (_movement == 0 || _faceRight == face) {
        return;
    }

    // Change facing
    scene2::TexturedNode* image = dynamic_cast<scene2::TexturedNode*>(_node.get());
    if (image != nullptr) {
        image->flipHorizontal(!image->isFlipHorizontal());
    }
    _faceRight = face;
}

#pragma mark -
#pragma mark Physics Methods

/** Creates new fixtures for the body and defines shape*/
void PlayerModel::createFixtures() {
    if (_body == nullptr) {
        return;
    }

    CapsuleObstacle::createFixtures();
    b2FixtureDef sensorDef;
    sensorDef.density = PLAYER_DENSITY;
    sensorDef.isSensor = true;

    // Sensor dimensions
    b2Vec2 corners[4];
    corners[0].x = -PLAYER_SSHRINK * getWidth() / 2.0f;
    corners[0].y = (-getHeight() + SENSOR_HEIGHT) / 2.0f;
    corners[1].x = -PLAYER_SSHRINK * getWidth() / 2.0f;
    corners[1].y = (-getHeight() - SENSOR_HEIGHT) / 3.0f;
    corners[2].x = PLAYER_SSHRINK * getWidth() / 2.0f;
    corners[2].y = (-getHeight() - SENSOR_HEIGHT) / 3.0f;
    corners[3].x = PLAYER_SSHRINK * getWidth() / 2.0f;
    corners[3].y = (-getHeight() + SENSOR_HEIGHT) / 2.0f;

    b2PolygonShape sensorShape;
    sensorShape.Set(corners, 4);

    sensorDef.shape = &sensorShape;
    sensorDef.userData.pointer = reinterpret_cast<uintptr_t>(getSensorName());
    _sensorFixture = _body->CreateFixture(&sensorDef);
    setMass(PLAYER_MASS);
}

/** Releases the fixtures for this body, resets the shape*/
void PlayerModel::releaseFixtures() {
    if (_body != nullptr) {
        return;
    }

    CapsuleObstacle::releaseFixtures();
    if (_sensorFixture != nullptr) {
        _body->DestroyFixture(_sensorFixture);
        _sensorFixture = nullptr;
    }
}

/** Disposes all resources and assets of this PlayerModel*/
void PlayerModel::dispose() {
    _core = nullptr;
    _node = nullptr;
    _sensorNode = nullptr;
}

void PlayerModel::reset(Vec2 pos) {
    setPosition(pos);
    _health = PLAYER_HEALTH;
    _faceRight = true;
    _movement = 0.0f;
    _body->SetLinearVelocity(b2Vec2(0,0));
    _isJumping = false;
    _isMovingUp = false;
    _hasJustLanded = false;
    _isGrounded = true;
    _isInvincible = false;
    _invincibilityTime = 0;
    _postStunInvincibilityTimer = 0;
    _walkTime = 0;
    _idleTime = 0;
    _jumpTime =  0;
    _isStunned = false;
}

void PlayerModel::setVX(float value) {
    _body->SetLinearVelocity(b2Vec2(value, _body->GetLinearVelocity().y));
}

float PlayerModel::getVX() {
    return _body->GetLinearVelocity().x;
}

void PlayerModel::applyForce() {
    if (!isEnabled()) {
        return;
    }

    // Don't want to be moving. Damp out player motion
    if (getMovement() == 0.0f) {
        if (isGrounded()) {
            // Instant friction on the ground
            b2Vec2 vel = _body->GetLinearVelocity();
            vel.x = 0; // If you set y, you will stop a jump in place
            _body->SetLinearVelocity(vel);
        }
        else {
            // Damping factor in the air
            b2Vec2 force(-getDamping() * getVX(), 0);
            _body->ApplyForce(force, _body->GetPosition(), true);
        }
    }

    // Velocity too high, clamp it
    if (fabs(getVX()) >= getMaxSpeed()) {
        setVX(SIGNUM(getVX()) * getMaxSpeed());
    }
    else {
        b2Vec2 force(getMovement(), 0);
        _body->ApplyForce(force, _body->GetPosition(), true);
    }

    // Jump!
    if (isJumping() && isGrounded()) {
        b2Vec2 force(0, PLAYER_JUMP);
        _body->ApplyLinearImpulse(force, _body->GetPosition(), true);
    }
}

/** Updates the object's physics state (outside of game logic) */
void PlayerModel::update(float dt) {
    CapsuleObstacle::update(dt);

    if (_node != nullptr) {
        _node->setPosition(getPosition() * _drawScale);
        _node->setAngle(getAngle());
    }

    b2Filter filter = getFilterData();
    if (getVY() > 0.1 || _dropTime > 0) {
        filter.maskBits = 0b101000;
    }
    else {
        filter.maskBits = 0b101100;
    }
    setFilterData(filter);
    _dropTime -= dt;
}

void PlayerModel::applyAerialSustain() {
    if (!isGrounded()) {
//        b2Vec2 vel(_body->GetLinearVelocity().x, 0);
//        _body->SetLinearVelocity(vel);
        b2Vec2 force(0, 50 * (1 - (clampf(_body->GetLinearVelocity().y, 0.0f, 24.0f) / 24.0f)));
        _body->ApplyLinearImpulse(force, _body->GetPosition(), true);
    }
}

#pragma mark -
#pragma mark Scene Graph Methods
/**
 * Redraws the outline of the physics fixtures to the debug node
 *
 * The debug node is use to outline the fixtures attached to this object.
 * This is very useful when the fixtures have a very different shape than
 * the texture (e.g. a circular shape attached to a square texture).
 */
void PlayerModel::resetDebug() {
    CapsuleObstacle::resetDebug();
    float w = PLAYER_SSHRINK * _dimension.width;
    float h = SENSOR_HEIGHT;
    Poly2 poly(Rect(-w / 2.0f, -h / 2, w, h*5/6)); // This may no longer be accurately lined up with the model

    _sensorNode = scene2::WireNode::allocWithTraversal(poly, poly2::Traversal::INTERIOR);
    _sensorNode->setColor(DEBUG_COLOR);
    _sensorNode->setPosition(Vec2(_debug->getContentSize().width / 2.0f, 0.0f));
    _debug->addChild(_sensorNode);
}
