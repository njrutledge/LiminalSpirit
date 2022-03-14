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

bool AttackController::Attack::init(const cugl::Vec2 p, float radius, float a, float dmg, float scale, cugl::Size size, Type s, cugl::Vec2 oof, cugl::PolyFactory b, cugl::Vec2 vel) {
    
    _position = (p + oof);
    _radius = radius;
    _age = a;
    _damage = dmg;
    _type = s;
    _scale = scale;
    _vel = vel;
    cugl::Size nsize = size;
    _offset = oof;
    _active = true;
    _ball = b.makeCircle(_position, _radius);
    if (CapsuleObstacle::init(_position, nsize)) {
        // TODO change the sensor naming based on if its player attack
        if (_type == Type::p_left || _type == Type::p_right){
            _sensorName = "player" + _sensorName;
        }else {
            _sensorName = "enemy"  + _sensorName;
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
    for(int i = 0; i < 8; i++){
        corners[i] = b2Vec2(vec.x, vec.y);
        vec.rotate(45);
    }
//    std::vector<cugl::Vec2> cuglVerts = ball.getVertices();
//    int n = cuglVerts.size();
//    std::vector<b2Vec2>* verts = new vector<b2Vec2>(0);
//    //Following is a temporary copy fix, hopefully will find a better solution.
//    for (auto it = cuglVerts.begin(); it != cuglVerts.end();  ++it) {
//        verts->push_back(b2Vec2((*it).x, (*it).y));
//    }
    sensorShape.Set(corners, 8);
    sensorDef.shape = &sensorShape;
    sensorDef.userData.pointer = reinterpret_cast<uintptr_t>(getSensorName());
    _sensorFixture = _body->CreateFixture(&sensorDef);
}

void AttackController::Attack::releaseFixtures() {
    if (_body != nullptr) {
        return;
    }
    CapsuleObstacle::releaseFixtures();
    if (_sensorFixture != nullptr) {
        _body->DestroyFixture(_sensorFixture);
        _sensorFixture = nullptr;
    }
}

void AttackController::Attack::update(const cugl::Vec2 p, bool follow, float dt, b2Vec2 VX) {
    if (_active) {
        if (follow) {
            _position = p + _offset;
            _body->SetLinearVelocity(VX);
        }else{
            _body->SetLinearVelocity(b2Vec2(_vel.x*_scale, _vel.y*_scale));
        }
        _position = _position + _vel;
        _age -= 1;
        if (_age == 0) {
            _active =  false;
        }
    }
}

AttackController::AttackController() {
    //need to add initialization for left and right offsets
}

void AttackController::init(cugl::Size size, float scale, float oof, cugl::Vec2 p_vel, cugl::Vec2 c_vel, float hit_wind, float hit_cooldown, float reload, float swingSpeed) {
    _nsize = size;
    _scale = scale;
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
}

void AttackController::update(const cugl::Vec2 p, b2Vec2 VX, float dt) {
    auto it = _current.begin();
    while(it != _current.end()) {
        if ((*it)->getType() == Type::p_left) {
            (*it)->update(p, false, dt, VX);
        } else {
            (*it)->update(p, true, dt, VX);
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

void AttackController::attackLeft(cugl::Vec2 p, SwipeController::SwipeAttack attack, bool grounded) {
    if (_rangedCounter > _reload) {
        switch (attack) {
            case SwipeController::leftAttack:
                _pending.emplace(Attack::alloc(p, 1, 30, 9001, _scale, Type::p_left, cugl::Vec2(_p_vel).rotate(M_PI * 0.5), _leftOff, ballMakyr));
                _rangedCounter = 0;
                break;
            case SwipeController::rightAttack:
                _pending.emplace(Attack::alloc(p, 1, 30, 9001, _scale, Type::p_left, cugl::Vec2(_p_vel).rotate(M_PI * 1.5), _rightOff, ballMakyr));
                _rangedCounter = 0;
                break;
            case SwipeController::upAttack:
                _pending.emplace(Attack::alloc(p, 1, 30, 9001, _scale, Type::p_left, _p_vel, _upOff, ballMakyr));
                _rangedCounter = 0;
                break;
            case SwipeController::downAttack:
                if(!grounded){
                _pending.emplace(Attack::alloc(p, 1, 30, 9001, _scale, Type::p_left, cugl::Vec2(_p_vel).rotate(M_PI), _downOff, ballMakyr));
                } else{
                    _pending.emplace(Attack::alloc(p, 1, 4, 9001, _scale, Type::p_left, cugl::Vec2(_p_vel).rotate(M_PI * 0.5), _leftOff, ballMakyr));
                    _pending.emplace(Attack::alloc(p, 1, 4, 9001, _scale, Type::p_left, cugl::Vec2(_p_vel).rotate(M_PI * 1.5), _rightOff, ballMakyr));
                }
                _rangedCounter = 0;
                break;
            case SwipeController::chargedLeft:
                _pending.emplace(Attack::alloc(p, 5, 30, 9001, _scale, Type::p_left, cugl::Vec2(_c_vel).rotate(M_PI * 0.5), _leftOff, ballMakyr));
                _rangedCounter = 0;
                break;
            case SwipeController::chargedRight:
                _pending.emplace(Attack::alloc(p, 5, 30, 9001, _scale, Type::p_left, cugl::Vec2(_c_vel).rotate(M_PI * 1.5), _rightOff, ballMakyr));
                _rangedCounter = 0;
                break;
            case SwipeController::chargedUp:
                _pending.emplace(Attack::alloc(p, 5, 30, 9001, _scale, Type::p_left, _c_vel, _upOff, ballMakyr));
                _rangedCounter = 0;
                break;
            case SwipeController::chargedDown:
                _pending.emplace(Attack::alloc(p, 3, 10, 9001, _scale, Type::p_left, cugl::Vec2::ZERO, _leftOff,  ballMakyr));
                _pending.emplace(Attack::alloc(p, 3, 10, 9001, _scale, Type::p_left, cugl::Vec2::ZERO, _rightOff, ballMakyr));
                _rangedCounter = 0;
                break;
            default:
                break;
        }
    }
}


/**
 * Right size represents melee in this case.
 */
void AttackController::attackRight(cugl::Vec2 p, SwipeController::SwipeAttack attack, bool grounded) {
    if (_meleeCounter > _swing) {
        switch (attack) {
            case SwipeController::leftAttack:
                if (_melee == cool) {
                    break;
                } else if (_melee == h2_left && _multiCounter < _hit_window) {
                    _pending.emplace(Attack::alloc(p, 2, 3, 9001, _scale, Type::p_right, cugl::Vec2::ZERO, _leftOff, ballMakyr));
                    _melee = h3_left;
                } else if (_melee == h3_left && _multiCounter < _hit_window) {
                    _pending.emplace(Attack::alloc(p, 2.5, 3, 9001, _scale, Type::p_right, cugl::Vec2::ZERO, _leftOff, ballMakyr));
                    _melee = cool;
                } else {
                    _pending.emplace(Attack::alloc(p, 1.5, 3, 9001, _scale, Type::p_right, cugl::Vec2::ZERO, _leftOff, ballMakyr));
                    _melee = h2_left;
                }
                break;
            case SwipeController::rightAttack:
                if (_melee == cool) {
                    break;
                } else if (_melee == h2_right && _multiCounter < _hit_window) {
                    _pending.emplace(Attack::alloc(p, 2, 3, 9001, _scale, Type::p_right, cugl::Vec2::ZERO, _rightOff, ballMakyr));
                    _melee = h3_right;
                } else if (_melee == h3_right && _multiCounter < _hit_window) {
                    _pending.emplace(Attack::alloc(p, 2.5, 3, 9001, _scale, Type::p_right, cugl::Vec2::ZERO, _rightOff, ballMakyr));
                    _melee = cool;
                } else {
                    _pending.emplace(Attack::alloc(p, 1.5, 3, 9001, _scale, Type::p_right, cugl::Vec2::ZERO, _rightOff, ballMakyr));
                    _melee = h2_right;
                }
                break;
            case SwipeController::upAttack:
                _pending.emplace(Attack::alloc(p, 1.5, 5, 9001, _scale, Type::p_right, cugl::Vec2::ZERO, _upOff, ballMakyr));
                break;
            case SwipeController::downAttack:
                if(!grounded){
                _pending.emplace(Attack::alloc(p, 1.5, 5, 9001, _scale, Type::p_right, cugl::Vec2::ZERO, _downOff, ballMakyr));
                } else{
                    _pending.emplace(Attack::alloc(p, 1.5, 3, 9001, _scale, Type::p_right, cugl::Vec2::ZERO, _leftOff, ballMakyr));
                    _pending.emplace(Attack::alloc(p, 1.5, 3, 9001, _scale,  Type::p_right, cugl::Vec2::ZERO, _rightOff, ballMakyr));
                }
                break;
            default:
                break;
        }
    }
}

        
void AttackController::createAttack(cugl::Vec2 p, float radius, float age, float damage, Type s, cugl::Vec2 vel) {
    _pending.emplace(Attack::alloc(p, radius, age, damage, _scale, cugl::Size(1,1), s, cugl::Vec2::ZERO, ballMakyr, vel));
}

//void AttackController::createEnemyAttack(cugl::Vec2 pos, float radius, float age, int damage, float scale, cugl::Size size, //cugl::Vec2 offset, cugl::Vec2 vel) {
  //  _pending.emplace(Attack::alloc(pos, radius, age, damage, scale, size, Type::enemy, offset, ballMakyr, vel));
//}

//void AttackController::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
//    for(auto it = _current.begin(); it != _current.end(); ++it) {
//        //cugl::Vec2 center = cugl::Vec2((*it)->getRadius(),(*it)->getRadius());
//        cugl::Affine2 trans;
        
                //trans.scale(_scale);
                //trans.translate((*it)->getPosition()*_scale);
//                if ((*it)->getSide() == Side::left) {
//                    batch->setColor(cugl::Color4::GREEN);
//                    batch->fill((*it)->getBall(), cugl::Vec2::ZERO, cugl::Vec2(_scale, _scale), 0, (*it)->getPosition()*_scale);
//                } else {
//                    batch->setColor(cugl::Color4::RED);
//                    batch->fill((*it)->getBall(), cugl::Vec2::ZERO, cugl::Vec2(_scale, _scale), 0, (*it)->getPosition()*_scale);
//                }
        
        //trans.scale(_scale);
        //trans.translate((*it)->getPosition()*_scale);
//        b2Vec2 pos = (*it)->getBody()->GetPosition();
//        cugl::Vec2 pos2 = (*it)->getPosition();

//        if ((*it)->isPlayerAttack()) {
//            if ((*it)->getSide() == Side::left) {
//                batch->setColor(cugl::Color4::GREEN);
//                batch->fill((*it)->getBall(), cugl::Vec2::ZERO, cugl::Vec2(_scale, _scale), 0, (*it)->getPosition() * _scale);
//            }
//            else {
//                batch->setColor(cugl::Color4::RED);
//                batch->fill((*it)->getBall(), cugl::Vec2::ZERO, cugl::Vec2(_scale, _scale), 0, cugl::Vec2(pos.x, pos.y) * _scale);
//            }
//        }
//        else {
//            batch->setColor(cugl::Color4::YELLOW);
//            batch->fill((*it)->getBall(), cugl::Vec2::ZERO, cugl::Vec2(_scale, _scale), 0, (*it)->getPosition() * _scale);
//        }
//    }
//}

void AttackController::Attack::resetDebug() {
    CapsuleObstacle::resetDebug();
    float w = ATTACK_SSHRINK * _dimension.width;
    float h = SENSOR_HEIGHT;
    Poly2 poly(Rect(-w / 2.0f, -h / 2.0f, w, h));

    _sensorNode = scene2::WireNode::allocWithTraversal(poly, poly2::Traversal::INTERIOR);
    _sensorNode->setColor(DEBUG_COLOR);
    _sensorNode->setPosition(Vec2(_debug->getContentSize().width / 2.0f, 0.0f));
    _debug->addChild(_sensorNode);
}
