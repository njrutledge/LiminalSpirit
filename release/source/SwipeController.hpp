//
//  SwipeController.hpp
//  LiminalSpirit
//
//  Created by Sashimi Software
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef SwipeController_hpp
#define SwipeController_hpp

#include <cugl/cugl.h>
#include <vector>
#include "InputController.hpp"

/**
 * This class figures out what type of swipe was drawn based on the input
 */
class SwipeController {
public:
    /** Enum for swipe directions or none if no swipe was done */
    enum Swipe {
        up,
        right,
        down,
        left,
        none
    };
    
protected:
    /** Swipe completed on the right side */
    Swipe _rightSwipe;
    /** Swipe completed on the left side */
    Swipe _leftSwipe;
    
    /** Input controller */
    InputController _input;
    
    /**
     * Set the left sided swipe
     *
     * @param s, the swipe completed on the left side
     */
    void setLeftSwipe(Swipe s){
        _leftSwipe = s;
    };
    /**
     * Set the right sided swipe
     *
     * @param s  the swipe completed on the right side
     */
    void setRightSwipe(Swipe s){
        _rightSwipe = s;
    };
    
    /**
     * Print the side and direction of the swipe.
     *
     * @param s  the swipe we want to print
     * @param isLeftSidedSwipe if the swipe was completed on the left side
     */
    void printSwipe(Swipe s, bool isLeftSidedSwipe);

    
public:
    
    /**
     * Creates a new swipe controller.
     *
     * Should call the {@link #init} method to initialize the controller.
     */
    SwipeController();
    
    /**
     * Deletes this swipe controller, releasing all resources.
     */
    ~SwipeController() { dispose(); }
    
    /**
     * Initializes the swipe controller by initializing the input controller
     *
     * @param leftmostX      the leftmost X coordinate that is safe for swipes
     * @param screenWidth  the width of the screen
     *
     * @return true if the initialization was successful
     */
    bool init(float leftMostX, float width);
    
    /**
     * Deletes this swipe controller, releasing all resources.
     */
    void dispose();
    
    /**
     * Updates the swipe controller for the latest frame.
     */
    void update();
    
    /**
     * Calculates the direction of the swipe.
     *
     * @param startPos  the starting position of the swipe
     * @param endPos       the ending posiition of the swipe
     * @param isLeftSidedSwipe  if the swipe was on the left side of the screen
     *
     * @return Swipe, the direction of the swipe
     */
    Swipe calculateSwipeDirection(cugl::Vec2 startPos, cugl::Vec2 endPos, bool isLeftSidedSwipe);

    /**
     * Returns the type of swipe that was just completed on the right side
     *
     * @return the right-sided swipe
     */
    Swipe getRightSwipe(){
        return _rightSwipe;
    };
    /**
     * Returns the type of swipe that was just completed on the left side
     *
     * @return the left-sided swipe
     */
    Swipe getLeftSwipe(){
        return _leftSwipe;
    };
    

    
};

#endif /* SwipeController_hpp */
