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
    void update(InputController& input, float width, float characterWidth);
    
    /**
     * Clears any buffered inputs so that we may start fresh.
     */
    void clear();

    /**
     * Returns the current xpos
     */
    const float getXpos() { return _xpos; }
    
};

#endif
