//
//  AttackController.cpp
//  LiminalSpirit
//
//  Created by Sashimi Software on 3/2/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "AttackController.hpp"

AttackController::Attack::Attack(const cugl::Vec2 p, float radius, float a, float dmg, float scale, Side s, cugl::Vec2 vel, cugl::Vec2 oof, cugl::PolyFactory b) {
    
    position = (p + oof);
    _radius = radius;
    age = a;
    damage = dmg;
    side = s;
    _scale = scale;
    _vel = vel;
    offset = oof;
    active = true;
    ball = b.makeCircle(cugl::Vec2::ZERO, radius);
}


void AttackController::Attack::update(const cugl::Vec2 p, bool follow) {
    if (active) {
        if (follow) {
            position = p + offset;
        }
        position = position + _vel;
        age -= 1;
        if (age == 0) {
            active =  false;
        }
    }
}

AttackController::AttackController() {
    //need to add initialization for left and right offsets
}

void AttackController::init(float scale, cugl::Vec2 oof, cugl::Vec2 p_vel, cugl::Vec2 c_vel) {
    _scale = scale;
    leftOff = cugl::Vec2(-1.5f, 0.0f) + (oof / (2 * scale));
    rightOff = cugl::Vec2(1.5f, 0.0f) + (oof / (2 * scale));
    upOff = cugl::Vec2(0, 1.5f) + (oof / (2 * scale));
    downOff = cugl::Vec2(0, -1.5f) + (oof / (2 * scale));
    _p_vel = p_vel;
    _c_vel = c_vel;
}

void AttackController::update(const cugl::Vec2 p) {
    
    auto it = _current.begin();
    while(it != _current.end()) {
        if ((*it)->getSide() == Side::left) {
            (*it)->update(p, false);
        } else {
            (*it)->update(p, true);
        }
        if (!((*it)->isActive())) {
            it = _current.erase(it);
        } else {
            it++;
        }
    }
    
    for(auto it = _pending.begin(); it != _pending.end(); ++it) {
        _current.emplace(*it);
    }
    _pending.clear();
}
            
    
void AttackController::attackLeft(cugl::Vec2 p, SwipeController::SwipeAttack attack, bool grounded) {
    switch (attack) {
        case SwipeController::leftAttack:
            _pending.emplace(std::make_shared<Attack>(p, 1, 30, 9001, _scale, Side::left, cugl::Vec2(_p_vel).rotate(M_PI * 0.5) ,leftOff, ballMakyr));
            break;
        case SwipeController::rightAttack:
            _pending.emplace(std::make_shared<Attack>(p, 1, 30, 9001, _scale, Side::left, cugl::Vec2(_p_vel).rotate(M_PI * 1.5), rightOff, ballMakyr));
            break;
        case SwipeController::upAttack:
            _pending.emplace(std::make_shared<Attack>(p, 1, 30, 9001, _scale, Side::left, _p_vel, upOff, ballMakyr));
            break;
        case SwipeController::downAttack:
            if(!grounded){
            _pending.emplace(std::make_shared<Attack>(p, 1, 30, 9001, _scale, Side::left, cugl::Vec2(_p_vel).rotate(M_PI), downOff, ballMakyr));
            } else{
                _pending.emplace(std::make_shared<Attack>(p, 1, 4, 9001, _scale, Side::left, cugl::Vec2(_p_vel).rotate(M_PI * 0.5), leftOff, ballMakyr));
                _pending.emplace(std::make_shared<Attack>(p, 1, 4, 9001, _scale, Side::left, cugl::Vec2(_p_vel).rotate(M_PI * 1.5), rightOff, ballMakyr));
            }
            break;
        case SwipeController::chargedLeft:
            _pending.emplace(std::make_shared<Attack>(p, 5, 30, 9001, _scale, Side::left, cugl::Vec2(_c_vel).rotate(M_PI * 0.5) ,leftOff, ballMakyr));
            break;
        case SwipeController::chargedRight:
            _pending.emplace(std::make_shared<Attack>(p, 5, 30, 9001, _scale, Side::left, cugl::Vec2(_c_vel).rotate(M_PI * 1.5), rightOff, ballMakyr));
            break;
        case SwipeController::chargedUp:
            _pending.emplace(std::make_shared<Attack>(p, 5, 30, 9001, _scale, Side::left, _c_vel, upOff, ballMakyr));
            break;
        case SwipeController::chargedDown:
            _pending.emplace(std::make_shared<Attack>(p, 3, 10, 9001, _scale, Side::left, cugl::Vec2::ZERO, leftOff,  ballMakyr));
            _pending.emplace(std::make_shared<Attack>(p, 3, 10, 9001, _scale, Side::left, cugl::Vec2::ZERO, rightOff, ballMakyr));
            break;
        default:
            break;
    }
}

/**
 * Right size represents melee in this case.
 */
void AttackController::attackRight(cugl::Vec2 p, SwipeController::SwipeAttack attack, bool grounded) {
    switch (attack) {
        case SwipeController::leftAttack:
            _pending.emplace(std::make_shared<Attack>(p, 2, 3, 9001, _scale, Side::right, cugl::Vec2::ZERO, leftOff, ballMakyr));
            break;
        case SwipeController::rightAttack:
            _pending.emplace(std::make_shared<Attack>(p, 2, 3, 9001, _scale, Side::right, cugl::Vec2::ZERO, rightOff, ballMakyr));
            break;
        case SwipeController::upAttack:
            _pending.emplace(std::make_shared<Attack>(p, 2, 5, 9001, _scale, Side::right, cugl::Vec2::ZERO, upOff, ballMakyr));
            break;
        case SwipeController::downAttack:
            if(!grounded){
            _pending.emplace(std::make_shared<Attack>(p, 2, 5, 9001, _scale, Side::right, cugl::Vec2::ZERO, downOff, ballMakyr));
            } else{
                _pending.emplace(std::make_shared<Attack>(p, 2, 3, 9001, _scale, Side::right, cugl::Vec2::ZERO, leftOff, ballMakyr));
                _pending.emplace(std::make_shared<Attack>(p, 2, 3, 9001, _scale,  Side::right, cugl::Vec2::ZERO, rightOff, ballMakyr));
            }
            break;
        default:
            break;
    }
}

void AttackController::createAttack(cugl::Vec2 p, float radius, float age, float damage, Side s, cugl::Vec2 vel) {
    _pending.emplace(std::make_shared<Attack>(p, radius, age, damage, _scale, s, vel, cugl::Vec2::ZERO, ballMakyr));
}

void AttackController::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    for(auto it = _current.begin(); it != _current.end(); ++it) {
        //cugl::Vec2 center = cugl::Vec2((*it)->getRadius(),(*it)->getRadius());
        cugl::Affine2 trans;
        //trans.scale(_scale);
        //trans.translate((*it)->getPosition()*_scale);
        if ((*it)->getSide() == Side::left) {
            batch->setColor(cugl::Color4::GREEN);
            batch->fill((*it)->getBall(), cugl::Vec2::ZERO, cugl::Vec2(_scale, _scale), 0, (*it)->getPosition()*_scale);
        } else {
            batch->setColor(cugl::Color4::RED);
            batch->fill((*it)->getBall(), cugl::Vec2::ZERO, cugl::Vec2(_scale, _scale), 0, (*it)->getPosition()*_scale);
        }
    }
}

