//
//  AttackController.cpp
//  LiminalSpirit
//
//  Created by Sashimi Software on 3/2/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "AttackController.hpp"

AttackController::Attack::Attack(const cugl::Vec2 p,float a, float dmg, float scale, Side s, cugl::Vec2 oof, cugl::PolyFactory b) {
    
    position = p+oof;
    radius = 2;
    age = a;
    damage = dmg;
    side = s;
    _scale = scale;
    offset = oof;
    active = true;
    ball = b.makeCircle(cugl::Vec2(radius, radius), radius);
}


void AttackController::Attack::update(const cugl::Vec2 p, bool follow) {
    if (active) {
        if (follow) {
            position = p + offset;
        }
        age -= 1;
        if (age == 0) {
            active =  false;
        }
    }
}

AttackController::AttackController() {
    //need to add initialization for left and right offsets
    leftOff = cugl::Vec2(-1.0f, 0.0f);
    rightOff = cugl::Vec2(1.0f, 0.0f);
}

void AttackController::init(float scale) {
    _scale = scale;
}

void AttackController::update(const cugl::Vec2 p) {
    
    auto it = _current.begin();
    while(it != _current.end()) {
        (*it)->update(p, true);
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
            
    
void AttackController::attackLeft(cugl::Vec2 p, SwipeController::Swipe direction) {
    
    switch (direction) {
        case SwipeController::Swipe::left:
            _pending.emplace(std::make_shared<Attack>(p, 3, 9001, _scale, Side::left, leftOff, ballMakyr));
            break;
        case SwipeController::Swipe::right:
            _pending.emplace(std::make_shared<Attack>(p, 3, 9001, _scale, Side::left, rightOff, ballMakyr));
            break;
        case SwipeController::up:
        case SwipeController::down:
        case SwipeController::none:
            break;
    }
}

void AttackController::attackRight(cugl::Vec2 p, SwipeController::Swipe direction) {
    switch (direction) {
        case SwipeController::Swipe::left:
            _pending.emplace(std::make_shared<Attack>(p, 3, 9001, _scale, Side::right, leftOff, ballMakyr));
            break;
        case SwipeController::Swipe::right:
            _pending.emplace(std::make_shared<Attack>(p, 3, 9001, _scale, Side::right, rightOff, ballMakyr));
            break;
        case SwipeController::up:
        case SwipeController::down:
        case SwipeController::none:
            break;
    }
}

void AttackController::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    for(auto it = _current.begin(); it != _current.end(); ++it) {
        cugl::Affine2 trans;
        trans.scale(_scale);
        trans.translate((*it)->getPosition());
        if ((*it)->getSide() == Side::left) {
            batch->setColor(cugl::Color4::GREEN);
            batch->fill((*it)->getBall(), cugl::Vec2::ZERO, trans);
        } else {
            batch->setColor(cugl::Color4::RED);
            batch->fill((*it)->getBall(), cugl::Vec2::ZERO, trans);
        }
    }
}

