//
//  SwipeController.cpp
//  LiminalSpirit
//
//  Created by Sashimi Software
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "SwipeController.hpp"
#include <math.h>

/**
 * Creates a new swipe controller.
 *
 * Should call the {@link #init} method to initialize the controller.
 */
SwipeController::SwipeController() :
_leftSwipe(noAttack),
_rightSwipe(noAttack)
{
    _leftState.construct();
}

/**
 * Initializes the swipe controller by initializing the input controller
 *
 * @param leftmostX      the leftmost X coordinate that is safe for swipes
 * @param screenWidth  the width of the screen
 *
 * @return true if the initialization was successful
 */
bool SwipeController::init(float leftMostX, float width) {
    return _input.init(leftMostX, width);
}

/**
 * Deletes this swipe controller, releasing all resources.
 */
void SwipeController::dispose() {
    //Nothing to dispose, input controller has a destructor
}

/**
 * Updates the swipe controller for the latest frame.
 *
 * Updates the input controller and the current swipes.
 */
void SwipeController::update() {
    _input.update();
    
    // If the left finger is pressed down, check if it has been pressed long enough for
    // a charge attack
    if(_input.isLeftDown()) {
        calculateChargeAttack();
    }
    // If left finger lifted, process left swipe
    else if(_input.didLeftRelease()) {
        calculateSwipeDirection(_input.getLeftStartPosition(), _input.getLeftEndPosition(), true);
    }
    // Otherwise note that no left swipe was completed this frame
    else {
        setLeftSwipe(noAttack);
    }
    
    // If right finger lifted, process right swipe
    if(_input.didRightRelease()) {
        calculateSwipeDirection(_input.getRightStartPosition(), _input.getRightEndPosition(), false);
    }
    // Otherwise note that no right swipe was completed this frame
    else {
        setRightSwipe(noAttack);
    }

}

/**
 * Calculates whether a finger has been pressed down long enough for a charge attack
 * and updates the state accordingly
 */
void SwipeController::calculateChargeAttack() {
    
    // If the attack is already charged, stop calculating the time diff
    if (hasChargedAttack()) return;
    
    _currTime.mark();

    Uint64 chargeTime = cugl::Timestamp::ellapsedMillis(_input.getLeftStartTime(), _currTime);
    
    // This is currently 500 for easier testing, change it back to 1000 when done testing
    if (chargeTime >= 500) { //1000) {
        chargeAttack();
        CULog("charged projectile");
    }
    
}

/**
 * Calculates the direction of the swipe and sets the state/swipe.
 *
 * @param startPos  the starting position of the swipe
 * @param endPos       the ending posiition of the swipe
 * @param isLeftSidedSwipe  if the swipe was on the left side of the screen
 *
 */
void SwipeController::calculateSwipeDirection(cugl::Vec2 startPos, cugl::Vec2 endPos, bool isLeftSidedSwipe){
    
    // Do we have to check that endPos and startPos on the same side of screen for
    // the swipe to count?
    
    float startx = startPos.x;
    float starty = startPos.y;
    float endx = endPos.x;
    float endy = endPos.y;
    
    float xdiff = endx - startx;
    float ydiff = endy - starty;
    
    // If the xdiff and ydiff is really small, the "swipe" was to charge
    // the attack or was a mistap, no direction calculation needed
    if (xdiff > -20 && xdiff < 20 && ydiff > -20 && ydiff < 20) {
        return;
    }
    
    // If the xdiff is 0, set it to 0.01 to avoid division by 0 error
    if (xdiff == 0) {
        xdiff = 0.01;
    }
    
    // Calculate the angle the swipe makes with the horizontal in degrees
    // Angles should never be less than -90 or greater than 90
    float swipeAngle = atan(ydiff/xdiff) * 180 / M_PI;
    
    // x increases from left to right
    // y increases from top to bottom
    
    // Up swipe, y diff is negative (b/c 0 at top of screen)
    // Angle from 45 to 90 or -45 to -90, angle should never be > 90
    // So > 45 or < -45 is sufficient
    if (ydiff < 0 && (swipeAngle < -45 || swipeAngle > 45)) {
        if (isLeftSidedSwipe) {
            setLeftDirection(up);
        } else {
            setRightSwipe(upAttack);
        }
    }
    // Down swipe, y diff is positive
    // Angle from 45 to 90 or -45 to -90, > 45 or < -45
    else if (ydiff > 0 && (swipeAngle < -45 || swipeAngle > 45)) {
        if (isLeftSidedSwipe) {
            setLeftDirection(down);
        } else {
            setRightSwipe(downAttack);
        }
    }
    // Right swipe, x diff is poisitve
    // Angle from -45 to 45
    else if (xdiff > 0 && swipeAngle > -45 && swipeAngle < 45) {
        if (isLeftSidedSwipe) {
            setLeftDirection(right);
        } else {
            setRightSwipe(rightAttack);
        }
    }
    // Left swipe, x diff is negative
    // Angle from -45 to 45
    else if (xdiff < 0 && swipeAngle > -45 && swipeAngle < 45) {
        if (isLeftSidedSwipe) {
            setLeftDirection(left);
        } else {
            setRightSwipe(leftAttack);
        }
    }
    
    // Process the left swipe state now that a swipe direction was calculated
    // and print swipes for testing
    if (isLeftSidedSwipe) {
        processLeftState();
        printSwipe(getLeftSwipe(),true);
    }
    else {
        printSwipe(getRightSwipe(),false);
    }
    
}

/**
 * Processes the type of swipe attack that was just completed on the left side
 * and resets the left side state
 */
void SwipeController::processLeftState(){
    
    bool charged = hasChargedAttack();
    SwipeDirection dir = _leftState.direction;
    resetLeftState();
    
    if(charged) {
        switch (dir) {
            case up:
                setLeftSwipe(chargedUp);
                break;
            case right:
                setLeftSwipe(chargedRight);
                break;
            case left:
                setLeftSwipe(chargedLeft);
                break;
            case down:
                setLeftSwipe(chargedDown);
                break;
            default:
                setLeftSwipe(noAttack);
                break;
        }
    } else {
        switch (dir) {
            case up:
                setLeftSwipe(upAttack);
                break;
            case right:
                setLeftSwipe(rightAttack);
                break;
            case left:
                setLeftSwipe(leftAttack);
                break;
            case down:
                setLeftSwipe(downAttack);
                break;
            default:
                setLeftSwipe(noAttack);
                break;
        }
    }

};

/**
 * Print the side and direction of the swipe (for testing only)
 *
 * @param s  the swipe we want to print
 * @param isLeftSidedSwipe if the swipe was completed on the left side
 */
void SwipeController::printSwipe(SwipeAttack s, bool isLeftSidedSwipe) {
    
    if (isLeftSidedSwipe) {
        if (s == upAttack) {
            CULog("Left Sided Swipe: Up");
        }
        if (s == rightAttack) {
            CULog("Left Sided Swipe: Right");
        }
        if (s == downAttack) {
            CULog("Left Sided Swipe: Down");
        }
        if (s == leftAttack) {
            CULog("Left Sided Swipe: Left");
        }
        if (s == chargedUp) {
            CULog("Left Sided Swipe: Charged Up");
        }
        if (s == chargedRight) {
            CULog("Left Sided Swipe: Charged Right");
        }
        if (s == chargedDown) {
            CULog("Left Sided Swipe: Charged Down");
        }
        if (s == chargedLeft) {
            CULog("Left Sided Swipe: Charged Left");
        }
        if (s == noAttack) {
            CULog("No left sided swipe completed this frame");
        }
    }
    else {
        if (s == upAttack) {
            CULog("Right Sided Swipe: Up");
        }
        if (s == rightAttack) {
            CULog("Right Sided Swipe: Right");
        }
        if (s == downAttack) {
            CULog("Right Sided Swipe: Down");
        }
        if (s == leftAttack) {
            CULog("Right Sided Swipe: Left");
        }
        if (s == noAttack) {
            CULog("No right sided swipe completed this frame");
        }
    }
    
}
