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
#define PATTACK_TEXTURE "pattack"
#define ATTACK_SSHRINK 1.0f
/**Height of the sensor */
#define SENSOR_HEIGHT 01.f



#include <cugl/cugl.h>
#include <unordered_set>
#include "SwipeController.hpp"
#include "PlayerModel.h"
#include "SoundController.hpp"
#include "BaseEnemyModel.h"

class AttackController {
    
public:

    enum Type {
        p_melee,
        p_dash,
        p_range,
        p_exp_package,
        p_exp,
        e_melee,
        e_range
    };
    
    enum Side {
        right,
        left,
        up,
        down,
        neither
    };

    enum MeleeState {
        first,
        h1_right,
        h2_right,
        h3_right,
        h1_left,
        h2_left,
        h3_left,
        cool
    };
    


protected:
    // Used to make attacks in bounds
    static float _worldWidth;
    static float _worldHeight;
    
public:
    class Attack : public cugl::physics2::CapsuleObstacle{
        
    protected:
        //The position of the player
        cugl::Vec2 _position;
        
        //The Offset from the position of the attack hitbox
        cugl::Vec2 _offset;
        
        //The radius of the attack hitbox
        float _radius;
        
        //The age of the hitbox (how long it stays active)
        float _age;

        //The max age of the hitbox 
        float _maxAge;
        
        //Whether the hitbox is active or not
        bool _active;
        
        //Drawing scale for hitbox
        float _scale;
        
        //The damage of the hitbox
        float _damage;

        //ID of this attack
        float _UID;

        //A velocity vector to update the projectile
        cugl::Vec2 _vel;
        
        /**Attack angle in degrees*/
        float _angle;
        
        //Which type of attack this is
        Type _type;
        
        MeleeState _meleeState;
        
        cugl::Poly2 _ball;

        /**Attack sensor */
        b2Fixture* _sensorFixture;
        /**Attack sensor for inside player body (melee only) */
        b2Fixture* _bodySensorFixture;
        /** Name of sensor */
        std::string _sensorName;
        /** Debug Sensor */
        std::shared_ptr<cugl::scene2::WireNode> _sensorNode;

        std::shared_ptr<cugl::scene2::WireNode> _bodySensorNode;

        std::vector<cugl::Vec2> _debugVerticies;

        std::vector<cugl::Vec2> _debugVerticies2;

        //can the attack split?
        bool _splitable;
        
        virtual void resetDebug() override;
        
        string _attackID;

        float _timer;

        int _maxFrames;

        vector<BaseEnemyModel*> _hitEnemies;
        
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

        virtual ~Attack(void) { dispose(); }

        void dispose();

        bool init(const cugl::Vec2 p, float radius, float a, float dmg, float scale, Type s, MeleeState m,
                  cugl::Vec2 oof, cugl::PolyFactory b, cugl::Vec2 vel, float angle, float timer, string attackID, int frames);
        
        
        /**
         * Updates the attack hitbox. Set true to move with player while active.
         *
         * @param p          The position of the player
         * @param follow      Whether to follow the player's movement while active
         */
        void update(const cugl::Vec2 p, bool follow, float dt, b2Vec2 VX);
        
        void setInactive() {_active = false;}
        
        bool isActive() {return _active;}
        
        float getRadius() {return _radius;}

        float getMaxAge() { return _maxAge;}

        string getAttackID() { return _attackID; }

        int getFrames() { return _maxFrames; }
        
        cugl::Poly2 getBall() {return _ball;}
        cugl::Vec2 getPosition() { return cugl::Vec2(_body->GetPosition().x, _body->GetPosition().y); }
        float getAngle() {return _angle; }
        int getDamage() { return _damage; }
        Type getType(){ return _type; }
        MeleeState getMeleeState(){ return _meleeState; }

        std::string* getSensorName() { return &_sensorName; }
        void setSensorName(string s) { _sensorName = s; }

        bool isSplitable() { return _splitable; }

        void setSplitable(bool val) { _splitable = val; }

        bool isSame(Attack* a);

        cugl::Vec2 getVel() { return _vel; }
        float getScale() { return _scale; }

        bool hasHitEnemy(BaseEnemyModel* enemy) {
            for (auto it = _hitEnemies.begin(); it != _hitEnemies.end(); ++it) {
                if ((* it) == enemy) {
                    return true;
                }
            }
            return false;
        }

        void hitEnemy(BaseEnemyModel* enemy) {
            _hitEnemies.push_back(enemy);
        }

#pragma mark - 
#pragma mark Physics Methods
        /**Creates and adds the physics body(s) to the world */
        void createFixtures() override;

        /** Releases the fixtures of this body(s) from the world */
        void releaseFixtures() override;


 public:

#pragma mark -
#pragma mark Static Constructors
        static std::shared_ptr<Attack> alloc(cugl::Vec2 p, float radius, float age, float dmg, float scale,
                                             Type t, MeleeState m, cugl::Vec2 oof, cugl::PolyFactory b,
                                             cugl::Vec2 vel, float angle, Side s, float timer, string attackID, int frames) {
            //TODO make this not hardcoded
            float off = 2.5f;
            if (p.x + radius + off > _worldWidth && s == right) {
                p.x = _worldWidth - 0.1f - radius - off;
            }
            else if (p.x - radius - off < 0.0f && s == left) {
                p.x = 0.1f + radius + off;
            }

            if (p.y + radius > _worldHeight && s == up) {
                p.y = _worldHeight - 0.1f - radius;
            } else if (p.y - radius < 0.0f && s == down) {
                p.y = 0.1f + radius;
            }

            std::shared_ptr<Attack> result = std::make_shared<Attack>();
            return (result->init(p, radius, age, dmg, scale, t, m, oof, b, vel, angle, timer, attackID, frames) ? result : nullptr);
        }
        
    };
    
    std::unordered_set<std::shared_ptr<Attack>> _pending;
    
    std::unordered_set<std::shared_ptr<Attack>> _current;
    
    float _scale;
    
    /** Scale for range attacks */
    float _rscale;

    std::shared_ptr<PlayerModel> _player;
    
    cugl::Vec2 _leftOff;
    
    cugl::Vec2 _rightOff;

    cugl::Vec2 _upOff;

    cugl::Vec2 _downOff;
    
    cugl::Vec2 _p_vel;
    
    cugl::Vec2 _c_vel;
    
    cugl::PolyFactory ballMakyr = cugl::PolyFactory(0.05f);
    
    float _meleeCounter;
    
    float _multiCounter;
    
    float _hit_window;
    
    float _multi_cooldown;
    
    float _rangedCounter;
    
    float _reload;
    
    float _swing;
    
    MeleeState _melee;
    
    
    /**
     *  Creates an empty attack controller. The attack controller simply updates and creates attack hitboxes and removes them from the active queue.
     */
    AttackController();
    
    
    /**
     *  Initializes the attack controller. Currently greyed out because we only have basic attack hitboxes. Can use a json to set predetermined attack shapes, designs, and damage if we have more complicated moves and attacks.
     *  Projectile velocities are vectors facing the +y direction. They are rotated accordingly when initializing different direction attacks.
     */
    void init(float scale, float rscale, float oof, cugl::Vec2 p_vel, cugl::Vec2 c_vel, float hit_wind, float hit_cooldown, float reload, float swingSpeed, float worldWidth, float worldHeight);
    
    /**
     *  Update function for attack controller. Updates all attacks and removes inactive attacks from queue.
     *
     *  @param p    The player position
     *  @param VX   The linear velocity of the player
     *  @param dt   The timestep
     */
    void update(const cugl::Vec2 p, b2Vec2 VX, float dt) ;
    
    
    /**
     *  Helper method to determine whether there are no active hitboxes.
     */
    bool isEmpty() { return _current.empty() && _pending.empty(); }
    
    /**
     *  Creates an attack for a right sided swipe.
     */
    void attackRight(cugl::Vec2 p, SwipeController::SwipeAttack attack, float angle, bool grounded, float timer, std::shared_ptr<SoundController> sound);
    
    /**
     *  Creates an attack for a left sided swipe.
     */
    void attackLeft(cugl::Vec2 p, SwipeController::SwipeAttack attack, float angle, bool grounded, float timer, std::shared_ptr<SoundController> sound);
    
    /**
     *  Creates an attack with the designated parameters. This is mostly to create enemy attacks, but also any explosion attacks for the player. There is no parameter. This must be calculated in the position. The attack will be splitable
     */
    void createAttack(cugl::Vec2 p, float radius, float age, float damage, Type s, cugl::Vec2 vel, float timer, string attackID, int frames);

    /**
     *  Creates an attack with the designated parameters. This is mostly to create enemy attacks, but also any explosion attacks for the player. There is no parameter. This must be calculated in the position.
     */
    void createAttack(cugl::Vec2 p, float radius, float age, float damage, Type s, cugl::Vec2 vel, float timer, string attackID, int frames, bool splitable);

    /** Get left offset */
    cugl::Vec2 getLeftOff() { return _leftOff; }

    /** Get right offset */
    cugl::Vec2 getRightOff() { return _rightOff; }

    /** Get right offset */
    cugl::Vec2 getUpOff() { return _upOff; }

    /** Get right offset */
    cugl::Vec2 getDownOff() { return _downOff; }
    
    void reset();
    
};

#endif
