//
//  TiltController.hpp
//  LiminalSpirit
//
//  Created by Sashimi Software
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef TiltController_hpp
#define TiltController_hpp

#include <cugl/cugl.h>
#include "InputController.hpp"

class TiltController {
    
    /** x position */
    float _xpos;
    
    /** store the latest landscape orientation of the phone*/
    cugl::Display::Orientation _lastLandscape;
    
    void updateLandscapeOrientation();
    
public:

#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new tilt controller.
     */
    TiltController();
    
    /**
     * Deletes this tilt controller, releasing all resources.
     */
    ~TiltController();
    
#pragma mark -
#pragma mark Functions
    
    /**
     * Processes the currently cached inputs.
     *
     * This method is used to to poll the current input state.  This will poll the
     * keyboard or accelerometer.
     */
    void update(InputController& input, float width);
    
    /**
     * Resets tilt controller.
     */
    void reset();
    /** 
     * Returns the xFactor multiplier
     */
    float getXFactor();

    /**
     * Returns the current xpos
     */
    const float getXpos() { return _xpos; }

    void winTime();
    
};

#endif
