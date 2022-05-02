//
//  TiltController.cpp
//  LiminalSpirit
//
//  Created by Sashimi Software
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "TiltController.hpp"
using namespace cugl;

#pragma mark -
#pragma mark Accelerom Factors

/** Historical choice from Marmalade */
#define KEYBOARD_FORCE_INCREMENT    5.0f
/** Adjustment factor for accelerometer input (found experimentally) */
#define ACCELEROM_X_FACTOR          35.0f

#pragma mark -
#pragma mark Tilt Input

/**
 * Creates a new tilt controller.
 */
TiltController::TiltController() :
_xpos(0)
{
}

/**
 * Deletes this tilt controller, releasing all resources.
 */
TiltController::~TiltController() {
    //Nothing to release
}

/**
 * Processes the currently cached inputs.
 *
 * This method is used to to poll the current input state.  This will poll the
 * keyboard or accelerometer.
 */
void TiltController::update(InputController& input, float width) {
// TODO: Leaving the keyboard input stuff here for now, will fix when merging with Nick's keyboard stuff
#ifdef CU_TOUCH_SCREEN
    // MOBILE CONTROLS
    Vec3 acc = input.getAcceleration();
    float xAcc = acc.x;
    
    // Only process accelerometer if tilt is more than 0.1 so
    // player does not move when phone is straight
    // Cap the acceleration at 0.5 for both directions
    if(!(xAcc > -0.1 && xAcc < 0.1)) {
        if (xAcc > 0.5) {
            xAcc = 0.5;
        } else if (xAcc < -0.5) {
            xAcc = -0.5;
        }
        _xpos = xAcc * ACCELEROM_X_FACTOR;
    } else {
        _xpos = 0;
    }
#else
    // Only process keyboard on desktop
    if (input._moveCode == -1) {
        _xpos = -KEYBOARD_FORCE_INCREMENT;
    } else if (input._moveCode == 1) {
        _xpos = KEYBOARD_FORCE_INCREMENT;
    } else {
        _xpos = 0;
    }
#endif
}

/**
 * Resets tilt controller.
 */
void TiltController::reset() {
    _xpos = 0;
}

float TiltController::getXFactor() {
    return ACCELEROM_X_FACTOR;
}

void TiltController::winTime() {
    _xpos = 0.4 * ACCELEROM_X_FACTOR;
}
