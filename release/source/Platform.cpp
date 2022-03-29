//
//  Platform.cpp
//  LiminalSpirit
//
//  Created by Janice Wei on 3/14/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "Platform.hpp"
using namespace cugl;
#define DEBUG_COLOR Color4::RED
/** Initializes the player at the given position*/
bool PlatformModel::init(const cugl::Vec2& pos, const float width, const float height, float scale) {
    Size nsize;
    nsize.width = width;
    _width = width;
    _height = height;
    _leftBotPos = pos;
    nsize.height = height;
    _drawScale = scale;

    if (BoxObstacle::init(pos, nsize)) {
        setDensity(0);
        setFriction(0.0f);
        setFixedRotation(true);
        b2Filter filter = b2Filter();
        filter.categoryBits = 0b100;
        //filter.maskBits = 0b1100;
        setFilterData(filter);

        return true;
    }
    return false;
}
void PlatformModel::dispose() {
    _node = nullptr;
    _sensorTopNode = nullptr;
    _sensorBottomNode = nullptr;
}

/** Creates new fixtures for the body and defines shape*/
void PlatformModel::createFixtures() {
    if (_body == nullptr) {
        return;
    }

    BoxObstacle::createFixtures();
    b2FixtureDef sensorDef;
    sensorDef.density = 0;
    sensorDef.isSensor = true;

    // Top Sensor dimensions
    b2Vec2 corners[4];
    corners[0].x = -getWidth() / 2.0f;
    corners[0].y =0;
    corners[1].x = -getWidth() / 2.0f;
    corners[1].y = getHeight() / 2.0f;
    corners[2].x = getWidth() / 2.0f;
    corners[2].y = getHeight()  / 2.0f;
    corners[3].x = getWidth() / 2.0f;
    corners[3].y = 0;

    b2PolygonShape sensorShape;
    sensorShape.Set(corners, 4);

    sensorDef.shape = &sensorShape;
    sensorDef.userData.pointer = reinterpret_cast<uintptr_t>(getTopSensorName());
    _sensorFixtureTop = _body->CreateFixture(&sensorDef);
    
    // Bottom Sensor dimensions
    b2Vec2 cornersB[4];
    cornersB[0].x = -getWidth() / 2.0f;
    cornersB[0].y =0;
    cornersB[1].x = -getWidth() / 2.0f;
    cornersB[1].y = -getHeight() / 2.0f;
    cornersB[2].x = getWidth() / 2.0f;
    cornersB[2].y = -getHeight()  / 2.0f;
    cornersB[3].x = getWidth() / 2.0f;
    cornersB[3].y = 0;

    sensorShape.Set(cornersB, 4);
    sensorDef.shape = &sensorShape;
    sensorDef.userData.pointer = reinterpret_cast<uintptr_t>(getBottomSensorName());
    _sensorFixtureBottom = _body->CreateFixture(&sensorDef);
}

/** Releases the fixtures for this body, resets the shape*/
void PlatformModel::releaseFixtures() {
    if (_body != nullptr) {
        return;
    }

    BoxObstacle::releaseFixtures();
    if (_sensorFixtureTop != nullptr) {
        _body->DestroyFixture(_sensorFixtureTop);
        _sensorFixtureTop = nullptr;
    }
    if (_sensorFixtureBottom != nullptr) {
        _body->DestroyFixture(_sensorFixtureBottom);
        _sensorFixtureBottom = nullptr;
    }
}

void PlatformModel::update(float dt) {
    BoxObstacle::update(dt);
    if (_node != nullptr) {
        _node->setPosition(getPosition() * _drawScale);
        _node->setAngle(getAngle());
    }
}

void PlatformModel::resetDebug() {
    BoxObstacle::resetDebug();
    Poly2 poly(Rect(-_width/2.0f, 0, _width, _height/2.0f));
    _sensorTopNode = scene2::WireNode::allocWithTraversal(poly, poly2::Traversal::INTERIOR);
    _sensorTopNode->setColor(Color4::RED);
    _sensorTopNode->setPosition(_debug->getContentSize().width / 2.0f, _debug->getContentSize().height / 4);
    _debug->addChild(_sensorTopNode);
    Poly2 polyB(Rect(-_width/2.0f, -_height/2.0f, _width, _height/2.0f));
    _sensorBottomNode = scene2::WireNode::allocWithTraversal(polyB, poly2::Traversal::INTERIOR);
    _sensorBottomNode->setColor(Color4::RED);
    _sensorBottomNode->setPosition(_debug->getContentSize().width / 2.0f, 3*_debug->getContentSize().height / 4);
    _debug->addChild(_sensorBottomNode);
}
