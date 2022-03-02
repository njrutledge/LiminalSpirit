//
//  AttackController.hpp
//  LiminalSpirit
//
//  Created by Sashimi Software on 3/2/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef AttackController_hpp
#define AttackController_hpp

#include <cugl/cugl.h>
#include <unordered.set>
#include "SwipeController.hpp"

class AttackController {
    
    class Attack {
        
        //The position of the player
        cugl::Vec2 position;
        
        //The Transform from the position of the attack hitbox
        cugl::Affine2 transform;
        
        //The radius of the attack hitbox
        float radius;
        
        //The age of the hitbox (how long it stays active)
        float age;
        
        //Whether the hitbox is active or not
        bool active;
        
        //Drawing scale for hitbox
        float scale;
        
        //The damage of the hitbox
        float damage;
        
        enum Side {
            left,
            right
        };
        
        /**
         * Creates an attack circle with the specified parameters. Scale is constant as it is dependent on the drawing scene.
         *
         * @param p         The position of the hitbox
         * @param r         The radius / size of the hitbox
         * @param age     The duration of the hitbox
         * @param dmg     The amount of damage the hitbox does
         * @param scale The drawing scale size of the hitbox
         */
        Attack(const cugl::Vec2 p, float r, float age, float dmg, float scale);
        
        
        /**
         * Updates the attack hitbox. Set true to move with player while active.
         *
         * @param move          The vecture to move the hitbox by
         * @param follow      Whether to follow the player's movement while active
         */
        void update(const cugl::Vec2 move, bool follow);
    }
    
    std::unordered_set<std::shared_ptr<Attack>> _pending;
    
    std::unordered_set<std::shared_ptr<Attack>> _current;
    
    float scale;
    
    cugl::Affine2 left;
    
    cugl::Affine2 right;
    
    
    /**
     *  Creates an empty attack controller. The attack controller simply updates and creates attack hitboxes and removes them from the active queue.
     */
    AttackController();
    
    
    /**
     *  Initializes the attack controller. Currently greyed out because we only have basic attack hitboxes. Can use a json to set predetermined attack shapes, designs, and damage if we have more complicated moves and attacks.
     */
    //bool init(std::shared_ptr<cugl::JsonValue> data);
    
    
    /**
     *  Helper method to determine whether there are no active hitboxes.
     */
    bool isEmpty() { return _current.empty() && _pending.empty(); }
    
    /**
     *  Creates an attack for a right sided swipe.
     */
    void attackRight();
    
    
    /**
     *  Creates an attack for a left sided swipe.
     */
    void attackLeft();
    
}
