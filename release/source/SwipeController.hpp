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
    
    /** State of the left side swipes */
    LeftSwipeState _leftState;
    
    /** Swipe attack completed on the right side */
    SwipeAttack _rightSwipe;
    /** Swipe attack completed on the left side */
    SwipeAttack _leftSwipe;
    
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
    void calculateChargeAttack(cugl::Timestamp leftStartTime);
    
    /**
     * Charge the attack for the left side state
     */
    void chargeAttack() {
        _leftState.isCharged = true;
    }
    
    /**
     * Calculates the direction of the swipe and sets the state/swipe.
     *
     * @param startPos  the starting position of the swipe
     * @param endPos       the ending posiition of the swipe
     * @param isLeftSidedSwipe  if the swipe was on the left side of the screen
     *
     */
    void calculateSwipeDirection(cugl::Vec2 startPos, cugl::Vec2 endPos, bool isLeftSidedSwipe);
    
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
    void update(InputController& input);
    
    /**
     * Returns the type of swipe attack that was just completed on the right side
     *
     * @return the right-sided swipe attack
     */
    SwipeAttack getRightSwipe() {
        return _rightSwipe;
    };
    
    /**
     * Returns the type of swipe attack that was just completed on the left side
     *
     * @return the left-sided swipe attack
     */
    SwipeAttack getLeftSwipe() {
        return _leftSwipe;
    };
    
    /**
     * Returns whether the left attack is charged
     *
     * @return whether the left attack is charged
     */
    bool hasChargedAttack() {
        return _leftState.isCharged;
    }
    
};

#endif /* SwipeController_hpp */
