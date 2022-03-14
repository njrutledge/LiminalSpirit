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
#include "LiminalSpiritApp.hpp"
#include <cugl/base/CUBase.h>
#include <box2d/b2_contact.h>
#include "BaseEnemyModel.h"
#include "Lost.hpp"
#include "Specter.hpp"
#include "AttackController.hpp"
#include "AIController.hpp"
#include "PlayerModel.h"
#include "CollisionController.hpp"

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
#define GRAVITY 30



/** The initial position of the enemies */
float ENEMY_POS[] = {20.0f, 5.0f};
float ENEMY_POS2[] = { 24.0f, 10.0f };

/** The initial position of the player*/
float PLAYER_POS[] = { 10.0f, 4.0f };


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
    Size dimen = getDisplaySize();
    float ratio1 = dimen.width/dimen.height;
    float ratio2 = ((float)SCENE_WIDTH)/((float)SCENE_HEIGHT);
    if (ratio1 < ratio2) {
        dimen *= SCENE_WIDTH/dimen.width;
    } else {
        dimen *= SCENE_HEIGHT/dimen.height;
    }
    CULog("Dimen: %f, %f", dimen.width, dimen.height);
    
    #if defined(CU_TOUCH_SCREEN)
    Input::activate<Touchscreen>();
    #else
        Input::activate<Mouse>();
    #endif

    // Create a scene graph the same size as the window
    _scene = Scene2::alloc(dimen.width, dimen.height);
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

    _tiltInput.init();
    // Activate mouse or touch screen input as appropriate
    // We have to do this BEFORE the scene, because the scene has a button

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
    _world = physics2::ObstacleWorld::alloc(Rect(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT), Vec2(0, -GRAVITY));
    _world->activateCollisionCallbacks(true);
        _world->onBeginContact = [this](b2Contact* contact) {
          _collider.beginContact(contact, _player);
        };
        _world->onEndContact = [this](b2Contact* contact) {
          _collider.endContact(contact, _player);
        };
    
    _scale = dimen.width == SCENE_WIDTH ? dimen.width / DEFAULT_WIDTH : dimen.height / DEFAULT_HEIGHT;
    Vec2 offset((dimen.width - SCENE_WIDTH) / 2.0f, (dimen.height - SCENE_HEIGHT) / 2.0f);
    CULog("Offset: %f,%f, Scale: %f, Width: %f, Height: %f", offset.x,offset.y,_scale,DEFAULT_WIDTH,DEFAULT_HEIGHT);
    
    // Create the scene graph
    _worldnode = scene2::SceneNode::alloc();
    _worldnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _worldnode->setPosition(offset);
    _scene->addChild(_worldnode);

    _debugnode = scene2::SceneNode::alloc();
    _debugnode->setScale(_scale); // Debug node draws in PHYSICS coordinates
    _debugnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _debugnode->setPosition(offset);
    _scene->addChild(_debugnode);

    _swipes.init(0, getDisplayWidth());
    

    _ai = AIController();

    _collider = CollisionController();

    setDebug(false);
    buildScene();
    _pMeleeTexture = _assets->get<Texture>(PATTACK_TEXTURE);
    _attacks.init(_pMeleeTexture->getSize()/_scale/2.0f, _scale / 2.0f, offset, _player);


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
    _debugnode = nullptr;

    //TODO: CHECK IF THIS IS RIGHT FOR DISPOSING
    for (auto it = _enemies.begin(); it != _enemies.end(); ++it) {
        (*it).~shared_ptr();
    }
    _enemies.clear();

    _ai.dispose();

    _tiltInput.dispose();
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

    // Update tilt input controller
    _tiltInput.update(timestep, _player->getX(), SCENE_WIDTH, _logo->getSize().width);
    float xPos = _tiltInput.getXpos();
    _player->setVX(xPos);

    //Debug Mode on/off
    if (_tiltInput.getDebugKeyPressed()) { setDebug(!isDebug()); }
    
    //FLIPPING LOGIC
    if(xPos > 0){
        _player->setFacingRight(true);
    } else if (xPos < 0){
        _player->setFacingRight(false);
    }
    scene2::TexturedNode* image = dynamic_cast<scene2::TexturedNode*>(_player->getSceneNode().get());
    if (image != nullptr) {
        image->flipHorizontal(_player->isFacingRight());
    }

    // Enemy AI logic
    // For each enemy
    for (auto it = _enemies.begin(); it != _enemies.end(); ++it) {
        Vec2 direction = _ai.getMovement(*it, _player->getPosition(), timestep);
        (*it)->setVX(direction.x);
        //TODO: Add vertical movement
    }
    
    _world->update(timestep);
    _swipes.update();
    _attacks.attackLeft(_swipes.getLeftSwipe(), _player->isGrounded());
    _attacks.attackRight(_swipes.getRightSwipe(),_player->isGrounded());
    for (auto it = _attacks._pending.begin(); it != _attacks._pending.end(); ++it) {
        //FIX WHEN TEXTURE EXISTS
        std::shared_ptr<scene2::PolygonNode> attackSprite = scene2::PolygonNode::allocWithTexture(_pMeleeTexture);
        attackSprite->setScale(.5f);

        addObstacle((*it), attackSprite, true);
    }
    _attacks.update(_player->getPosition(), _player->getBody()->GetLinearVelocity());
    if(_swipes.getLeftSwipe() == _swipes.up || _swipes.getRightSwipe() == _swipes.up){
        _player->setJumping(true);
    } else {
        _player->setJumping(false);
    }
    //_player->setGrounded(true);
    _player->applyForce();

    //Remove attacks
    auto ait = _attacks._current.begin();
    while(ait != _attacks._current.end()) {
        if ((*ait)->isRemoved()) {
            //int log1 = _world->getObstacles().size();
            cugl::physics2::Obstacle* obj = dynamic_cast<cugl::physics2::Obstacle*>(&**ait);
            _world->removeObstacle(obj);
            _worldnode->removeChild(obj->_node);

            //int log2 = _world->getObstacles().size();
            ait = _attacks._current.erase(ait);
        }
        else {
            ait++;
        }
    }

    //remove enemies
    auto eit = _enemies.begin();
    while (eit != _enemies.end()) {
        if ((*eit)->isRemoved()) {
            //int log1 = _world->getObstacles().size();
            cugl::physics2::Obstacle* obj = dynamic_cast<cugl::physics2::Obstacle*>(&**eit);
            _world->removeObstacle(obj);
            _worldnode->removeChild(obj->_node);

            //int log2 = _world->getObstacles().size();
            eit = _enemies.erase(eit);
        }
        else {
            eit++;
        }
    }
    //CULog("Attacks size: %d", _attacks._current.size());
    //CULog("World Size: %d", _world->getObstacles().size());
   
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
    
    _batch->begin(_scene->getCamera()->getCombined());
    _attacks.draw(_batch);
    _batch->end();
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
    
    // Initialize swipe controller
    // Must use safe bounds for swipe width
    Rect bounds = getSafeBounds();
    //_swiper.init(bounds.getMinX(), bounds.size.width);
    
    // Get the image and add it to the node.
    std::shared_ptr<Texture> texture  = _assets->get<Texture>("logo");
    _logo = scene2::PolygonNode::allocWithTexture(texture);
    _logo->setScale(0.2f); // Magic number to rescale asset
    // Put the logo in the middle of the screen
    _logo->setAnchor(Vec2::ANCHOR_CENTER);
    _logo->setPosition(size.width/2,size.height/2);

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

    // Making the floor -jdg274
    Rect floorRect = Rect(0, 0, 32, 0.5);
    std::shared_ptr<physics2::PolygonObstacle> floor = physics2::PolygonObstacle::allocWithAnchor(floorRect, Vec2::ANCHOR_CENTER);
    floor->setBodyType(b2_staticBody);
    std::shared_ptr<scene2::PolygonNode> floorNode = scene2::PolygonNode::allocWithPoly(floorRect*_scale);
    floorNode->setColor(Color4::BLACK);
    addObstacle(floor, floorNode, 1);

    // Making the ceiling -jdg274
    Rect ceilingRect = Rect(0, 17.5, 32, 0.5);
    std::shared_ptr<physics2::PolygonObstacle> ceiling = physics2::PolygonObstacle::allocWithAnchor(ceilingRect, Vec2::ANCHOR_CENTER);
    ceiling->setBodyType(b2_staticBody);
    std::shared_ptr<scene2::PolygonNode> ceilingNode = scene2::PolygonNode::allocWithPoly(ceilingRect*_scale);
    ceilingNode->setColor(Color4::BLACK);
    addObstacle(ceiling, ceilingNode, 1);

    // Making the left wall -jdg274
    Rect leftRect = Rect(0, 0, 0.5, 18);
    std::shared_ptr<physics2::PolygonObstacle> left = physics2::PolygonObstacle::allocWithAnchor(leftRect, Vec2::ANCHOR_CENTER);
    left->setBodyType(b2_staticBody);
    std::shared_ptr<scene2::PolygonNode> leftNode = scene2::PolygonNode::allocWithPoly(leftRect*_scale);
    leftNode->setColor(Color4::BLACK);
    addObstacle(left, leftNode, 1);

    // Making the right wall -jdg274
    Rect rightRect = Rect(31.5, 0, 0.5, 18);
    std::shared_ptr<physics2::PolygonObstacle> right = physics2::PolygonObstacle::allocWithAnchor(rightRect, Vec2::ANCHOR_CENTER);
    right->setBodyType(b2_staticBody);
    std::shared_ptr<scene2::PolygonNode> rightNode = scene2::PolygonNode::allocWithPoly(rightRect*_scale);
    rightNode->setColor(Color4::BLACK);
    addObstacle(right, rightNode, 1);

    // Position the button in the bottom right corner
    button->setAnchor(Vec2::ANCHOR_CENTER);
    button->setPosition(size.width - (bsize.width + rOffset) / 2, (bsize.height + bOffset) / 2);

    Vec2 enemyPos = ENEMY_POS;
    std::shared_ptr<scene2::SceneNode> enemyNode = scene2::SceneNode::alloc();
    std::shared_ptr<Texture> enemyImage = _assets->get<Texture>(ENEMY_TEXTURE);
    std::shared_ptr<Lost> enemy = Lost::alloc(enemyPos, enemyImage->getSize() / _scale / 5, _scale);
    std::shared_ptr<scene2::PolygonNode> enemySprite = scene2::PolygonNode::allocWithTexture(enemyImage);
    enemy->setSceneNode(enemySprite);
    enemy->setDebugColor(Color4::RED);
    enemySprite->setScale(0.2f);
    addObstacle(enemy, enemySprite, true);
    _enemies.insert(enemy);

    Vec2 enemyPos2 = ENEMY_POS2; 
    std::shared_ptr<scene2::SceneNode> specterNode = scene2::SceneNode::alloc();
    std::shared_ptr<Texture> specterImage = _assets->get<Texture>(ENEMY_TEXTURE);
    std::shared_ptr<Specter> specter = Specter::alloc(enemyPos2, specterImage->getSize() / _scale / 10, _scale);
    std::shared_ptr<scene2::PolygonNode> specterSprite = scene2::PolygonNode::allocWithTexture(specterImage);
    specter->setSceneNode(specterSprite);
    specter->setDebugColor(Color4::BLUE);
    specterSprite->setScale(0.2f);
    addObstacle(specter, specterSprite, true);
    _enemies.insert(specter);

    Vec2 playerPos = PLAYER_POS;
    std::shared_ptr<scene2::SceneNode> node = scene2::SceneNode::alloc();
    std::shared_ptr<Texture> image = _assets->get<Texture>(PLAYER_TEXTURE);
    _player = PlayerModel::alloc(playerPos, image->getSize() / _scale / 5, _scale);
    _player->setMovement(0);
    std::shared_ptr<scene2::PolygonNode> sprite = scene2::PolygonNode::allocWithTexture(image);
    _player->setSceneNode(sprite);
    _player->setDebugColor(Color4::RED);
    sprite->setScale(0.2f);
    addObstacle(_player, sprite, true);


    // Add the logo and button to the scene graph
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
    obj->setDebugScene(_debugnode);

    // Position the scene graph node (enough for static objects)
    if (useObjPosition)
    {
        node->setPosition(obj->getPosition() * _scale);
    }
    _worldnode->addChild(node);
    obj->setNode(node);

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

#pragma mark Collision Handling
