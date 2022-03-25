//
//  Platform.hpp
//  LiminalSpirit
//
//  Created by Chenyu Wei on 3/14/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef Platform_hpp
#define Platform_hpp
#include <cugl/physics2/CUBoxObstacle.h>
#include <cugl/physics2/CUCapsuleObstacle.h>
#include <cugl/scene2/graph/CUWireNode.h>
#include <stdio.h>

using namespace cugl;
#define TOP_SENSOR_NAME "platformtopsensor"
#define BOTTOM_SENSOR_NAME "platformbottomsensor"
class PlatformModel : public cugl::physics2::BoxObstacle {

protected:
    
    /** top sensor */
    b2Fixture* _sensorFixtureTop;
    /** Name of top sensor */
    std::string _sensorTopName;
    /** bottom sensor */
    b2Fixture* _sensorFixtureBottom;
    /** Name of bottom sensor */
    std::string _sensorBottomName;
    /** Debug Sensor */
    std::shared_ptr<cugl::scene2::WireNode> _sensorTopNode;
    std::shared_ptr<cugl::scene2::WireNode> _sensorBottomNode;
    /** Scene Graph Node */
    std::shared_ptr<cugl::scene2::SceneNode> _node;
    /** Draw Scale*/
    float _drawScale;
    float _width;
    float _height;
    Vec2 _leftBotPos;
    /** Redraws outline of physics fixtures */
    virtual void resetDebug() override;

public:

#pragma mark Hidden Constructors
    /** Creates a degenerate Enemy */
    PlatformModel() : BoxObstacle(), _sensorTopName(TOP_SENSOR_NAME), _sensorBottomName(BOTTOM_SENSOR_NAME) { }

    /**Destroys this Base Enemy Model, releasing all resources */
    virtual ~PlatformModel(void) { dispose(); }

    /**Disposes all resources and assets of this Enemy Model */
    void dispose();

    /** Base init function (includes scale) */
    virtual bool init(const cugl::Vec2& pos, const float width, const float height, float scale);

#pragma mark -
#pragma mark Static Constructors

    static std::shared_ptr<PlatformModel> alloc(const cugl::Vec2& pos, const float width, const float height, float scale) {
        std::shared_ptr<PlatformModel> result = std::make_shared<PlatformModel>();
        return (result->init(pos, width, height, scale) ? result : nullptr);
    }

#pragma mark -
#pragma mark Scene Node
    /** Returns the scene graph node representing this enemy*/
    const std::shared_ptr<cugl::scene2::SceneNode>& getSceneNode() const { return _node; }

    /** Sets the scene graph node representing this enemy */
    void setSceneNode(const std::shared_ptr<cugl::scene2::SceneNode>& node) {
        _node = node;
        _node->setPosition(getPosition() * _drawScale);
    }

#pragma mark -
#pragma mark Attribute Properties

    /** Returns the name of the ground sensor */
    std::string* getBottomSensorName() { return &_sensorBottomName; }
    /** Returns the name of the ground sensor */
    std::string* getTopSensorName() { return &_sensorTopName; }
    float getWidth() {return _width;}
    float getHeight() {return _height;}
    Vec2 getLeftBotPos() {return _leftBotPos;}

#pragma mark -
#pragma mark Physics Methods
    /**Creates and adds the physics body(s) to the world */
    void createFixtures() override;

    /** Releases the fixtures of this body(s) from the world */
    void releaseFixtures() override;
    /** Updates the object's physics state (not game logic) */
    void update(float dt) override;
    /** Applies the force of the body of this enemy */

};
#endif /* Platform_hpp */
