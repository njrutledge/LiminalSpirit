//
//  MovementInput.hpp
//  LiminalSpirit
//
//  Created by Chenyu Wei on 3/2/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef MovementInput_hpp
#define MovementInput_hpp

#include <cugl/cugl.h>
class MovementInput {
    /** Whether or not this input is active */
    bool _active;
    
    /** x velocity */
    float _xvelocity;
    
    /** x position*/
    float _xpos;

protected:
    // Input results

    /** The thrust produced by the player input */
    cugl::Vec2 _inputThrust;
    
public:

#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new movement input controller.
     *
     * This constructor does NOT do any initialzation.  It simply allocates the
     * object. This makes it safe to use this class without a pointer.
     */
    MovementInput(); // Don't initialize.  Allow stack based
    
    /**
     * Disposes of this input controller, releasing all listeners.
     */
    ~MovementInput() { dispose(); }
    
    /**
     * Deactivates this movement input controller.
     *
     * This method will not dispose of the input controller. It can be reused
     * once it is reinitialized.
     */
    void dispose();
    
    /**
     * Initializes the input control for the given drawing scale.
     *
     * This method works like a proper constructor, initializing the input
     * controller and allocating memory.
     *
     * @return true if the controller was initialized successfully
     */
    bool init();
    
    
#pragma mark -
#pragma mark Input Detection
    /**
     * Returns true if the input handler is currently active
     *
     * @return true if the input handler is currently active
     */
    bool isActive( ) const { return _active; }
    
    /**
     * Processes the currently cached inputs.
     *
     * This method is used to to poll the current input state.  This will poll the
     * keyboard or accelerometer.
     */
    void update(float dt, float xpos, float width, float characterWidth);
    
    /**
     * Clears any buffered inputs so that we may start fresh.
     */
    void clear();

    
#pragma mark -
#pragma mark Input Results
    /**
     * Returns the current input thrust.
     *
     * The thrust is determined by the last input method.
     *
     * @return The input thrust
     */
    const cugl::Vec2& getThrust() { return _inputThrust; }
    const float getXpos() { return _xpos; }
    
};

#endif
