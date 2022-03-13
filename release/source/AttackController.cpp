//
//  AttackController.cpp
//  LiminalSpirit
//
//  Created by Sashimi Software on 3/2/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "AttackController.hpp"
#include "PlayerModel.h"

bool AttackController::Attack::init(const cugl::Vec2 p,float a, float dmg, float scale, Side s, cugl::Vec2 oof, cugl::PolyFactory b, boolean playerAttack) {
    
    position = (p + oof);
    radius = 2;
    age = a;
    damage = dmg;
    side = s;
    _scale = scale;
    offset = oof;
    active = true;
    ball = b.makeCircle(position, radius);
    if (CapsuleObstacle::init(position)) {
        if (playerAttack) {
            _sensorName = "player" + _sensorName;
        }
        else {
            _sensorName = "enemy" + _sensorName;
        }
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

    std::vector<cugl::Vec2> cuglVerts = ball.getVertices();
    int n = cuglVerts.size();
    std::vector<b2Vec2>* verts = new vector<b2Vec2>(0);
    //Following is a temporary copy fix, hopefully will find a better solution.
    for (auto it = cuglVerts.begin(); it != cuglVerts.end();  ++it) {
        verts->push_back(b2Vec2((*it).x, (*it).y));
    }
    sensorShape.Set(verts->data(), verts->size());
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


void AttackController::Attack::update(const cugl::Vec2 p, b2Vec2 VX, bool follow) {
    if (active) {
        if (follow) {
            position = p + offset;
            _body->SetLinearVelocity(VX);
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

void AttackController::init(float scale, cugl::Vec2 oof, std::shared_ptr<PlayerModel> player) {
    _scale = scale;
    _player = player;
    leftOff = cugl::Vec2(-1.5f, 0.0f) + (oof / (2 * scale));
    rightOff = cugl::Vec2(1.5f, 0.0f) + (oof / (2 * scale));
    upOff = cugl::Vec2(0, 1.5f) + (oof / (2 * scale));
    downOff = cugl::Vec2(0, -1.5f) + (oof / (2 * scale));
}

void AttackController::update(const cugl::Vec2 p, b2Vec2 VX) {
    
    auto it = _current.begin();
    while(it != _current.end()) {
        (*it)->update(p, VX, true);
        if (!((*it)->isActive())) {
            (*it)->markRemoved(true);
            if ((*it)->isRemoved()) {
                int breaking = 1;
            }
            //it = _current.erase(it);
            it++;
        } else {
            it++;
        }
    }
    
    for(auto it = _pending.begin(); it != _pending.end(); ++it) {
        _current.emplace(*it);
    }
    _pending.clear();
}
            
    
void AttackController::attackLeft(SwipeController::Swipe direction, bool grounded) {
    
    switch (direction) {
        case SwipeController::Swipe::left:
            _pending.emplace(Attack::alloc(_player->getPosition(), 3, 9001, _scale, Side::left, leftOff, ballMakyr, true));
            break;
        case SwipeController::Swipe::right:
            _pending.emplace(Attack::alloc(_player->getPosition(), 3, 9001, _scale, Side::left, rightOff, ballMakyr, true));
            break;
        case SwipeController::up:
            _pending.emplace(Attack::alloc(_player->getPosition(), 5, 9001, _scale, Side::left, upOff, ballMakyr, true));
            break;
        case SwipeController::down:
            if(!grounded){
            _pending.emplace(Attack::alloc(_player->getPosition(), 5, 9001, _scale, Side::left, downOff, ballMakyr, true));
            } else{
                _pending.emplace(Attack::alloc(_player->getPosition(), 3, 9001, _scale, Side::left, leftOff, ballMakyr, true));
                _pending.emplace(Attack::alloc(_player->getPosition(), 3, 9001, _scale, Side::left, rightOff, ballMakyr, true));
            }
            break;
        case SwipeController::none:
            break;
    }
}

void AttackController::attackRight(SwipeController::Swipe direction, bool grounded) {
    switch (direction) {
        case SwipeController::Swipe::left:
            _pending.emplace(Attack::alloc(_player->getPosition(), 3, 9001, _scale, Side::right, leftOff, ballMakyr, true));
            break;
        case SwipeController::Swipe::right:
            _pending.emplace(Attack::alloc(_player->getPosition(), 3, 9001, _scale, Side::right, rightOff, ballMakyr, true));
            break;
        case SwipeController::up:
            _pending.emplace(Attack::alloc(_player->getPosition(), 5, 9001, _scale, Side::right, upOff, ballMakyr, true));
            break;
        case SwipeController::down:
            if(!grounded){
            _pending.emplace(Attack::alloc(_player->getPosition(), 5, 9001, _scale, Side::right, downOff, ballMakyr, true));
            } else{
                _pending.emplace(Attack::alloc(_player->getPosition(), 3, 9001, _scale, Side::right, leftOff, ballMakyr, true));
                _pending.emplace(Attack::alloc(_player->getPosition(), 3, 9001, _scale, Side::right, rightOff, ballMakyr, true));
            }
            break;
        case SwipeController::none:
            break;
    }
}

void AttackController::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    for(auto it = _current.begin(); it != _current.end(); ++it) {
        //cugl::Vec2 center = cugl::Vec2((*it)->getRadius(),(*it)->getRadius());
        cugl::Affine2 trans;
        //trans.scale(_scale);
        //trans.translate((*it)->getPosition()*_scale);
        b2Vec2 pos = (*it)->getBody()->GetPosition();
        /*
        if ((*it)->getSide() == Side::left) {
            batch->setColor(cugl::Color4::GREEN);
            batch->fill((*it)->getBall(), cugl::Vec2::ZERO, cugl::Vec2(_scale, _scale), 0, cugl::Vec2(pos.x, pos.y) * _scale);
        } else {
            batch->setColor(cugl::Color4::RED);
            batch->fill((*it)->getBall(), cugl::Vec2::ZERO, cugl::Vec2(_scale, _scale), 0, cugl::Vec2(pos.x, pos.y) * _scale);
        }
        */
    }
}

