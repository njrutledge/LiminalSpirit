//
//  MovementInput.cpp
//  LiminalSpirit
//
//  Created by Chenyu Wei on 3/2/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "MovementInput.hpp"
using namespace cugl;

#define RANGE_CLAMP(x,y,z)  (x < y ? y : (x > z ? z : x))

#pragma mark -
#pragma mark Input Factors

/** Historical choice from Marmalade */
#define INPUT_MAXIMUM_FORCE         1000.0f
#define KEYBOARD_FORCE_INCREMENT    50.0f
/** Adjustment factor for touch input */
#define X_ADJUST_FACTOR             50.0f
/** Adjustment factor for accelerometer input (found experimentally) */
#define ACCELEROM_X_FACTOR          5.0f
/** Whether to active the accelerometer (this is TRICKY!) */
#define USE_ACCELEROMETER           true
/** How the time necessary to process a double tap (in milliseconds) */
#define EVENT_DOUBLE_CLICK          400
/** The key for the event handlers */
#define LISTENER_KEY                1

#pragma mark -
#pragma mark Ship Input
/**
 * Creates a new input controller.
 *
 * This constructor does NOT do any initialzation.  It simply allocates the
 * object. This makes it safe to use this class without a pointer.
 */
MovementInput::MovementInput() :
_active(false),
_posx(100.0f)

{
}

/**
 * Deactivates this input controller, releasing all listeners.
 *
 * This method will not dispose of the input controller. It can be reused
 * once it is reinitialized.
 */
void MovementInput::dispose() {
    if (_active) {
#ifndef CU_TOUCH_SCREEN
        Input::deactivate<Keyboard>();
#else
        if (USE_ACCELEROMETER) {
            Input::deactivate<Accelerometer>();
        }
        Touchscreen* touch = Input::get<Touchscreen>();
        touch->removeBeginListener(LISTENER_KEY);
        touch->removeEndListener(LISTENER_KEY);
        _active = false;
#endif
    }
}

/**
 * Initializes the input control for the given drawing scale.
 *
 * This method works like a proper constructor, initializing the input
 * controller and allocating memory.  However, it still does not activate
 * the listeners.  You must call start() do that.
 *
 * @return true if the controller was initialized successfully
 */
bool MovementInput::init() {
    _timestamp.mark();
    bool success = true;
    
// Only process keyboard on desktop
#ifndef CU_TOUCH_SCREEN
    success = Input::activate<Keyboard>();
#else
    if (USE_ACCELEROMETER) {
        success = Input::activate<Accelerometer>();
    }
    Touchscreen* touch = Input::get<Touchscreen>();

//    touch->addBeginListener(LISTENER_KEY,[=](const cugl::TouchEvent& event, bool focus) {
//        this->touchBeganCB(event,focus);
//    });
//    touch->addEndListener(LISTENER_KEY,[=](const cugl::TouchEvent& event, bool focus) {
//        this->touchEndedCB(event,focus);
//    });
#endif
    
    _active = success;
    return success;
}

/**
 * Processes the currently cached inputs.
 *
 * This method is used to to poll the current input state.  This will poll the
 * keyboad and accelerometer.
 *
 * This method also gathers the delta difference in the touches. Depending on
 * the OS, we may see multiple updates of the same touch in a single animation
 * frame, so we need to accumulate all of the data together.
 */
void MovementInput::update(float dt, float width, float characterWidth) {
// Only process keyboard on desktop
#ifndef CU_TOUCH_SCREEN
    Keyboard* keys = Input::get<Keyboard>();

    // Forces increase the longer you hold a key.
    if (keys->keyDown(KeyCode::ARROW_LEFT)) {
        if (_posx-KEYBOARD_FORCE_INCREMENT >= characterWidth/2) {
            _posx -= KEYBOARD_FORCE_INCREMENT;
        }
        cout << "left pressed" << endl;
    } else if (keys->keyDown(KeyCode::ARROW_RIGHT)) {
        if (_posx+KEYBOARD_FORCE_INCREMENT <= width-characterWidth/2) {
            _posx += KEYBOARD_FORCE_INCREMENT;
        }
        
        cout << "right pressed" << endl;
    }

    
//    // Clamp everything so it does not fly off to infinity.
//    _forceLeft  = (_forceLeft  > INPUT_MAXIMUM_FORCE ? INPUT_MAXIMUM_FORCE : _forceLeft);
//    _forceRight = (_forceRight > INPUT_MAXIMUM_FORCE ? INPUT_MAXIMUM_FORCE : _forceRight);
//
//    // Update the keyboard thrust.  Result is cumulative.
//    _keybdThrust.x += _forceRight;
//    _keybdThrust.x -= _forceLeft;
//    //_keybdThrust.x = RANGE_CLAMP(_keybdThrust.x, -INPUT_MAXIMUM_FORCE, INPUT_MAXIMUM_FORCE);
//
//    // Transfer to main thrust. This keeps us from "adding" to accelerometer or touch.
//    _inputThrust.x = _keybdThrust.x/X_ADJUST_FACTOR;
#else
    // MOBILE CONTROLS
    if (USE_ACCELEROMETER) {
        Vec3 acc = Input::get<Accelerometer>()->getAcceleration();
        if(_posx+acc.x*ACCELEROM_X_FACTOR>=characterWidth/2 || _posx+acc.x*ACCELEROM_X_FACTOR <= width-characterWidth/2) {
            _posx +=  acc.x*ACCELEROM_X_FACTOR;
        }
    }
#endif

}

/**
 * Clears any buffered inputs so that we may start fresh.
 */
void MovementInput::clear() {

    _inputThrust = Vec2::ZERO;
//    _keybdThrust = Vec2::ZERO;
//
//    _forceLeft  = 0.0f;
//    _forceRight = 0.0f;
    _posx = 100.0f;

    
    _dtouch = Vec2::ZERO;
    _timestamp.mark();
}

#pragma mark -
#pragma mark Touch Callbacks
/**
 * Callback for the beginning of a touch event
 *
 * @param t     The touch information
 * @param event The associated event
 */
 void MovementInput::touchBeganCB(const cugl::TouchEvent& event, bool focus) {
     // Update the touch location for later gestures
     _dtouch.set(event.position);
}
 
 /**
  * Callback for the end of a touch event
  *
  * @param t     The touch information
  * @param event The associated event
  */
 void MovementInput::touchEndedCB(const cugl::TouchEvent& event, bool focus) {
     
     _timestamp = event.timestamp;
     
//     // Move the ship in this direction
//     Vec2 finishTouch = event.position-_dtouch;
//     //finishTouch.x = RANGE_CLAMP(finishTouch.x, -INPUT_MAXIMUM_FORCE, INPUT_MAXIMUM_FORCE);
//
//     // Go ahead and apply to thrust now.
//     _inputThrust.x = finishTouch.x/X_ADJUST_FACTOR;
     _posx = event.position.x-_dtouch.x;
 }
