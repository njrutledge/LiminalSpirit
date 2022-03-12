//
//  MovementInput.cpp
//  LiminalSpirit
//
//  Created by Chenyu Wei on 3/2/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "MovementInput.hpp"
using namespace cugl;

#pragma mark -
#pragma mark Input Factors

/** Historical choice from Marmalade */
#define KEYBOARD_FORCE_INCREMENT    5.0f
/** Adjustment factor for accelerometer input (found experimentally) */
#define ACCELEROM_X_FACTOR          30.0f

#pragma mark -
#pragma mark Tilt Input

/**
 * Creates a new movement input controller.
 *
 * This constructor does NOT do any initialzation.  It simply allocates the
 * object. This makes it safe to use this class without a pointer.
 */
MovementInput::MovementInput() :
_active(false),
_xpos(0)
{
}

/**
 * Deactivates this movement input controller.
 *
 * This method will not dispose of the input controller. It can be reused
 * once it is reinitialized.
 */
void MovementInput::dispose() {
    if (_active) {
#ifndef CU_TOUCH_SCREEN
        Input::deactivate<Keyboard>();
#else
        Input::deactivate<Accelerometer>();
#endif
        _active = false;
    }
}

/**
 * Initializes the input control for the given drawing scale.
 *
 * This method works like a proper constructor, initializing the input
 * controller and allocating memory.
 *
 * @return true if the controller was initialized successfully
 */
bool MovementInput::init() {
// Only process keyboard on desktop
#ifndef CU_TOUCH_SCREEN
    _active = Input::activate<Keyboard>();
    _xpos = 0;
#else
    _active = Input::activate<Accelerometer>();
#endif
    return _active;
}

/**
 * Processes the currently cached inputs.
 *
 * This method is used to to poll the current input state.  This will poll the
 * keyboard or accelerometer.
 */
void MovementInput::update(float dt, float xpos, float width, float characterWidth) {
// Only process keyboard on desktop
#ifndef CU_TOUCH_SCREEN
    Keyboard* keys = Input::get<Keyboard>();

    // Forces increase the longer you hold a key.
    if (keys->keyDown(KeyCode::ARROW_LEFT)) {
        if (true|| _xpos - KEYBOARD_FORCE_INCREMENT >= characterWidth / 2) {
            _xpos = -KEYBOARD_FORCE_INCREMENT;
        }
    } else if (keys->keyDown(KeyCode::ARROW_RIGHT)) {
        if (true|| _xpos+KEYBOARD_FORCE_INCREMENT <= width-characterWidth/2) {
            _xpos = KEYBOARD_FORCE_INCREMENT;
        }
        
    }
#else
    // MOBILE CONTROLS
    Vec3 acc = Input::get<Accelerometer>()->getAcceleration();
    
    // Only process accelerometer if tilt is more than 0.1 so
    // player does not move when phone is straight
    if(!(acc.x > -0.1 && acc.x < 0.1)) {
        _xpos = acc.x * ACCELEROM_X_FACTOR;
    } else {
        _xpos = 0;
    }
#endif

}

/**
 * Clears any buffered inputs so that we may start fresh.
 */
void MovementInput::clear() {
    _inputThrust = Vec2::ZERO;
    _xpos = 0;
}
