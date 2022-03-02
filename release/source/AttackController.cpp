//
//  AttackController.cpp
//  LiminalSpirit
//
//  Created by Sashimi Software on 3/2/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "AttackController.hpp"

AttackController::Attack::Attack(const cugl::Vec2 p, float r, float a, float dmg, float scale, Side s, cugl::Vec2 oof, cugl::PolyFactory b) {
    
    position = p;
    radius = r;
    age = a;
    damage = dmg;
    side = s;
    _scale = scale;
    offset = oof;
    active = true;
    ball = b.makeCircle(p+oof, r);
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
        case SwipeController::left:
            _pending.emplace(std::make_shared<Attack>(p, 3, 3, 9001, 1.5, Side::left, leftOff, ballMakyr));
        case SwipeController::right:
            _pending.emplace(std::make_shared<Attack>(p, 3, 3, 9001, 1.5, Side::left, rightOff, ballMakyr));
        case SwipeController::up:
        case SwipeController::down:
        case SwipeController::none:
            break;
    }
}

void AttackController::attackRight(cugl::Vec2 p, SwipeController::Swipe direction) {
    switch (direction) {
        case SwipeController::left:
            _pending.emplace(std::make_shared<Attack>(p, 3, 3, 9001, 1.5, Side::right, leftOff, ballMakyr));
        case SwipeController::right:
            _pending.emplace(std::make_shared<Attack>(p, 3, 3, 9001, 1.5, Side::right, rightOff, ballMakyr));
        case SwipeController::up:
        case SwipeController::down:
        case SwipeController::none:
            break;
    }
}

void AttackController::setLeftOffset (const cugl::Vec2 l) {
    leftOff = l;
}

void AttackController::setRightOffset (const cugl::Vec2 r) {
    rightOff = r;
}


