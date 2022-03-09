//
//  InputController.hpp
//  LiminalSpirit
//
//  Created by Sashimi Software
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef InputController_hpp
#define InputController_hpp

#include <stdio.h>
#include <cugl/cugl.h>

/**
 * Device-independent input manager.
 *
 * This class supports drag controls. This means pressing at a location,
 * dragging while pressed, and then releasing. However, the implementation
 * for this approach varies according to whether it is a mouse or mobile
 * touch controls. This interface hides the details.
 */
class InputController {

protected:
    /** Whether the input device was successfully initialized */
    bool _active;
    /** The current right touch position */
    cugl::Vec2 _currRightPos;
    /** The current left touch position */
    cugl::Vec2 _currLeftPos;

    /** Whether there is an active press on the right side of the screen */
    bool _currRightDown;
    /** Whether there was an active press on the right side last frame */
    bool _prevRightDown;
    /** Whether there is an active press on the left side of the screen */
    bool _currLeftDown;
    /** Whether there was an active press on the left side last frame */
    bool _prevLeftDown;
    /** Middle x coordinate for the screen for calculating if a finger press is on the left or right */
    float _screenMidpoint;

protected:
    // Touchscreen variables
    /** The key for the touch listeners */
    Uint32 _touchKey;
    
    // Right finger variables
    /** Right finger ID */
    cugl::TouchID _rightFingerID;
    /** Right finger position at start of swipe */
    cugl::Vec2 _rightStartPos;
    /** Right finger position at end of swipe */
    cugl::Vec2 _rightEndPos;
    /** Whether the current finger is down */
    bool _rightFingerDown;
    
    // Left finger variables
    /** Left finger ID */
    cugl::TouchID _leftFingerID;
    /** Left finger position at start of swipe */
    cugl::Vec2 _leftStartPos;
    /** Left finger position at end of swipe */
    cugl::Vec2 _leftEndPos;
    /** Whether the left finger is down */
    bool _leftFingerDown;

    

#pragma mark Input Control
public:
    int _leftCode;
    int _rightCode;
    int _moveCode;
    /**
     * Creates a new input controller.
     *
     * This constructor DOES NOT attach any listeners, as we are not
     * ready to do so until the scene is created. You should call
     * the {@link #init} method to initialize the scene.
     */
    InputController();

    /**
     * Deletes this input controller, releasing all resources.
     */
    ~InputController() { dispose(); }
    
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
    bool init(float leftmostX, float screenWidth);

    /** TESTING ONLY: reads keyboard on PC*/
    void readInput();
    
    
    /**
     * Disposes this input controller, deactivating all listeners.
     *
     * As the listeners are deactived, the user will not be able to
     * monitor input until the controller is reinitialized with the
     * {@link #init} method.
     */
    void dispose();
    
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
    void update();

#pragma mark Attributes
    /**
     * Returns true if this control is active.
     *
     * An active control is one where all of the listeners are attached
     * and it is actively monitoring input. An input controller is only
     * active if {@link #init} is called, and if {@link #dispose} is not.
     *
     * @return true if this control is active.
     */
    bool isActive() const { return _active; }
    
    /**
     * Returns the position the left swipe started at
     *
     * @return the left swipe start position
     */
    const cugl::Vec2& getLeftStartPosition() const {
        return _leftStartPos;
    }
    
    /**
     * Returns the position the left swipe ended at
     *
     * @return the left swipe end position
     */
    const cugl::Vec2& getLeftEndPosition() const {
        return _leftEndPos;
    }
    
    /**
     * Returns the position the right swipe started at
     *
     * @return the right swipe start position
     */
    const cugl::Vec2& getRightStartPosition() const {
        return _rightStartPos;
    }

    /**
     * Returns the position the right swipe ended at
     *
     * @return the right swipe end position
     */
    const cugl::Vec2& getRightEndPosition() const {
        return _rightEndPos;
    }
    
    /**
     * Return true if the user initiated a press this frame.
     *
     * A press means that the user is pressing a finger down this
     * animation frame, but was not pressing during the last frame.
     *
     * @return true if the user initiated a press this frame.
     */
    bool didLeftPress() const {
        return !_prevLeftDown && _currLeftDown;
    }

    /**
     * Return true if the user initiated a release this frame on the left side of the screen.
     *
     * A release means that the user was pressing a finger down last
     * animation frame, but is not pressing during this frame.
     *
     * @return true if the user initiated a release this frame.
     */
    bool didLeftRelease() const {
        return !_currLeftDown && _prevLeftDown;
    }

    
    /**
     * Return true if the user initiated a press this frame on the right side of the screen.
     *
     * A press means that the user is pressing a finger down this
     * animation frame, but was not pressing during the last frame.
     *
     * @return true if the user initiated a press this frame.
     */
    bool didRightPress() const {
        return !_prevRightDown && _currRightDown;
    }

    /**
     * Return true if the user initiated a release this frame on the right side of the screen.
     *
     * A release means that the user was pressing a finger down last
     * animation frame, but is not pressing during this frame.
     *
     * @return true if the user initiated a release this frame.
     */
    bool didRightRelease() const {
        return !_currRightDown && _prevRightDown;
    }


#pragma mark Touchscreen Callbacks
    /**
     * Call back to execute when a finger is pressed.
     *
     * This function will record a press only if it is the first finger pressed on the left or right.
     *
     * @param event     The event with the touch information
     * @param focus     Whether this device has focus (UNUSED)
     */
    void fingerDownCB(const cugl::TouchEvent& event, bool focus);

    /**
     * Call back to execute when a finger is first released.
     *
     * This function will record a release for the first right or left finger pressed..
     *
     * @param event     The event with the touch information
     * @param focus     Whether this device has focus (UNUSED)
     */
    void fingerUpCB(const cugl::TouchEvent& event, bool focus);

};

#endif /* InputController_hpp */
