//
//  LiminalSpirit.cpp
//  Cornell University Game Library (CUGL)
//
//  This is the implementation file for the custom application. This is the
//  definition of your root (and in this case only) class.
//
//  CUGL zlib License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 1/8/17
//
// Include the class header, which includes all of the CUGL classes
#include "LiminalSpiritApp.h"
#include <cugl/base/CUBase.h>
#include "BaseEnemyModel.h"

// Add support for simple random number generation
#include <cstdlib>
#include <ctime>

// This keeps us from having to write cugl:: all the time
using namespace cugl;

// The number of frames before moving the logo to a new position
#define TIME_STEP 60
/** This is adjusted by screen aspect ratio to get the height */
#define SCENE_WIDTH 1024
#define SCENE_HEIGHT 576
/** Width of the game world in Box2d units */
#define DEFAULT_WIDTH 32.0f
/** Height of the game world in Box2d units */
#define DEFAULT_HEIGHT 18.0f
/** The constant for gravity in the physics world. */
#define GRAVITY 9.8

/** The initial position of the dude */
float ENEMY_POS[] = {500.0f, 300.0f};

static void test_cases()
{
}

/**
 * The method called after OpenGL is initialized, but before running the application.
 *
 * This is the method in which all user-defined program intialization should
 * take place.  You should not create a new init() method.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to FOREGROUND,
 * causing the application to run.
 */
void LiminalSpirit::onStartup()
{
    Size size = getDisplaySize();
    size *= SCENE_WIDTH / size.width;

    // Create a scene graph the same size as the window
    _scene = Scene2::alloc(size.width, size.height);
    // Create a sprite batch (and background color) to render the scene
    _batch = SpriteBatch::alloc();
    setClearColor(Color4(229, 229, 229, 255));

    // Create an asset manager to load all assets
    _assets = AssetManager::alloc();

    // You have to attach the individual loaders for each asset type
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<Font>(FontLoader::alloc()->getHook());

    // This reads the given JSON file and uses it to load all other assets
    _assets->loadDirectory("json/assets.json");

    // Activate mouse or touch screen input as appropriate
    // We have to do this BEFORE the scene, because the scene has a button
#if defined(CU_TOUCH_SCREEN)
    Input::activate<Touchscreen>();
#else
    Input::activate<Mouse>();
#endif

    // Build the scene from these assets
    Application::onStartup();

    // Report the safe area
    Rect bounds = Display::get()->getSafeBounds();
    CULog("Safe Area %sx%s", bounds.origin.toString().c_str(),
          bounds.size.toString().c_str());

    bounds = getSafeBounds();
    CULog("Safe Area %sx%s", bounds.origin.toString().c_str(),
          bounds.size.toString().c_str());

    bounds = getDisplayBounds();
    CULog("Full Area %sx%s", bounds.origin.toString().c_str(),
          bounds.size.toString().c_str());

    // Enable physics -jdg274
    bounds = Display::get()->getSafeBounds();
    _scale = size.width == SCENE_WIDTH ? size.width / DEFAULT_WIDTH : size.height / DEFAULT_HEIGHT;
    Vec2 offset((size.width - SCENE_WIDTH) / 2.0f, (size.height - SCENE_HEIGHT) / 2.0f);

    // Create the scene graph
    _worldnode = scene2::SceneNode::alloc();
    _worldnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _worldnode->setPosition(offset);
    _scene->addChild(_worldnode);

    _world = physics2::ObstacleWorld::alloc(Rect(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT), Vec2(0, -GRAVITY));
    _world->activateCollisionCallbacks(true);
    //    _world->onBeginContact = [this](b2Contact* contact) {
    //      beginContact(contact);
    //    };
    //    _world->onEndContact = [this](b2Contact* contact) {
    //      endContact(contact);
    //    };

    buildScene();
}

/**
 * The method called when the application is ready to quit.
 *
 * This is the method to dispose of all resources allocated by this
 * application.  As a rule of thumb, everything created in onStartup()
 * should be deleted here.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to NONE,
 * causing the application to be deleted.
 */
void LiminalSpirit::onShutdown()
{
    // Delete all smart pointers
    _logo = nullptr;
    _scene = nullptr;
    _batch = nullptr;
    _assets = nullptr;
    _world = nullptr;
    _worldnode = nullptr;
    _enemy = nullptr;

    // Deativate input
#if defined CU_TOUCH_SCREEN
    Input::deactivate<Touchscreen>();
#else
    Input::deactivate<Mouse>();
#endif
    Application::onShutdown();
}

/**
 * The method called to update the application data.
 *
 * This is your core loop and should be replaced with your custom implementation.
 * This method should contain any code that is not an OpenGL call.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void LiminalSpirit::update(float timestep)
{
    if (_countdown == 0)
    {
        // Move the logo about the screen
        Size size = getDisplaySize();
        size *= SCENE_WIDTH / size.width;
        float x = (float)(std::rand() % (int)(size.width / 2)) + size.width / 4;
        float y = (float)(std::rand() % (int)(size.height / 2)) + size.height / 8;
        _logo->setPosition(Vec2(x, y));
        _countdown = TIME_STEP;
    }
    else
    {
        _countdown--;
    }
    _world->update(timestep);
}

/**
 * The method called to draw the application to the screen.
 *
 * This is your core loop and should be replaced with your custom implementation.
 * This method should OpenGL and related drawing calls.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 */
void LiminalSpirit::draw()
{
    // This takes care of begin/end
    _scene->render(_batch);
}

/**
 * Internal helper to build the scene graph.
 *
 * Scene graphs are not required.  You could manage all scenes just like
 * you do in 3152.  However, they greatly simplify scene management, and
 * have become standard in most game engines.
 */
void LiminalSpirit::buildScene()
{
    Size size = getDisplaySize();
    float scale = SCENE_WIDTH / size.width;
    size *= scale;

    // The logo is actually an image+label.  We need a parent node
    _logo = scene2::SceneNode::alloc();

    // Get the image and add it to the node.
    std::shared_ptr<Texture> texture = _assets->get<Texture>("logo");
    _logo = scene2::PolygonNode::allocWithTexture(texture);
    _logo->setScale(0.2f); // Magic number to rescale asset

    // Put the logo in the middle of the screen
    _logo->setAnchor(Vec2::ANCHOR_CENTER);
    _logo->setPosition(size.width / 2, size.height / 2);

    // Create a button.  A button has an up image and a down image
    std::shared_ptr<Texture> up = _assets->get<Texture>("close-normal");
    std::shared_ptr<Texture> down = _assets->get<Texture>("close-selected");

    Size bsize = up->getSize();
    std::shared_ptr<scene2::Button> button = scene2::Button::alloc(scene2::PolygonNode::allocWithTexture(up),
                                                                   scene2::PolygonNode::allocWithTexture(down));

    // Create a callback function for the button
    button->setName("close");
    button->addListener([=](const std::string &name, bool down)
                        {
        // Only quit when the button is released
        if (!down) {
            CULog("Goodbye!");
            this->quit();
        } });

    // Find the safe area, adapting to the iPhone X
    Rect safe = getSafeBounds();
    safe.origin *= scale;
    safe.size *= scale;

    // Get the right and bottom offsets.
    float bOffset = safe.origin.y;
    float rOffset = (size.width) - (safe.origin.x + safe.size.width);

    // Making left and top offsets -jdg274
    float lOffset = safe.origin.x;
    float tOffset = (size.height) - (safe.origin.y + safe.size.height);

    // Making the floor -jdg274
    Rect floorRect = Rect(22, 8, 10, 10);
    std::shared_ptr<physics2::PolygonObstacle> floor = physics2::PolygonObstacle::allocWithAnchor(floorRect, Vec2::ANCHOR_CENTER);
    floor->setBodyType(b2_staticBody);
    std::shared_ptr<scene2::PolygonNode> floorNode = scene2::PolygonNode::allocWithPoly(floorRect);
    floorNode->setColor(Color4::BLUE);
    addObstacle(floor, floorNode, 1);
    //    std::shared_ptr<scene2::PolygonNode> floor = scene2::PolygonNode::alloc();
    //    floor->setPolygon(Rect(0, 0, safe.size.width, 10));
    //    floor->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    //    floor->setPosition(lOffset, bOffset);
    //    _scene->addChild(floor);

    // Making the ceiling -jdg274
    //    std::shared_ptr<scene2::PolygonNode> ceiling = scene2::PolygonNode::alloc();
    //    ceiling->setPolygon(Rect(0, 0, safe.size.width, 10));
    //    ceiling->setAnchor(Vec2::ANCHOR_TOP_LEFT);
    //    ceiling->setPosition(lOffset, size.height-tOffset);
    //    _scene->addChild(ceiling);

    // Making the left wall -jdg274
    //    std::shared_ptr<scene2::PolygonNode> left = scene2::PolygonNode::alloc();
    //    left->setPolygon(Rect(0,0,10, safe.size.height));
    //    left->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    //    left->setPosition(lOffset, bOffset);
    //    _scene->addChild(left);

    // Making the right wall -jdg274
    //    std::shared_ptr<scene2::PolygonNode> right = scene2::PolygonNode::alloc();
    //    right->setPolygon(Rect(0, 0, 10, safe.size.height+10));
    //    right->setAnchor(Vec2::ANCHOR_BOTTOM_RIGHT);
    //    right->setPosition(size.width-rOffset, bOffset);
    //    _scene->addChild(right);

    // Position the button in the bottom right corner
    button->setAnchor(Vec2::ANCHOR_CENTER);
    button->setPosition(size.width - (bsize.width + rOffset) / 2, (bsize.height + bOffset) / 2);

//    Vec2 enemyPos = ENEMY_POS;
//    std::shared_ptr<scene2::SceneNode> node = scene2::SceneNode::alloc();
//    std::shared_ptr<Texture> image = _assets->get<Texture>(ENEMY_TEXTURE);
//    _enemy = BaseEnemyModel::alloc(enemyPos, image->getSize() / scale, scale);
//    std::shared_ptr<scene2::PolygonNode> sprite = scene2::PolygonNode::allocWithTexture(image);
//    sprite->setScale(Vec2(scale / 2, scale / 2));
//    _enemy->setSceneNode(sprite);
//    _enemy->setDebugColor(Color4::RED);
//    _scene->addChild(_enemy->getSceneNode());

    // Add the logo and button to the scene graph
    _scene->addChild(_logo);
    _scene->addChild(button);

    // We can only activate a button AFTER it is added to a scene
    button->activate();

    // Start the logo countdown and C-style random number generator
    _countdown = TIME_STEP;
    std::srand((int)std::time(0));
}

/**
 * Adds the physics object to the physics world and loosely couples it to the scene graph
 *
 * There are two ways to link a physics object to a scene graph node on the
 * screen.  One way is to make a subclass of a physics object, like we did
 * with dude.  The other is to use callback functions to loosely couple
 * the two.  This function is an example of the latter.
 *
 * @param obj             The physics object to add
 * @param node            The scene graph node to attach it to
 * @param useObjPosition  Whether to update the node's position to be at the object's position
 */
void LiminalSpirit::addObstacle(const std::shared_ptr<cugl::physics2::Obstacle> &obj,
                                const std::shared_ptr<cugl::scene2::SceneNode> &node,
                                bool useObjPosition)
{
    _world->addObstacle(obj);

    // Position the scene graph node (enough for static objects)
    if (useObjPosition)
    {
        node->setPosition(obj->getPosition() * _scale);
    }
    _worldnode->addChild(node);

    // Dynamic objects need constant updating
    if (obj->getBodyType() == b2_dynamicBody)
    {
        scene2::SceneNode *weak = node.get(); // No need for smart pointer in callback
        obj->setListener([=](physics2::Obstacle *obs)
                         {
            weak->setPosition(obs->getPosition()*_scale);
            weak->setAngle(obs->getAngle()); });
    }
}
