//
//  AttackController.hpp
//  LiminalSpirit
//
//  Created by Sashimi Software on 3/2/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef AttackController_h
#define AttackController_h

#define ATTACK_SENSOR_NAME "attacksensor"



#include <cugl/cugl.h>
#include <unordered_set>
#include "SwipeController.hpp"
#include "PlayerModel.h"

class AttackController {
    
    enum Side {
        left,
        right
    };
    
public:
    
    class Attack : public cugl::physics2::CapsuleObstacle{
        
        //The position of the player
        cugl::Vec2 position;
        
        //The Offset from the position of the attack hitbox
        cugl::Vec2 offset;
        
        //The radius of the attack hitbox
        float radius;
        
        //The age of the hitbox (how long it stays active)
        float age;
        
        //Whether the hitbox is active or not
        bool active;
        
        //Drawing scale for hitbox
        float _scale;
        
        //The damage of the hitbox
        float damage;
        
        //Which type of swipe this is
        Side side;
        
        cugl::Poly2 ball;

        /**Attack sensor */
        b2Fixture* _sensorFixture;
        /** Name of sensor */
        std::string _sensorName;
        
        
    public:
        /**
         * Creates an attack circle with the specified parameters. Scale is constant as it is dependent on the drawing scene.
         *
         * @param p         The position of the hitbox
         * @param age     The duration of the hitbox
         * @param dmg     The amount of damage the hitbox does
         * @param scale The drawing scale size of the hitbox
         */
        Attack() : CapsuleObstacle(), _sensorName(ATTACK_SENSOR_NAME) { }
        bool init(const cugl::Vec2 p, float a, float dmg, float scale, Side s, cugl::Vec2 oof, cugl::PolyFactory b, boolean playerAttack);
        
        
        /**
         * Updates the attack hitbox. Set true to move with player while active.
         *
         * @param p          The position of the player
         * @param follow      Whether to follow the player's movement while active
         */
        void update(const cugl::Vec2 p, b2Vec2 VX, bool follow);
        
        bool isActive() {return active;}
        
        float getRadius() {return radius;}
        
        cugl::Poly2 getBall() {return ball;}
        cugl::Vec2 getPosition() { return position + offset; }
        int getDamage() { return damage; }
        Side getSide(){return side;}

        std::string* getSensorName() { return &_sensorName; }
        std::string* setSensorName(string s) { _sensorName = s; }

#pragma mark - 
#pragma mark Physics Methods
        /**Creates and adds the physics body(s) to the world */
        void createFixtures() override;

        /** Releases the fixtures of this body(s) from the world */
        void releaseFixtures() override;
#pragma mark -
#pragma mark Static Constructors
        static std::shared_ptr<Attack> alloc(const cugl::Vec2 p, float a, float dmg, float scale, Side s, cugl::Vec2 oof, cugl::PolyFactory b, boolean playerAttack) {
            std::shared_ptr<Attack> result = std::make_shared<Attack>();
            return (result->init(p, a, dmg, scale, s, oof, b, playerAttack) ? result : nullptr);
        }
    };
    
    std::unordered_set<std::shared_ptr<Attack>> _pending;
    
    std::unordered_set<std::shared_ptr<Attack>> _current;
    
    float _scale;

    std::shared_ptr<PlayerModel> _player;
    
    cugl::Vec2 leftOff;
    
    cugl::Vec2 rightOff;

    cugl::Vec2 upOff;

    cugl::Vec2 downOff;
    
    cugl::PolyFactory ballMakyr = cugl::PolyFactory(0.25f);
    
    
    /**
     *  Creates an empty attack controller. The attack controller simply updates and creates attack hitboxes and removes them from the active queue.
     */
    AttackController();
    
    
    /**
     *  Initializes the attack controller. Currently greyed out because we only have basic attack hitboxes. Can use a json to set predetermined attack shapes, designs, and damage if we have more complicated moves and attacks.
     */
    void init(float scale, cugl::Vec2 oof, std::shared_ptr<PlayerModel> player);
    
    /**
     *  Update function for attack controller. Updates all attacks and removes inactive attacks from queue.
     *
     *  @param p    The player position
     *  @param VX   The linear velocity of the player
     */
    void update(const cugl::Vec2 p, b2Vec2 VX);
    
    
    /**
     *  Helper method to determine whether there are no active hitboxes.
     */
    bool isEmpty() { return _current.empty() && _pending.empty(); }
    
    /**
     *  Creates an attack for a right sided swipe.
     */
    void attackRight(SwipeController::Swipe direction, bool grounded);
    
    /**
     *  Creates an attack for a left sided swipe.
     */
    void attackLeft(SwipeController::Swipe direction, bool grounded);
    
    void draw(const std::shared_ptr<cugl::SpriteBatch>& batch);
    
};

#endif
