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
#define KEYBOARD_FORCE_INCREMENT    50.0f
/** Adjustment factor for accelerometer input (found experimentally) */
#define ACCELEROM_X_FACTOR          30.0f

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
void TiltController::update(InputController& input, float width, float characterWidth) {
// TODO: Leaving the keyboard input stuff here for now, will fix when merging with Nick's keyboard stuff
#ifdef CU_TOUCH_SCREEN
    // MOBILE CONTROLS
    Vec3 acc = input.getAcceleration();
    
    // Only process accelerometer if tilt is more than 0.1 so
    // player does not move when phone is straight
    if(!(acc.x > -0.1 && acc.x < 0.1)) {
        _xpos = acc.x * ACCELEROM_X_FACTOR;
    } else {
        _xpos = 0;
    }
#else
    // Only process keyboard on desktop
    Keyboard* keys = Input::get<Keyboard>();

    // Forces increase the longer you hold a key.
    if (keys->keyDown(KeyCode::ARROW_LEFT)) {
        if (_xpos-KEYBOARD_FORCE_INCREMENT >= characterWidth/2) {
            _xpos -= KEYBOARD_FORCE_INCREMENT;
        }
        cout << "left pressed" << endl;
    } else if (keys->keyDown(KeyCode::ARROW_RIGHT)) {
        if (_xpos+KEYBOARD_FORCE_INCREMENT <= width-characterWidth/2) {
            _xpos += KEYBOARD_FORCE_INCREMENT;
        }
        
        cout << "right pressed" << endl;
    }
#endif
}

/**
 * Clears any buffered inputs so that we may start fresh.
 */
void TiltController::clear() {
    _xpos = 0;
}
