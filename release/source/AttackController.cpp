//
//  AttackController.cpp
//  LiminalSpirit
//
//  Created by Sashimi Software on 3/2/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "AttackController.hpp"
#include "PlayerModel.h"

/** Debug color for sensor */
#define DEBUG_COLOR Color4::RED
using namespace cugl;

// Declare static variables so they can be allocated
float AttackController::_worldHeight;
float AttackController::_worldWidth;

bool AttackController::Attack::init(const cugl::Vec2 p, float radius, float a, float dmg, float scale, Type s, MeleeState m, cugl::Vec2 oof, cugl::PolyFactory b, cugl::Vec2 vel, float angle, float timer, string attackID, int frames) {
    
    _position = (p + oof);
    _radius = radius;
    _age = a;
    _maxAge = a;
    _damage = dmg;
    _type = s;
    _meleeState = m;
    _scale = scale;
    _vel = vel;
    _angle = angle;
    _offset = oof;
    _active = true;
    //_ball = b.makeCircle(_position, _radius);
    _UID = rand();
    _attackID = attackID;
    _timer = 0;
    _maxFrames = frames;
    _sensorFixture = nullptr;
    _homingSensorFixture = nullptr;
    _bodySensorFixture = nullptr;
    _homingEnemy = nullptr;
    if (CapsuleObstacle::init(_position, Size(_radius, _radius))) {
        // TODO change the sensor naming based on if its player attack
        b2Filter filter = b2Filter();
        setBodyType(b2BodyType::b2_dynamicBody);
        switch (_type) {
            case Type::p_range:
            case Type::p_exp_package:
                _homingSensorName = "player" + _sensorName + "homing";
            case Type::p_dash:
            case Type::p_melee:
                _sensorName = "player" + _sensorName;
                filter.categoryBits = 0b010000;
                filter.maskBits = 0b001010;
                setFilterData(filter);
                break;
            case Type::p_exp:
                _sensorName = "player" + _sensorName;
                filter.categoryBits = 0b010000;
                filter.maskBits = 0b001010;
                setFilterData(filter);
                break;
                
            default:
                _sensorName = "enemy"  + _sensorName;
                filter.categoryBits = 0b100000;
                filter.maskBits = 0b000011;
                setFilterData(filter);
                break;
        }

        this->setSensor(true);
        return true;
    }
    return false;
}

void AttackController::Attack::createFixtures() {
    if (_body == nullptr) {
        return;
    }
    CapsuleObstacle::createFixtures();
    b2FixtureDef sensorDef;
    sensorDef.density = 0;
    sensorDef.isSensor = true;
    b2PolygonShape sensorShape;
    
    b2Vec2 corners[8];
    cugl::Vec2 vec(0, _radius);
    for (int i = 0; i < 8; i++) {
        corners[i] = b2Vec2(vec.x, vec.y);
        _debugVerticies.push_back(Vec2(vec));
        vec.rotate(M_PI / 4.0f);
    }

    sensorShape.Set(corners, 8);
    sensorDef.shape = &sensorShape;
    sensorDef.userData.pointer = reinterpret_cast<uintptr_t>(getSensorName());
    _sensorFixture = _body->CreateFixture(&sensorDef);


    if (_type == p_range || _type == p_exp_package) {
        b2FixtureDef sensorDef3;
        sensorDef3.density = 0;
        sensorDef3.isSensor = true;
        b2PolygonShape sensorShape3;
        b2Vec2 corners3[8];
        
        if (_type == p_exp_package) {
            cugl::Vec2 vec(0, _radius * 10.0);//TODO: 10.0 is random
            for (int i = 0; i < 8; i++) {
                corners3[i] = b2Vec2(vec.x, vec.y);
                _debugVerticies3.push_back(Vec2(vec));
                vec.rotate(M_PI / 4.0f);
            }
        } else {
            cugl::Vec2 vec(0, _radius * 5.0);//TODO: 5.0 is random
            for (int i = 0; i < 8; i++) {
                corners3[i] = b2Vec2(vec.x, vec.y);
                _debugVerticies3.push_back(Vec2(vec));
                vec.rotate(M_PI / 4.0f);
            }
        }
        
        
        sensorShape3.Set(corners3, 8);
        sensorDef3.shape = &sensorShape3;
        sensorDef3.userData.pointer = reinterpret_cast<uintptr_t>(getHomingSensorName());
        _homingSensorFixture = _body->CreateFixture(&sensorDef3);
    }

    
    if (_type == p_melee || _type == p_dash) {
        b2FixtureDef sensorDef2;
        sensorDef2.density = 0;
        sensorDef2.isSensor = true;
        b2PolygonShape sensorShape2;
        b2Vec2 corners2[8];
        cugl::Vec2 vec2(0, _radius /2.0f);

        for (int i = 0; i < 8; i++) {
            corners2[i] = b2Vec2(vec2.x, vec2.y) - b2Vec2(_offset.x, _offset.y);
            _debugVerticies2.push_back(Vec2(corners2[i].x, corners2[i].y));
            vec2.rotate(M_PI / 4.0f);
        }
        sensorShape2.Set(corners2, 8);
        sensorDef2.shape = &sensorShape2;
        sensorDef2.userData.pointer = reinterpret_cast<uintptr_t>(getSensorName());
        _bodySensorFixture = _body->CreateFixture(&sensorDef2);
    }
    setFilterData(getFilterData());//fix attacks!
}

void AttackController::Attack::releaseFixtures() {
    //if (_body != nullptr) {
    //    return;
    //}
    CapsuleObstacle::releaseFixtures();
    if (_sensorFixture != nullptr) {
        _body->DestroyFixture(_sensorFixture);
        _sensorFixture = nullptr;
    }

    if (_homingSensorFixture != nullptr) {
        _body->DestroyFixture(_homingSensorFixture);
        _homingSensorFixture = nullptr;
    }


    if (_bodySensorFixture != nullptr) {
        _body->DestroyFixture(_bodySensorFixture);
        _bodySensorFixture = nullptr;

    }
}

void AttackController::Attack::update(const cugl::Vec2 p, bool follow, float dt, b2Vec2 VX) {
    if (_active) {
        if (follow) {
            _position = p + _offset;
            _body->SetLinearVelocity(VX);
        }else if (_body->GetLinearVelocity() == b2Vec2(0,0)) {
            _body->SetLinearVelocity(b2Vec2(_vel.x*_scale, _vel.y*_scale));
        }
        _position = _position + _vel;
        _age -= dt;
        if (_age <= 0) {
            _active =  false;
        }
        //if (_homingEnemy && !_homingEnemy->isRemoved() && _homingEnemy->getHealth() > 0) {
        //    Vec2 enemyPos = _homingEnemy->getPosition();
        //    Vec2 attackPos = getPosition();
        //    Vec2 diffDirection = enemyPos - attackPos;
        //    diffDirection.normalize().scale(10);
        //    Vec2 start = getLinearVelocity();
        //    Vec2 end = start + diffDirection;
        //    setLinearVelocity(end.scale(start.length() / end.length()));
        //}
        setNodeAngle(getLinearVelocity().getAngle());

       //animations
        if (_maxFrames != 0) {
            _timer += dt;
            if (_type == p_exp) {
                if (_timer > .03) {
                    std::shared_ptr<scene2::SpriteNode> spriteNode = dynamic_pointer_cast<scene2::SpriteNode>(_node);
                    spriteNode->setFrame((spriteNode->getFrame() + 1) % _maxFrames);
                    _timer = 0;
                }
            } else {
                if (_timer > .06) {
                    std::shared_ptr<scene2::SpriteNode> spriteNode = dynamic_pointer_cast<scene2::SpriteNode>(_node);
                    spriteNode->setFrame((spriteNode->getFrame() + 1) % _maxFrames);
                    _timer = 0;
                }
            }
        }
    }
}

void AttackController::Attack::dispose() {
    _core = nullptr;
    _node = nullptr;
    _sensorNode = nullptr;
    _sensorFixture = nullptr;
    _bodySensorFixture = nullptr;
    _hitEnemies.clear();
    _homingEnemy = nullptr;

}

AttackController::AttackController() {
    //need to add initialization for left and right offsets
}

void AttackController::init(float scale, float rscale, float oof, cugl::Vec2 p_vel, cugl::Vec2 c_vel, float hit_wind, float hit_cooldown, float reload, float swingSpeed, float worldWidth, float worldHeight) {
    _scale = scale;
    _rscale = rscale;
    _leftOff = cugl::Vec2(-oof, 0.0f);
    _rightOff = cugl::Vec2(oof, 0.0f);
    _upOff = cugl::Vec2(0, oof);
    _downOff = cugl::Vec2(0, -oof);
    _p_vel = p_vel;
    _c_vel = c_vel;
    _hit_window = hit_wind;
    _multi_cooldown = hit_cooldown;
    _meleeCounter = 0;
    _rangedCounter = 0;
    _multiCounter = 0;
    _reload = reload;
    _swing = swingSpeed;
    _melee = first;
    _worldWidth = worldWidth;
    _worldHeight = worldHeight;
}

void AttackController::update(const cugl::Vec2 p, b2Vec2 VX, float dt) {
    auto it = _current.begin();
    while(it != _current.end()) {
        if ((*it)->getType() == Type::p_melee) {
            (*it)->update(p, true, dt, VX);
        } else if ((*it)->getType() == Type::p_dash) {
            (*it)->update(p, true, dt, VX);
        } else {
            (*it)->update(p, false, dt, VX);
        }
        if (!((*it)->isActive())) {
            (*it)->markRemoved(true);
            it++;
            // it = _current.erase(it);
        } else {
            it++;
        }
    }
    
    
    for(auto it = _pending.begin(); it != _pending.end(); ++it) {
            _current.emplace(*it);
    }
        
    _pending.clear();
    
    
    
    _meleeCounter += dt;
    _rangedCounter += dt;
    if (_melee != first) {
        _multiCounter += dt;
        if ((_melee != cool && _multiCounter > _hit_window) || (_multiCounter > _multi_cooldown)) {
            _melee = first;
            _multiCounter = 0;
        }
    }
}

void AttackController::attackLeft(cugl::Vec2 p, SwipeController::SwipeAttack attack, float angle, bool grounded, float timer, std::shared_ptr<SoundController> sound) {
    // Subtract 90 degrees from the angle because vector rotates with the angle starting from the north
    float angleAdjusted = angle - 90;
    if (_rangedCounter > _reload) {
        switch (attack) {
        case SwipeController::leftAttack:
            _pending.emplace(Attack::alloc(p, 0.6, 0.5, 10, _rscale, Type::p_range, first, Vec2(0, 0), ballMakyr, cugl::Vec2(_p_vel).rotate(angleAdjusted * M_PI / 180), angle, left, timer, PLAYER_RANGE, PLAYER_RANGE_FRAMES));
            _rangedCounter = 0;
            sound->play_player_sound(SoundController::playerSType::shoot);
            break;
        case SwipeController::rightAttack:
            _pending.emplace(Attack::alloc(p, 0.6, 0.5, 10, _rscale, Type::p_range, first, Vec2(0, 0), ballMakyr, cugl::Vec2(_p_vel).rotate(angleAdjusted * M_PI / 180), angle, right, timer, PLAYER_RANGE, PLAYER_RANGE_FRAMES));
            _rangedCounter = 0;
            sound->play_player_sound(SoundController::playerSType::shoot);
            break;
        case SwipeController::upAttack:
            _pending.emplace(Attack::alloc(p, 0.6, 0.5, 10, _rscale, Type::p_range, first, Vec2(0, 0), ballMakyr, cugl::Vec2(_p_vel).rotate(angleAdjusted * M_PI / 180), angle, up, timer, PLAYER_RANGE, PLAYER_RANGE_FRAMES));
            _rangedCounter = 0;
            sound->play_player_sound(SoundController::playerSType::shoot);
            break;
        case SwipeController::downAttack:
            _pending.emplace(Attack::alloc(p, 0.6, 0.5, 10, _rscale, Type::p_range, first, Vec2(0, 0), ballMakyr, cugl::Vec2(_p_vel).rotate(angleAdjusted * M_PI / 180), angle, down, timer, PLAYER_RANGE, PLAYER_RANGE_FRAMES));
            _rangedCounter = 0;
            sound->play_player_sound(SoundController::playerSType::shoot);
            break;
        case SwipeController::chargedLeft:
            _pending.emplace(Attack::alloc(p, 0.3, 4, 0, _scale, Type::p_exp_package, first, Vec2(0, 0), ballMakyr, cugl::Vec2(_c_vel).rotate(angleAdjusted * M_PI / 180), angle, left, timer, PLAYER_RANGE, PLAYER_EXP_PKG_FRAMES));
            _rangedCounter = 0;
            sound->play_player_sound(SoundController::playerSType::shootCharge);
            break;
        case SwipeController::chargedRight:
            _pending.emplace(Attack::alloc(p, 0.3, 4, 0, _scale, Type::p_exp_package, first, Vec2(0, 0), ballMakyr, cugl::Vec2(_c_vel).rotate(angleAdjusted * M_PI / 180), angle, right, timer, PLAYER_RANGE, PLAYER_EXP_PKG_FRAMES));
            _rangedCounter = 0;
            sound->play_player_sound(SoundController::playerSType::shootCharge);
            break;
        case SwipeController::chargedUp:
            _pending.emplace(Attack::alloc(p, 0.3, 4, 0, _scale, Type::p_exp_package, first, Vec2(0, 0), ballMakyr, cugl::Vec2(_c_vel).rotate(angleAdjusted * M_PI / 180), angle, up, timer, PLAYER_RANGE, PLAYER_EXP_PKG_FRAMES));
            _rangedCounter = 0;
            sound->play_player_sound(SoundController::playerSType::shootCharge);
            break;
        case SwipeController::chargedDown:
            _pending.emplace(Attack::alloc(p, 0.3, 4, 0, _scale, Type::p_exp_package, first, Vec2(0, 0), ballMakyr, cugl::Vec2(_c_vel).rotate(angleAdjusted * M_PI / 180), angle, down, timer, PLAYER_RANGE, PLAYER_EXP_PKG_FRAMES));
            _rangedCounter = 0;
            sound->play_player_sound(SoundController::playerSType::shootCharge);
            break;
        default:
            break;
        }
    }
}


/**
 * Right size represents melee in this case.
 */
void AttackController::attackRight(cugl::Vec2 p, SwipeController::SwipeAttack attack, float angle, bool grounded, bool facingRight, float timer, std::shared_ptr<SoundController> sound) {
    if (_meleeCounter > _swing) {
        switch (attack) {
            case SwipeController::leftAttack:
                if (_melee == cool) {
                    break;
                } else if (_melee == h2_left && _multiCounter < _hit_window) {
                    _pending.emplace(Attack::alloc(p, 2, 0.1, 12, _scale, Type::p_melee, h2_left, _leftOff, ballMakyr, cugl::Vec2::ZERO, 180, left, timer, PLAYER_MELEE, 0));
                    _meleeCounter = 0;
                    _melee = h3_left;
                    sound->play_player_sound(SoundController::playerSType::slashEmpty);
                } else if (_melee == h3_left && _multiCounter < _hit_window) {
                    _pending.emplace(Attack::alloc(p, 3, 0.1, 20, _scale, Type::p_melee, h3_left, _leftOff, ballMakyr, cugl::Vec2::ZERO, 180, left, timer, PLAYER_MELEE, 0));
                    _meleeCounter = 0;
                    _melee = cool;
                    sound->play_player_sound(SoundController::playerSType::slashEmpty);
                } else {
                    _pending.emplace(Attack::alloc(p, 2, 0.1, 8, _scale, Type::p_melee, h1_left, _leftOff, ballMakyr, cugl::Vec2::ZERO, 180, left, timer, PLAYER_MELEE, 0));
                    _meleeCounter = 0;
                    _melee = h2_left;
                    sound->play_player_sound(SoundController::playerSType::slashEmpty);
                }
                break;
            case SwipeController::rightAttack:
                if (_melee == cool) {
                    break;
                } else if (_melee == h2_right && _multiCounter < _hit_window) {
                    _pending.emplace(Attack::alloc(p, 2, 0.1, 12, _scale, Type::p_melee, h2_right, _rightOff, ballMakyr, cugl::Vec2::ZERO, 0, right, timer, PLAYER_MELEE, 0));
                    _meleeCounter = 0;
                    _melee = h3_right;
                    sound->play_player_sound(SoundController::playerSType::slashEmpty);
                } else if (_melee == h3_right && _multiCounter < _hit_window) {
                    _pending.emplace(Attack::alloc(p, 3, 0.1, 20, _scale, Type::p_melee, h3_right, _rightOff, ballMakyr, cugl::Vec2::ZERO, 0, right, timer, PLAYER_MELEE, 0));
                    _meleeCounter = 0;
                    _melee = cool;
                    sound->play_player_sound(SoundController::playerSType::slashEmpty);
                } else {
                    _pending.emplace(Attack::alloc(p, 2, 0.1, 8, _scale, Type::p_melee, h1_right, _rightOff, ballMakyr, cugl::Vec2::ZERO, 0, right, timer, PLAYER_MELEE, 0));
                    _meleeCounter = 0;
                    _melee = h2_right;
                    sound->play_player_sound(SoundController::playerSType::slashEmpty);
                }
                break;
            case SwipeController::upAttack:
                if(grounded){
                    _pending.emplace(Attack::alloc(p, 1.5, 0.5, 8, _scale, Type::p_melee, jump_attack, _upOff, ballMakyr, cugl::Vec2::ZERO, 0, up, timer, PLAYER_MELEE, 0));
                    break;
                }
            case SwipeController::downAttack:
//                if(!grounded){
//                _pending.emplace(Attack::alloc(p, 1, 0.08, 9001, _scale, Type::p_melee, _downOff, ballMakyr, cugl::Vec2::ZERO, down));
//                } else{
//                    _pending.emplace(Attack::alloc(p, 1, 0.05, 9001, _scale, Type::p_melee, _leftOff, ballMakyr, cugl::Vec2::ZERO, left));
//                    _pending.emplace(Attack::alloc(p, 1, 0.05, 9001, _scale,  Type::p_melee, _rightOff, ballMakyr, cugl::Vec2::ZERO, right));
//                }
                break;
            case SwipeController::chargedLeft:
                _pending.emplace(Attack::alloc(p, 2, DASHTIME, 20, _scale, Type::p_dash, first, _leftOff + Vec2(-0.5,0), ballMakyr, cugl::Vec2(-DASHX - 3, 0), 180, left, timer, PLAYER_MELEE, 0));
                _meleeCounter = 0;
                _melee = cool;
                sound->play_player_sound(SoundController::playerSType::slashDash);
                break;
            case SwipeController::chargedRight:
                _pending.emplace(Attack::alloc(p, 2, DASHTIME, 20, _scale, Type::p_dash, first, _rightOff + Vec2(0.5,0), ballMakyr, cugl::Vec2(DASHX + 3, 0), 0, right, timer, PLAYER_MELEE, 0));
                _meleeCounter = 0;
                _melee = cool;
                sound->play_player_sound(SoundController::playerSType::slashDash);
                break;
            case SwipeController::chargedUp:
            {
                float xOffset = 0.5;
                if (!facingRight) {
                    xOffset *= -1;
                }
                _pending.emplace(Attack::alloc(p, 2, DASHTIME, 20, _scale, Type::p_dash, first, _upOff + Vec2(xOffset,0.5), ballMakyr, cugl::Vec2(0, DASHY + 3), 90, up, timer, PLAYER_MELEE, 0));
                _meleeCounter = 0;
                _melee = cool;
                sound->play_player_sound(SoundController::playerSType::slashDash);
                break;
            }
            case SwipeController::chargedDown:
            {
                if(!grounded){
                    float xOffset = 0.5;
                    if (!facingRight) {
                        xOffset *= -1;
                    }
                    _pending.emplace(Attack::alloc(p, 2, DASHTIME, 20, _scale, Type::p_dash, first, _downOff + Vec2(xOffset,0.5), ballMakyr, cugl::Vec2(0, -DASHY - 3), 270, down, timer, PLAYER_MELEE, 0));
                    _meleeCounter = 0;
                    sound->play_player_sound(SoundController::playerSType::slashDash);
                } else {
//                    _pending.emplace(Attack::alloc(p, 2, 0.05, 2, _scale, Type::p_melee, first, _leftOff, ballMakyr, cugl::Vec2::ZERO, 180, left, timer, PLAYER_MELEE, 0));
//                    _meleeCounter = 0;
//                    _pending.emplace(Attack::alloc(p, 2, 0.05, 2, _scale,  Type::p_melee, first, _rightOff, ballMakyr, cugl::Vec2::ZERO, 0, right, timer, PLAYER_MELEE, 0));
//                    _meleeCounter = 0;
                }
                _melee = cool;
                break;
            }
            case SwipeController::chargedNortheast:
                _pending.emplace(Attack::alloc(p, 2, DASHTIME, 20, _scale, Type::p_dash, first, Vec2(_rightOff.x,_upOff.y), ballMakyr, cugl::Vec2(DASHX, DASHY), 45, northeast, timer, PLAYER_MELEE, 0));
                _meleeCounter = 0;
                _melee = cool;
                sound->play_player_sound(SoundController::playerSType::slashDash);
                break;
            case SwipeController::chargedNorthwest:
                _pending.emplace(Attack::alloc(p, 2, DASHTIME, 20, _scale, Type::p_dash, first, Vec2(_leftOff.x,_upOff.y), ballMakyr, cugl::Vec2(-DASHX, DASHY), 135, northwest, timer, PLAYER_MELEE, 0));
                _meleeCounter = 0;
                _melee = cool;
                sound->play_player_sound(SoundController::playerSType::slashDash);
                break;
            case SwipeController::chargedSouthwest:
                _pending.emplace(Attack::alloc(p, 2, DASHTIME, 20, _scale, Type::p_dash, first, Vec2(_leftOff.x,_downOff.y + 1), ballMakyr, cugl::Vec2(-DASHX, -DASHY), 225, southwest, timer, PLAYER_MELEE, 0));
                _meleeCounter = 0;
                _melee = cool;
                sound->play_player_sound(SoundController::playerSType::slashDash);
                break;
            case SwipeController::chargedSoutheast:
                _pending.emplace(Attack::alloc(p, 2, DASHTIME, 20, _scale, Type::p_dash, first, Vec2(_rightOff.x,_downOff.y + 1), ballMakyr, cugl::Vec2(DASHX, -DASHY), 315, southeast, timer, PLAYER_MELEE, 0));
                _meleeCounter = 0;
                _melee = cool;
                sound->play_player_sound(SoundController::playerSType::slashDash);
                break;
            default:
                break;
        }
    }
}

        
void AttackController::createAttack(cugl::Vec2 p, float radius, float age, float damage, Type t, cugl::Vec2 vel, float timer, string attackID, int frames) {
    float angle = vel.getAngle();
    float angle2 = Vec2(0, 1).getAngle();
    std::shared_ptr<Attack> attack = Attack::alloc(p, radius, age, damage, _scale, t, first, cugl::Vec2::ZERO, ballMakyr, vel, vel.getAngle(), neither, timer, attackID, frames);
    attack->setSplitable(true);
    _pending.emplace(attack);
}

void AttackController::createAttack(cugl::Vec2 p, float radius, float age, float damage, Type t, cugl::Vec2 vel, float timer, string attackID, int frames, bool splitable) {
    std::shared_ptr<Attack> attack = Attack::alloc(p, radius, age, damage, _scale, t, first, cugl::Vec2::ZERO, ballMakyr, vel, vel.getAngle(), neither, timer, attackID, frames);
    attack->setSplitable(splitable);
    _pending.emplace(attack);
}

void AttackController::Attack::resetDebug() {
    CapsuleObstacle::resetDebug();
    //Poly2 poly = _ball;
    std::vector<Uint32> debugIndicies1{ 0,1,2,   2,3,4,   4,5,6,   6,7,0 };
    Poly2 poly(_debugVerticies, debugIndicies1);

    _sensorNode = scene2::WireNode::allocWithTraversal(poly, poly2::Traversal::INTERIOR);
    _sensorNode->setColor(Color4::RED);
    _sensorNode->setPosition(Vec2(_debug->getContentSize().width/2.0f, _debug->getContentSize().height / 2.0f));
    _debug->addChild(_sensorNode);

    if (_type == p_range || _type == p_exp_package) {
        //std::vector<Uint32> debugIndicies2{ 8,9,10,   10,11,12,   12,13,14,   14,15,8 };
        Poly2 poly2(_debugVerticies3, debugIndicies1);
        _homingSensorNode = scene2::WireNode::allocWithTraversal(poly2, poly2::Traversal::INTERIOR);
        _homingSensorNode->setColor(Color4::PAPYRUS);
        _homingSensorNode->setPosition(Vec2(_debug->getContentSize().width / 2.0f - _offset.x, _debug->getContentSize().height / 2.0f - _offset.y));
        _debug->addChild(_homingSensorNode);
    }

    if (_type == p_melee || _type == p_dash) {
        //std::vector<Uint32> debugIndicies2{ 8,9,10,   10,11,12,   12,13,14,   14,15,8 };
        Poly2 poly2(_debugVerticies2, debugIndicies1);
        _bodySensorNode = scene2::WireNode::allocWithTraversal(poly2, poly2::Traversal::INTERIOR);
        _bodySensorNode->setColor(Color4::ORANGE);
        _bodySensorNode->setPosition(Vec2(_debug->getContentSize().width / 2.0f - _offset.x, _debug->getContentSize().height / 2.0f - _offset.y));
        _debug->addChild(_bodySensorNode);
    }
}

void AttackController::reset() {
    _pending.clear();
    _current.clear();
}

bool AttackController::Attack::isSame(Attack* a) {
    if (this && _UID == a->_UID) {
        if (_type == a->_type && _attackID == a->_attackID && _timer == a->_timer && _age == a->_age) {
            if (_radius == a->_radius && _damage == a->_damage) {
                return true;
            }
        }
    }
    return false;
}
