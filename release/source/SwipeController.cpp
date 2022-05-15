//
//  SwipeController.cpp
//  LiminalSpirit
//
//  Created by Sashimi Software
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "SwipeController.hpp"
#include <math.h>

#define RANGE_COOLDOWN 10.0f;
#define MELEE_COOLDOWN 6.0f;

#define RANGE_REDUCTION 0.5f;
#define MELEE_REDUCTON 0.8f;

/**
 * Creates a new swipe controller.
 */
SwipeController::SwipeController() : _leftSwipe(noAttack),
                                     _rightSwipe(noAttack)
{
    _leftState.construct();
    _rightState.construct();
    
    _cMeleeCount = MELEE_COOLDOWN;
    _cRangeCount = RANGE_COOLDOWN;
    
    _cMCool = MELEE_COOLDOWN;
    _cRCool = RANGE_COOLDOWN;
    
    _leftChargingTime = 0;
    _rightChargingTime = 0;
    
    _lStart = true;
    _rStart = true;
    
    _lCoolStart = 0;
    _rCoolStart = 0;
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
void SwipeController::update(InputController &input, bool grounded, float dt)
{
    if (!hasLeftChargedAttack()) {
        _cRangeCount += dt;
    }
    
    if (!hasRightChargedAttack()) {
        _cMeleeCount += dt;
    }
    
    
#ifdef CU_TOUCH_SCREEN

    // If the left finger is pressed down, check if it has been pressed long enough for
    // a charge attack
    if(input.isLeftDown()) {
        calculateChargeAttack(input.getLeftStartTime(), true);
    }
    // If left finger lifted, process left swipe
    else if (input.didLeftRelease())
    {
        _lStart = true;
        _lCoolStart = 0;
        calculateSwipeDirection(input.getLeftStartPosition(), input.getLeftEndPosition(), true, grounded, input.getLeftStartTime());
    }
    // Otherwise note that no left swipe was completed this frame
    else
    {
        setLeftSwipe(noAttack);
        _lStart = true;
        _lCoolStart = 0;
    }
    
    // If the right finger is pressed down, check if it has been pressed long enough for
    // a charge attack
    if(input.isRightDown()) {
        calculateChargeAttack(input.getRightStartTime(), false);
    }
    // If right finger lifted, process right swipe
    else if(input.didRightRelease()) {
        _rStart = true;
        _rCoolStart = 0;
        calculateSwipeDirection(input.getRightStartPosition(), input.getRightEndPosition(), false, grounded, input.getRightStartTime());
    }
    // Otherwise note that no right swipe was completed this frame
    else
    {
        setRightSwipe(noAttack);
        _rStart = true;
        _rCoolStart = 0;
    }
#else
    switch (input._leftCode)
    {
        case 0:
        {
            setLeftSwipe(noAttack);
            break;
        }
        case 1:
        {
            setLeftDirection(up);
            setLeftAngle(90);
            processLeftState();
            break;
        }
        case 2:
        {
            setLeftDirection(left);
            setLeftAngle(180);
            processLeftState();
            break;
        }
        case 3:
        {
            setLeftDirection(down);
            setLeftAngle(270);
            processLeftState();
            break;
        }
        case 4:
        {
            setLeftDirection(right);
            setLeftAngle(0);
            processLeftState();
            break;
        }
        case 5:
        {
            setLeftSwipe(jump);
        }
            
    }
    switch (input._rightCode)
    {
        case 0:
        {
            setRightSwipe(noAttack);
            break;
        }
        case 1:
        {
            setRightAngle(90);
            processRightState(grounded);
            break;
        }
        case 2:
        {
            setRightAngle(180);
            processRightState(grounded);
            break;
        }
        case 3:
        {
            setRightAngle(270);
            processRightState(grounded);
            break;
        }
        case 4:
        {
            setRightAngle(0);
            processRightState(grounded);
            break;
        }
        case 5:
        {
            setRightSwipe(jump);
        }
    }
    if (input._leftCharged) {
        chargeLeftAttack();
    }
    if (input._rightCharged) {
        chargeRightAttack();
    }
#endif
    
}

/**
 * Calculates whether a finger has been pressed down long enough for a charge attack
 * and updates the state accordingly
 */
void SwipeController::calculateChargeAttack(cugl::Timestamp startTime, bool isLeftSidedCharge) {
    
    // Don't increment charge countdown on cooldown, return
//    if (isLeftSidedCharge && _cRCool < _cRangeCount) {
//        return;
//    }
//
//    if (!isLeftSidedCharge && _cMCool >= _cMeleeCount) {
//        return;
//    }
    
    // If the attack is already charged, stop calculating the time diff
    if (isLeftSidedCharge) {
        if (hasLeftChargedAttack()) return;
    } else {
        if (hasRightChargedAttack()) return;
    }
        
    _currTime.mark();
    
    if (_lStart && isLeftSidedCharge) {
        _lStart = false;
        _lCoolStart = clampf(_cRCool - _cRangeCount, 0, _cRCool);
    }
    
    if (_rStart && !isLeftSidedCharge) {
        _rStart = false;
        _rCoolStart = clampf(_cMCool - _cMeleeCount, 0, _cMCool);
    }

    Uint64 chargeTime = cugl::Timestamp::ellapsedMillis(startTime, _currTime);
    if (isLeftSidedCharge) {
        chargeTime = chargeTime - (_lCoolStart * 1000) ;
        _leftChargingTime = chargeTime;
    } else {
        chargeTime = chargeTime - (_rCoolStart * 1000);
        _rightChargingTime = chargeTime;
    }
    
//    CULog("%llu, %f, %llu", chargeTime, _rCoolStart, _rightChargingTime);
    
    // half second charge time
    if (chargeTime >= CHARGE_TIME) {
        if (isLeftSidedCharge) {
            if (_cRCool <= _cRangeCount) {
                chargeLeftAttack();
                _cRangeCount = 0;
                _lCoolStart = 0;
                _lStart = true;
            }
            
        } else {
            if (_cMCool <= _cMeleeCount) {
                chargeRightAttack();
                _cMeleeCount = 0;
                _rCoolStart = 0;
                _rStart = true;
            }
            
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
void SwipeController::calculateSwipeDirection(cugl::Vec2 startPos, cugl::Vec2 endPos, bool isLeftSidedSwipe, bool grounded, cugl::Timestamp startTime)
{

    // x increases from left to right
    // y increases from top to bottom
    float startx = startPos.x;
    float starty = startPos.y;
    float endx = endPos.x;
    float endy = endPos.y;

    float xdiff = endx - startx;
    float ydiff = endy - starty;

    // If the xdiff and ydiff is really small the input was a tap, no need to process swipes
    if (xdiff > -20 && xdiff < 20 && ydiff > -20 && ydiff < 20)
    {
        _currTime.mark();
        Uint64 tapTime = cugl::Timestamp::ellapsedMillis(startTime, _currTime);
        
        // if the tap time is less than half a second the intent of the tap was a jump
        if (tapTime < CHARGE_TIME) {
            if (isLeftSidedSwipe) {
                setLeftSwipe(jump);
            } else {
                setRightSwipe(jump);
            }
        }
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
    
    if (isLeftSidedSwipe) {
        // Set swipe angle to angle divisible by 15 (24 directions)
        swipeAngle = (int)(swipeAngle + 7.5) / 15 * 15;
        // Set swipe angle to angle divisible by 10
        //    swipeAngle = (int)(swipeAngle + 5) / 10 * 10;
    
        // Right swipe
        // Angle from 0 to 45 or 315 to 360
        if (swipeAngle > 315 || swipeAngle <= 45) {
            setLeftDirection(right);
            setLeftAngle(swipeAngle);
        }
        // Up swipe
        // Angle from 45 to 135
        else if (swipeAngle > 45 && swipeAngle <= 135) {
            setLeftDirection(up);
            setLeftAngle(swipeAngle);
        }
        // Left swipe
        // Angle from 135 to 225
        else if (swipeAngle > 135 && swipeAngle <= 225) {
            setLeftDirection(left);
            setLeftAngle(swipeAngle);
        }
        // Down swipe
        // Angle from 225 to 315
        else if (swipeAngle > 225 && swipeAngle <= 315) {
            setLeftDirection(down);
            setLeftAngle(swipeAngle);
        }
        
    }
    else {
        // Right sided swipe
        setRightAngle(swipeAngle);
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
//        printSwipe(getRightSwipe(), false);
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
    float swipeAngle = _rightState.angle;
    
    // Can't charge attack up if not grounded, keep charged state
    if (charged && !grounded) {
        if (swipeAngle > 67.5 && swipeAngle <= 112.5) {
            return;
        }
    }
    // Can't charge attack down if grounded, keep charged state
    if (charged && grounded) {
        if (swipeAngle > 247.5 && swipeAngle <= 292.5) {
            return;
        }
    }
    
    resetRightState();
    
    if(charged) {
        if (swipeAngle > 337.5 || swipeAngle <= 22.5) {
            setRightSwipe(chargedRight);
        }
        else if (swipeAngle > 22.5 && swipeAngle <= 67.5) {
            setRightSwipe(chargedNortheast);
        }
        else if (swipeAngle > 67.5 && swipeAngle <= 112.5) {
            setRightSwipe(chargedUp);
        }
        else if (swipeAngle > 112.5 && swipeAngle <= 157.5) {
            setRightSwipe(chargedNorthwest);
        }
        else if (swipeAngle > 157.5 && swipeAngle <= 202.5) {
            setRightSwipe(chargedLeft);
        }
        else if (swipeAngle > 202.5 && swipeAngle <= 247.5) {
            setRightSwipe(chargedSouthwest);
        }
        else if (swipeAngle > 247.5 && swipeAngle <= 292.5) {
            setRightSwipe(chargedDown);
        }
        else if (swipeAngle > 292.5 && swipeAngle <= 337.5) {
            setRightSwipe(chargedSoutheast);
        }
    }
    else {
        // Right swipe
        // Angle from 0 to 45 or 315 to 360
        if (swipeAngle > 315 || swipeAngle <= 45) {
            setRightSwipe(rightAttack);
        }
        // Up swipe
        // Angle from 45 to 135
        else if (swipeAngle > 45 && swipeAngle <= 135) {
            setRightSwipe(upAttack);
        }
        // Left swipe
        // Angle from 135 to 225
        else if (swipeAngle > 135 && swipeAngle <= 225) {
            setRightSwipe(leftAttack);
        }
        // Down swipe
        // Angle from 225 to 315
        else if (swipeAngle > 225 && swipeAngle <= 315) {
            setRightSwipe(downAttack);
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
        if (s == chargedNortheast) {
            CULog("Right Sided Swipe: Charged Northeast");
        }
        if (s == chargedNorthwest) {
            CULog("Right Sided Swipe: Charged Northwest");
        }
        if (s == chargedSoutheast) {
            CULog("Right Sided Swipe: Charged Southeast");
        }
        if (s == chargedSouthwest) {
            CULog("Right Sided Swipe: Charged Southwest");
        }
        if (s == noAttack) {
            CULog("No right sided swipe completed this frame");
        }
    }
}

void SwipeController::coolMelee(int hits) {
    if (!hasRightChargedAttack()) {
        _cMeleeCount += hits * MELEE_REDUCTON;
    }
}

void SwipeController::coolRange(int hits) {
    if (!hasLeftChargedAttack()) {
        _cRangeCount += hits * RANGE_REDUCTION;
    }
}

void SwipeController::reset()
{
    resetLeftState();
    resetRightState();
    _leftSwipe = noAttack;
    _rightSwipe = noAttack;
    _cMeleeCount = _cMCool;
    _cRangeCount = _cRCool;
    _leftChargingTime = 0;
    _rightChargingTime = 0;
    _lStart = true;
    _lCoolStart = 0;
    _rStart = true;
    _rCoolStart = 0;
}
