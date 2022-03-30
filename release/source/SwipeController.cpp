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
 */
SwipeController::SwipeController() : _leftSwipe(noAttack),
                                     _rightSwipe(noAttack)
{
    _leftState.construct();
    _rightState.construct();
}

/**
 * Deletes this swipe controller, releasing all resources.
 */
SwipeController::~SwipeController()
{
    // Nothing to release
}

/**
 * Updates the swipe controller based on the latest inputs.
 */
void SwipeController::update(InputController &input, bool grounded)
{
#ifdef CU_TOUCH_SCREEN

    // If the left finger is pressed down, check if it has been pressed long enough for
    // a charge attack
    if(input.isLeftDown()) {
        calculateChargeAttack(input.getLeftStartTime(), true);
    }
    // If left finger lifted, process left swipe
    else if (input.didLeftRelease())
    {
        calculateSwipeDirection(input.getLeftStartPosition(), input.getLeftEndPosition(), true, grounded);
    }
    // Otherwise note that no left swipe was completed this frame
    else
    {
        setLeftSwipe(noAttack);
    }
    
    // If the right finger is pressed down, check if it has been pressed long enough for
    // a charge attack
    if(input.isRightDown()) {
        calculateChargeAttack(input.getRightStartTime(), false);
    }
    // If right finger lifted, process right swipe
    else if(input.didRightRelease()) {
        calculateSwipeDirection(input.getRightStartPosition(), input.getRightEndPosition(), false, grounded);
    }
    // Otherwise note that no right swipe was completed this frame
    else
    {
        setRightSwipe(noAttack);
    }
#else
    switch (input._leftCode)
    {
    case 0:
        setLeftSwipe(noAttack);
        break;
    case 1:
        setLeftSwipe(upAttack);
        break;
    case 2:
        setLeftSwipe(leftAttack);
        break;
    case 3:
        setLeftSwipe(downAttack);
        break;
    case 4:
        setLeftSwipe(rightAttack);
        break;
    }
    switch (input._rightCode)
    {
    case 0:
        setRightSwipe(noAttack);
        break;
    case 1:
        setRightSwipe(upAttack);
        break;
    case 2:
        setRightSwipe(leftAttack);
        break;
    case 3:
        setRightSwipe(downAttack);
        break;
    case 4:
        setRightSwipe(rightAttack);
        break;
    }
#endif
}

/**
 * Calculates whether a finger has been pressed down long enough for a charge attack
 * and updates the state accordingly
 */
void SwipeController::calculateChargeAttack(cugl::Timestamp startTime, bool isLeftSidedCharge) {
    
    // If the attack is already charged, stop calculating the time diff
    if (isLeftSidedCharge) {
        if (hasLeftChargedAttack()) return;
    } else {
        if (hasRightChargedAttack()) return;
    }
        
    _currTime.mark();

    Uint64 chargeTime = cugl::Timestamp::ellapsedMillis(startTime, _currTime);
    
    // This is currently 500 for easier testing, change it back to 1000 when done testing
    if (chargeTime >= 500) { //1000) {
        if (isLeftSidedCharge) {
            chargeLeftAttack();
        } else {
            chargeRightAttack();
        }
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
void SwipeController::calculateSwipeDirection(cugl::Vec2 startPos, cugl::Vec2 endPos, bool isLeftSidedSwipe, bool grounded)
{

    float startx = startPos.x;
    float starty = startPos.y;
    float endx = endPos.x;
    float endy = endPos.y;

    float xdiff = endx - startx;
    float ydiff = endy - starty;

    // If the xdiff and ydiff is really small, the "swipe" was to charge
    // the attack or was a mistap, no direction calculation needed
    if (xdiff > -20 && xdiff < 20 && ydiff > -20 && ydiff < 20)
    {
        return;
    }

    // If the xdiff is 0, set it to 0.01 to avoid division by 0 error
    if (xdiff == 0)
    {
        xdiff = 0.01;
    }

    // Calculate the angle the swipe makes with the horizontal in degrees
    // Angles should never be less than -90 or greater than 90
    float angle = atan(ydiff/xdiff) * 180 / M_PI;

    // Calculate swipeAngle in degrees out of 360
    float swipeAngle;
    // First quadrant
    if (ydiff < 0 && angle < 0) {
        swipeAngle = angle * -1;
    }
    // Second quadrant
    else if (ydiff < 0 && angle > 0) {
        swipeAngle = 180 - angle;;
    }
    // Third quadrant
    else if (ydiff > 0 && angle < 0) {
        swipeAngle = 180 + (angle * -1);
    }
    // Fourth quadrant
    else {
        swipeAngle = 360 - angle;
    }
    
    // Convert the swipe angle to one of 16 directions
    if (swipeAngle <= 11.25 || swipeAngle > 348.75) {
        swipeAngle = 0;
    } else if (swipeAngle > 11.25 && swipeAngle <= 33.75) {
        swipeAngle = 22.5;
    } else if (swipeAngle > 33.75 && swipeAngle <= 56.25) {
        swipeAngle = 45;
    } else if (swipeAngle > 56.25 && swipeAngle <= 78.75) {
        swipeAngle = 67.5;
    } else if (swipeAngle > 78.75 && swipeAngle <= 101.25) {
        swipeAngle = 90;
    } else if (swipeAngle > 101.25 && swipeAngle <= 123.75) {
        swipeAngle = 112.5;
    } else if (swipeAngle > 123.75 && swipeAngle <= 146.25) {
        swipeAngle = 135;
    } else if (swipeAngle > 146.25 && swipeAngle <= 168.75) {
        swipeAngle = 157.5;
    } else if (swipeAngle > 168.75 && swipeAngle <= 191.25) {
        swipeAngle = 180;
    } else if (swipeAngle > 191.25 && swipeAngle <= 213.75) {
        swipeAngle = 202.5;
    } else if (swipeAngle > 213.75 && swipeAngle <= 236.25) {
        swipeAngle = 225;
    } else if (swipeAngle > 236.25 && swipeAngle <= 258.75) {
        swipeAngle = 247.5;
    } else if (swipeAngle > 258.75 && swipeAngle <= 281.25) {
        swipeAngle = 270;
    } else if (swipeAngle > 281.25 && swipeAngle <= 303.75) {
        swipeAngle = 292.5;
    } else if (swipeAngle > 303.75 && swipeAngle <= 326.25) {
        swipeAngle = 315;
    } else if (swipeAngle > 326.25 && swipeAngle <= 348.75) {
        swipeAngle = 337.5;
    }

    // x increases from left to right
    // y increases from top to bottom
    
    // Right swipe
    // Angle from 0 to 45 or 315 to 360
    if (swipeAngle > 315 || swipeAngle <= 45) {
        if (isLeftSidedSwipe) {
            setLeftDirection(right);
            setLeftAngle(swipeAngle);
        } else {
            setRightDirection(right);
            setRightAngle(swipeAngle);
        }
    }
    // Up swipe
    // Angle from 45 to 135
    else if (swipeAngle > 45 && swipeAngle <= 135) {
        if (isLeftSidedSwipe) {
            setLeftDirection(up);
            setLeftAngle(swipeAngle);
        } else {
            setRightDirection(up);
            setRightAngle(swipeAngle);
        }
    }
    // Left swipe
    // Angle from 135 to 225
    else if (swipeAngle > 135 && swipeAngle <= 225) {
        if (isLeftSidedSwipe) {
            setLeftDirection(left);
            setLeftAngle(swipeAngle);
        } else {
            setRightDirection(left);
            setRightAngle(swipeAngle);
        }
    }
    // Down swipe
    // Angle from 225 to 315
    else if (swipeAngle > 225 && swipeAngle <= 315) {
        if (isLeftSidedSwipe) {
            setLeftDirection(down);
            setLeftAngle(swipeAngle);
        } else {
            setRightDirection(down);
            setRightAngle(swipeAngle);
        }
    }
    

    
    
    // Process the left swipe state now that a swipe direction was calculated
    // and print swipes for testing
    if (isLeftSidedSwipe)
    {
        processLeftState();
//        printSwipe(getLeftSwipe(), true);
    }
    else {
        processRightState(grounded);
//        printSwipe(getRightSwipe(),false);
    }
}

/**
 * Processes the type of swipe attack that was just completed on the left side
 * and resets the left side state
 */
void SwipeController::processLeftState(){
    
    bool charged = hasLeftChargedAttack();
    SwipeDirection dir = _leftState.direction;
    resetLeftState();

    if (charged)
    {
        switch (dir)
        {
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
    }
    else
    {
        switch (dir)
        {
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
 * Processes the type of swipe attack that was just completed on the left side
 * and resets the left side state
 */
void SwipeController::processRightState(bool grounded){
    
    bool charged = hasRightChargedAttack();
    SwipeDirection dir = _rightState.direction;
    if (!grounded && charged && dir == up) {
        return;
    }
    resetRightState();
    
    if(charged) {
        switch (dir) {
            case up:
                setRightSwipe(chargedUp);
                break;
            case right:
                setRightSwipe(chargedRight);
                break;
            case left:
                setRightSwipe(chargedLeft);
                break;
            case down:
                setRightSwipe(chargedDown);
                break;
            default:
                setRightSwipe(noAttack);
                break;
        }
    } else {
        switch (dir) {
            case up:
                setRightSwipe(upAttack);
                break;
            case right:
                setRightSwipe(rightAttack);
                break;
            case left:
                setRightSwipe(leftAttack);
                break;
            case down:
                setRightSwipe(downAttack);
                break;
            default:
                setRightSwipe(noAttack);
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
void SwipeController::printSwipe(SwipeAttack s, bool isLeftSidedSwipe)
{

    if (isLeftSidedSwipe)
    {
        if (s == upAttack)
        {
            CULog("Left Sided Swipe: Up");
        }
        if (s == rightAttack)
        {
            CULog("Left Sided Swipe: Right");
        }
        if (s == downAttack)
        {
            CULog("Left Sided Swipe: Down");
        }
        if (s == leftAttack)
        {
            CULog("Left Sided Swipe: Left");
        }
        if (s == chargedUp)
        {
            CULog("Left Sided Swipe: Charged Up");
        }
        if (s == chargedRight)
        {
            CULog("Left Sided Swipe: Charged Right");
        }
        if (s == chargedDown)
        {
            CULog("Left Sided Swipe: Charged Down");
        }
        if (s == chargedLeft)
        {
            CULog("Left Sided Swipe: Charged Left");
        }
        if (s == noAttack)
        {
            CULog("No left sided swipe completed this frame");
        }
    }
    else
    {
        if (s == upAttack)
        {
            CULog("Right Sided Swipe: Up");
        }
        if (s == rightAttack)
        {
            CULog("Right Sided Swipe: Right");
        }
        if (s == downAttack)
        {
            CULog("Right Sided Swipe: Down");
        }
        if (s == leftAttack)
        {
            CULog("Right Sided Swipe: Left");
        }
        if (s == chargedUp) {
            CULog("Right Sided Swipe: Charged Up");
        }
        if (s == chargedRight) {
            CULog("Right Sided Swipe: Charged Right");
        }
        if (s == chargedDown) {
            CULog("Right Sided Swipe: Charged Down");
        }
        if (s == chargedLeft) {
            CULog("Right Sided Swipe: Charged Left");
        }
        if (s == noAttack) {
            CULog("No right sided swipe completed this frame");
        }
    }
}

void SwipeController::reset()
{
    resetLeftState();
    resetRightState();
    _leftSwipe = noAttack;
    _rightSwipe = noAttack;
}
