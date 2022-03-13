//
//  AttackController.hpp
//  LiminalSpirit
//
//  Created by Sashimi Software on 3/2/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef AttackController_h
#define AttackController_h

#include <cugl/cugl.h>
#include <unordered_set>
#include "SwipeController.hpp"

class AttackController {
    
    enum Side {
        left,
        right
    };
    
public:
    
    class Attack {
        
        //The position of the player
        cugl::Vec2 position;
        
        //The Offset from the position of the attack hitbox
        cugl::Vec2 offset;
        
        //The radius of the attack hitbox
        float _radius;
        
        //The age of the hitbox (how long it stays active)
        float age;
        
        //Whether the hitbox is active or not
        bool active;
        
        //Drawing scale for hitbox
        float _scale;
        
        //The damage of the hitbox
        float damage;
        
        //A velocity vector to update the projectile
        
        cugl::Vec2 _vel;
        
        //Which type of swipe this is
        Side side;
        
        cugl::Poly2 ball;
        
        
    public:
        /**
         * Creates an attack circle with the specified parameters. Scale is constant as it is dependent on the drawing scene.
         *
         * @param p         The position of the hitbox
         * @param age     The duration of the hitbox
         * @param dmg     The amount of damage the hitbox does
         * @param scale The drawing scale size of the hitbox
         */
        Attack(const cugl::Vec2 p, float radius, float a, float dmg, float scale, Side s, cugl::Vec2 vel, cugl::Vec2 oof, cugl::PolyFactory b);
        
        
        /**
         * Updates the attack hitbox. Set true to move with player while active.
         *
         * @param p          The position of the player
         * @param follow      Whether to follow the player's movement while active
         */
        void update(const cugl::Vec2 p, bool follow);
        
        bool isActive() {return active;}
        
        float getRadius() {return _radius;}
        
        cugl::Poly2 getBall() {return ball;}
        cugl::Vec2 getPosition() { return position + offset; }
        int getDamage() { return damage; }
        Side getSide(){return side;}
    };
    
    std::unordered_set<std::shared_ptr<Attack>> _pending;
    
    std::unordered_set<std::shared_ptr<Attack>> _current;
    
    float _scale;
    
    cugl::Vec2 leftOff;
    
    cugl::Vec2 rightOff;

    cugl::Vec2 upOff;

    cugl::Vec2 downOff;
    
    cugl::Vec2 _p_vel;
    
    cugl::Vec2 _c_vel;
    
    cugl::PolyFactory ballMakyr = cugl::PolyFactory(0.05f);
    
    
    /**
     *  Creates an empty attack controller. The attack controller simply updates and creates attack hitboxes and removes them from the active queue.
     */
    AttackController();
    
    
    /**
     *  Initializes the attack controller. Currently greyed out because we only have basic attack hitboxes. Can use a json to set predetermined attack shapes, designs, and damage if we have more complicated moves and attacks.
     *  Projectile velocities are vectors facing the +y direction. They are rotated accordingly when initializing different direction attacks.
     */
    void init(float scale, cugl::Vec2 oof, cugl::Vec2 p_vel, cugl::Vec2 c_vel);
    
    /**
     *  Update function for attack controller. Updates all attacks and removes inactive attacks from queue.
     *
     *  @param p    The player position
     */
    void update(const cugl::Vec2 p);
    
    
    /**
     *  Helper method to determine whether there are no active hitboxes.
     */
    bool isEmpty() { return _current.empty() && _pending.empty(); }
    
    /**
     *  Creates an attack for a right sided swipe.
     */
    void attackRight(cugl::Vec2 p, SwipeController::SwipeAttack attack, bool grounded);
    
    /**
     *  Creates an attack for a left sided swipe.
     */
    void attackLeft(cugl::Vec2 p, SwipeController::SwipeAttack attack, bool grounded);
    
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch);
    
};

#endif
