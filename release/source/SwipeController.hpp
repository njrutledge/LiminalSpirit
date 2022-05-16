//
//  SwipeController.hpp
//  LiminalSpirit
//
//  Created by Sashimi Software
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef SwipeController_hpp
#define SwipeController_hpp

#include <cugl/cugl.h>
#include <vector>
#include "InputController.hpp"

#define CHARGE_TIME 200

/**
 * This class figures out what type of swipe was drawn based on the input
 */
class SwipeController {
public:
    
    /** Enum for swipe attacks or noAttack if no swipe was done */
    enum SwipeAttack {
        upAttack,
        rightAttack,
        downAttack,
        leftAttack,
        chargedUp,
        chargedRight,
        chargedDown,
        chargedLeft,
        chargedNortheast,
        chargedNorthwest,
        chargedSouthwest,
        chargedSoutheast,
        jump,
        noAttack
    };
    
protected:
    
    /** Enum for swipe directions or none if no swipe was done */
    enum SwipeDirection {
        up,
        right,
        down,
        left,
        none
    };
    
    /**
     * Struct for the state of the left side attacks
     *
     * direction is the direction of the left swipe
     * isCharged is whether the attack is charged
     */
    struct LeftSwipeState {
        SwipeDirection direction;
        bool isCharged;

        /**
         * Constructor to initialize the left side state
         */
        void construct() {
            direction = none;
            isCharged = false;
        }
    };
    
    /**
     * Struct for the state of the right sided attacks
     *
     * direction is the direction of the right swipe
     * isCharged is whether the attack is charged
     */
    struct RightSwipeState {
        float angle;
        bool isCharged;

        /**
         * Constructor to initialize the left side state
         */
        void construct() {
            angle = 0.0f;
            isCharged = false;
        }
    };
    
    /** State of the left side swipes */
    LeftSwipeState _leftState;
    
    /** Swipe attack completed on the left side */
    SwipeAttack _leftSwipe;
    /** Swipe angle in degrees on the left side */
    float _leftAngle;
    
    /** State of the left side swipes */
    RightSwipeState _rightState;
    
    /** Swipe attack completed on the right side */
    SwipeAttack _rightSwipe;
    /** Swipe angle in degrees on the left side */
    float _rightAngle;
    
    /** Charge counter for melee (right) */
    float _cMeleeCount;
    /** Charge threshold for melee*/
    float _cMCool;
    /** Charge counter for range (left) */
    float _cRangeCount;
    /** Charge threshold for range*/
    float _cRCool;
    
    bool _lStart;
    float _lCoolStart;
    
    bool _rStart;
    float _rCoolStart;
    
    /** Time spent charging in milliseconds */
    Uint64 _leftChargingTime;
    Uint64 _rightChargingTime;
    
    /** Timestamp to get the current time for charge calculations */
    cugl::Timestamp _currTime;
    
    /**
     * Set the left sided direction
     *
     * @param s the swipe direction completed on the left side
     */
    void setLeftDirection(SwipeDirection d){
        _leftState.direction = d;
    };
    
    /**
     * Set the left sided swipe angle
     *
     * @param s the swipe angle completed on the left side
     */
    void setLeftAngle(float a){
        _leftAngle = a;
    };
    
    /**
     * Set the right sided direction
     *
     * @param s the swipe direction completed on the right side
     */
    void setRightAngle(float a){
        _rightState.angle = a;
        _rightAngle = a;
    };
    
    /**
     * Set the left sided swipe
     *
     * @param s  the swipe attack completed on the left side
     */
    void setLeftSwipe(SwipeAttack s){
        _leftSwipe = s;
    };
    
    /**
     * Set the right sided swipe
     *
     * @param s  the swipe attack completed on the right side
     */
    void setRightSwipe(SwipeAttack s){
        _rightSwipe = s;
    };
    
    /**
     * Calculates whether a finger has been pressed down long enough for a charge attack
     * and updates the state accordingly
     *
     * @param leftStartTime  the timestamp for when the left finger went down
     *
     */
    void calculateChargeAttack(cugl::Timestamp startTime, bool isLeftSidedCharge);
    
    /**
     * Charge the attack for the left side state
     */
    void chargeLeftAttack() {
        _leftState.isCharged = true;
    }
    
    /**
     * Charge the attack for the left side state
     */
    void chargeRightAttack() {
        _rightState.isCharged = true;
    }
    
    /**
     * Calculates the direction of the swipe and sets the state/swipe.
     *
     * @param startPos  the starting position of the swipe
     * @param endPos       the ending posiition of the swipe
     * @param isLeftSidedSwipe  if the swipe was on the left side of the screen
     *
     */
    void calculateSwipeDirection(cugl::Vec2 startPos, cugl::Vec2 endPos, bool isLeftSidedSwipe, bool grounded, bool floored,  cugl::Timestamp startTime);
    
    /**
     * Processes the type of swipe attack that was just completed on the left side
     * and resets the left side state
     */
    void processLeftState();
    
    /**
     * Resets the left side state to no direction and not charged
     */
    void resetLeftState() {
        _leftState.direction = none;
        _leftState.isCharged = false;
        _leftChargingTime = 0;
        _lStart = true;
        _lCoolStart = 0;
    }
    
    /**
     * Processes the type of swipe attack that was just completed on the right side
     * and resets the right side state
     */
    void processRightState(bool grounded, bool floored);
    
    /**
     * Resets the left side state to no direction and not charged
     */
    void resetRightState() {
        _rightState.angle = 0;
        _rightState.isCharged = false;
        _rightChargingTime = 0;
        _rStart = true;
        _rCoolStart = 0;
    }
    
    /**
     * Print the side and direction of the swipe (for testing only)
     *
     * @param s  the swipe we want to print
     * @param isLeftSidedSwipe if the swipe was completed on the left side
     */
    void printSwipe(SwipeAttack s, bool isLeftSidedSwipe);
    
public:
    
    /**
     * Creates a new swipe controller.
     */
    SwipeController();
    
    /**
     * Deletes this swipe controller, releasing all resources.
     */
    ~SwipeController();
    
    /**
     * Updates the swipe controller based on the latest inputs.
     */
    void update(InputController& input, bool grounded, bool floored, float dt);
    
    /**
     * Returns the type of swipe attack that was just completed on the left side
     *
     * @return the left-sided swipe attack
     */
    SwipeAttack getLeftSwipe() {
        return _leftSwipe;
    };
    
    /**
     * Returns the angle of the swipe attack that was just completed on the left side
     *
     * @return the left-sided swipe angle
     */
    float getLeftAngle() {
        return _leftAngle;
    };
    
    /**
     * Returns the type of swipe attack that was just completed on the right side
     *
     * @return the right-sided swipe attack
     */
    SwipeAttack getRightSwipe() {
        return _rightSwipe;
    };
    
    /**
     * Returns the angle of the swipe attack that was just completed on the right side
     *
     * @return the right-sided swipe angle
     */
    float getRightAngle() {
        return _rightAngle;
    };
    
    float getRangeCharge() {
        return std::min(_cRangeCount / _cRCool, 1.0f);
    }
    
    float getMeleeCharge() {
        return std::min(_cMeleeCount / _cMCool, 1.0f);
    }
    
    Uint64 getRightChargingTime() {
        return _rightChargingTime;
    }
    
    Uint64 getLeftChargingTime() {
        return _leftChargingTime;
    }
    
    void coolMelee(int hits);
    
    void coolRange(int hits);
    
    /**
     * Returns whether the left attack is charged
     *
     * @return whether the left attack is charged
     */
    bool hasLeftChargedAttack() {
        return _leftState.isCharged;
    }
    
    /**
     * Returns whether the right attack is charged
     *
     * @return whether the right attack is charged
     */
    bool hasRightChargedAttack() {
        return _rightState.isCharged;
    }
    
    bool isRightAttackCharged() {
        return _rightSwipe == chargedUp ||
        _rightSwipe == chargedRight ||
        _rightSwipe == chargedLeft;
    }
    
    /**
     * Reset swipes states and values
     */
    void reset();
    
};

#endif /* SwipeController_hpp */
