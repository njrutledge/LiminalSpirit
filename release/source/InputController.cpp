//
//  InputController.cpp
//  LiminalSpirit
//
//  Created by Sashimi Software
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "InputController.hpp"

using namespace cugl;

#pragma mark Input Control
/**
 * Creates a new input controller.
 *
 * This constructor DOES NOT attach any listeners, as we are not
 * ready to do so until the scene is created. You should call
 * the {@link #init} method to initialize the scene.
 */
InputController::InputController() :
_active(false),
_currRightDown(false),
_prevRightDown(false),
_currLeftDown(false),
_prevLeftDown(false),
_leftFingerDown(false),
_rightFingerDown(false),
_touchKey(0)
{
}


/**
 * Initializes the control to support touch.
 *
 * This method attaches all of the listeners.
 *
 * This method will fail (return false) if the listeners cannot
 * be registered or if there is a second attempt to initialize
 * this controller
 *
 * @param leftmostX      the leftmost X coordinate that is safe for swipes
 * @param screenWidth  the width of the screen
 *
 * @return true if the initialization was successful
 */
bool InputController::init(float leftmostX, float screenWidth) {
    Touchscreen* tscreen = Input::get<Touchscreen>();
    if (tscreen) {
        _touchKey = tscreen->acquireKey();
        tscreen->addBeginListener(_touchKey, [=](const cugl::TouchEvent& event, bool focus) {
            this->fingerDownCB(event, focus);
        });
        tscreen->addEndListener(_touchKey, [=](const cugl::TouchEvent& event, bool focus) {
            this->fingerUpCB(event, focus);
        });
        _active = true;
        _screenMidpoint = leftmostX + (screenWidth/2);
    }
    return _active;
}

/**
 * Disposes this input controller, deactivating all listeners.
 *
 * As the listeners are deactived, the user will not be able to
 * monitor input until the controller is reinitialized with the
 * {@link #init} method.
 */
void InputController::dispose() {
    if (_active) {
        Touchscreen* tscreen = Input::get<Touchscreen>();
        tscreen->removeBeginListener(_touchKey);
        tscreen->removeEndListener(_touchKey);
        _active = false;
    }
}

/**
 * Updates the input controller for the latest frame.
 *
 * It might seem weird to have this method given that everything
 * is processed with call back functions.  But we need some way
 * to synchronize the input with the animation frame.  Otherwise,
 * how can we know what was the touch location *last frame*?
 * Maybe there has been no callback function executed since the
 * last frame. This method guarantees that everything is properly
 * synchronized.
 */
void InputController::update() {
    _prevRightDown = _currRightDown;
    _currRightDown = _rightFingerDown;
    _prevLeftDown = _currLeftDown;
    _currLeftDown = _leftFingerDown;
}

#pragma mark Touchscreen Callbacks

/**
 * Call back to execute when a finger is pressed.
 *
 * This function will record a press only if it is the first finger pressed.
 *
 * @param event     The event with the touch information
 * @param focus     Whether this device has focus (UNUSED)
 */
void InputController::fingerDownCB(const cugl::TouchEvent& event, bool focus) {
    // Figure out which side of the screen the finger went down on
    // and update accordingly
    if (event.position.x < _screenMidpoint) {
        if(!_leftFingerDown) {
            _leftFingerDown = true;
            _leftFingerID = event.touch;
            _leftStartPos = event.position;
        }
    }
    else {
        if(!_rightFingerDown) {
            _rightFingerDown = true;
            _rightFingerID = event.touch;
            _rightStartPos = event.position;
        }
    }
}

/**
 * Call back to execute when a finger is first released.
 *
 * This function will record a release for the first finger pressed.
 *
 * @param event     The event with the touch information
 * @param focus     Whether this device has focus (UNUSED)
 */
void InputController::fingerUpCB(const cugl::TouchEvent& event, bool focus) {
    // Only recognize the finger being lifted if it is one of the fingers
    // being tracked right now
    if (_leftFingerDown && (event.touch == _leftFingerID)) {
        _leftFingerDown = false;
        _leftEndPos = event.position;
    }
    else if (_rightFingerDown && (event.touch == _rightFingerID)) {
        _rightFingerDown = false;
        _rightEndPos = event.position;
    }
}
