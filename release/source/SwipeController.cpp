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
_rightSwipe(none),
_leftSwipe(none)
{
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
    
    // If left finger lifted, process left swipe
    if(_input.didLeftRelease()) {
        calculateSwipeDirection(_input.getLeftStartPosition(), _input.getLeftEndPosition(), true);
    }
    // Otherwise note that no left swipe was completed this frame
    else {
        setLeftSwipe(none);
    }
    
    // If right finger lifted, process right swipe
    if(_input.didRightRelease()) {
        calculateSwipeDirection(_input.getRightStartPosition(), _input.getRightEndPosition(), false);
    }
    // Otherwise note that no right swipe was completed this frame
    else {
        setRightSwipe(none);
    }

}

/**
 * Calculates the direction of the swipe.
 *
 * @param startPos  the starting position of the swipe
 * @param endPos       the ending posiition of the swipe
 * @param isLeftSidedSwipe  if the swipe was on the left side of the screen
 *
 * @return Swipe, the direction of the swipe
 */
SwipeController::Swipe SwipeController::calculateSwipeDirection(cugl::Vec2 startPos, cugl::Vec2 endPos, bool isLeftSidedSwipe){
    
    // Do we have to check that endPos and startPos on the same side of screen for
    // the swipe to count?
    
    float startx = startPos.x;
    float starty = startPos.y;
    float endx = endPos.x;
    float endy = endPos.y;
    
    float xdiff = endx - startx;
    float ydiff = endy - starty;
    
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
            setLeftSwipe(up);
        } else {
            setRightSwipe(up);
        }
    }
    // Down swipe, y diff is positive
    // Angle from 45 to 90 or -45 to -90, > 45 or < -45
    else if (ydiff > 0 && (swipeAngle < -45 || swipeAngle > 45)) {
        if (isLeftSidedSwipe) {
            setLeftSwipe(down);
        } else {
            setRightSwipe(down);
        }
    }
    // Right swipe, x diff is poisitve
    // Angle from -45 to 45
    else if (xdiff > 0 && swipeAngle > -45 && swipeAngle < 45) {
        if (isLeftSidedSwipe) {
            setLeftSwipe(right);
        } else {
            setRightSwipe(right);
        }
    }
    // Left swipe, x diff is negative
    // Angle from -45 to 45
    else if (xdiff < 0 && swipeAngle > -45 && swipeAngle < 45) {
        if (isLeftSidedSwipe) {
            setLeftSwipe(left);
        } else {
            setRightSwipe(left);
        }
    }
    
    // Printing swipes for testing
    if (isLeftSidedSwipe) {
        printSwipe(getLeftSwipe(),true);
    }
    else {
        printSwipe(getRightSwipe(),false);
    }
    
    return none;
    
}

/**
 * Print the side and direction of the swipe.
 *
 * @param s  the swipe we want to print
 * @param isLeftSidedSwipe if the swipe was completed on the left side
 */
void SwipeController::printSwipe(Swipe s, bool isLeftSidedSwipe) {
    
    if (isLeftSidedSwipe) {
        if (s == up) {
            CULog("Left Sided Swipe: Up");
        }
        if (s == right) {
            CULog("Left Sided Swipe: Right");
        }
        if (s == down) {
            CULog("Left Sided Swipe: Down");
        }
        if (s == left) {
            CULog("Left Sided Swipe: Left");
        }
        if (s == none) {
            CULog("No left sided swipe completed this frame");
        }
    }
    else {
        if (s == up) {
            CULog("Right Sided Swipe: Up");
        }
        if (s == right) {
            CULog("Right Sided Swipe: Right");
        }
        if (s == down) {
            CULog("Right Sided Swipe: Down");
        }
        if (s == left) {
            CULog("Right Sided Swipe: Left");
        }
        if (s == none) {
            CULog("No right sided swipe completed this frame");
        }
    }
    
}
