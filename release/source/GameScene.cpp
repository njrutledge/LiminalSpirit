//
//  GameScene.cpp
//  Cornell University Game Library (CUGL)
//
//  Root scene to manage the gameplay
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
#include "GameScene.hpp"
#include <cugl/base/CUBase.h>
#include <box2d/b2_contact.h>
#include "BaseEnemyModel.h"
#include "Lost.hpp"
#include "Specter.hpp"
#include "AttackController.hpp"
#include "AIController.hpp"
#include "PlayerModel.h"
#include "CollisionController.hpp"
#include "Platform.hpp"

// Add support for simple random number generation
#include <cstdlib>
#include <ctime>

// This keeps us from having to write cugl:: all the time
using namespace cugl;

// The number of frames before moving the logo to a new position
#define TIME_STEP 60
/** This is the size of the active portion of the screen */
#define SCENE_WIDTH 1024
// #define SCENE_HEIGHT 576
 #define SCENE_HEIGHT 768

/** Width of the game world in Box2d units */
#define DEFAULT_WIDTH 32.0f
/** Height of the game world in Box2d units */
float DEFAULT_HEIGHT = DEFAULT_WIDTH/SCENE_WIDTH*SCENE_HEIGHT;

/** The constant for gravity in the physics world. */
#define GRAVITY 30
#define PLATFORM_ATT 4
#define PLATFORM_COUNT 3
#define PLATFORMTEXTURE "platform"

/** The initial position of the enemies */
float ENEMY_POS[] = {18.0f, 15.0f};
float ENEMY_POS2[] = { 28.0f, 10.0f };

/** The initial position of the player*/
float PLAYER_POS[] = { 5.0f, 4.0f };

float PLATFORMS[PLATFORM_COUNT][PLATFORM_ATT] = {
    {15, 3, 10, 0.5},
    {5, 7, 8, 0.5},
    {7, 10, 9, 0.5}
};

/**
 * Initializes the controller contents, and starts the game
 *
 * The constructor does not allocate any objects or memory.  This allows
 * us to have a non-pointer reference to this controller, reducing our
 * memory allocation.  Instead, allocation happens in this method.
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool GameScene::init(const std::shared_ptr<cugl::AssetManager> &assets)
{

    Size dimen = Application::get()->getDisplaySize();
    float boundScale = SCENE_WIDTH / dimen.width;
    dimen *= boundScale;
    
    if (assets == nullptr)
    {
        return false;
    }
    else if (!Scene2::init(dimen))
    {
        return false;
    }
    
    // Start up the input handler
#if defined(CU_TOUCH_SCREEN)
    Input::activate<Touchscreen>();
#else
    Input::activate<Mouse>();
#endif

    // set assets
    _assets = assets;

    // Create a scene graph the same size as the window
    //_scene = Scene2::alloc(dimen.width, dimen.height);
    auto scene = _assets->get<scene2::SceneNode>("game");
    scene->setContentSize(dimen);
    scene->doLayout();

    // Application::get()->setClearColor(Color4(229, 229, 229, 255));

    // You have to attach the individual loaders for each asset type
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<Font>(FontLoader::alloc()->getHook());

    // This reads the given JSON file and uses it to load all other assets
    //_assets->loadDirectory("json/assets.json");
    // Activate mouse or touch screen input as appropriate
    // We have to do this BEFORE the scene, because the scene has a button

    // Enable physics -jdg274
    _world = physics2::ObstacleWorld::alloc(Rect(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT), Vec2(0, -GRAVITY));
    _world->activateCollisionCallbacks(true);
   //    _world->PreSolve = [this](b2Contact* contact, const b2Manifold *oldManifold) {
//        preSolve(contact, oldManifold);
//    };
    _world->onBeginContact = [this](b2Contact* contact) {
        _collider.beginContact(contact, _player, _attacks);
    };
    _world->onEndContact = [this](b2Contact* contact) {
        _collider.endContact(contact, _player);
    };
    
    // Only want to get swipes within safe bounds
    Rect bounds = Application::get()->getSafeBounds();
    CULog("Safe Area %sx%s", bounds.origin.toString().c_str(),
          bounds.size.toString().c_str());
    _input.init(bounds.getMinX(), bounds.size.width);
    
    bounds.origin *= boundScale;
    bounds.size *= boundScale;

    _scale = bounds.size.width / DEFAULT_WIDTH;
    Vec2 offset(bounds.getMinX(),0);
    CULog("Offset: %f,%f, Scale: %f, Width: %f, Height: %f", offset.x,offset.y,_scale,DEFAULT_WIDTH,DEFAULT_HEIGHT);
    
    // Create the scene graph
    // Bounds do not matter when constraint is false
    _worldnode = scene2::ScrollPane::allocWithBounds(bounds.size);
    _worldnode->setPosition(offset);
    _worldnode->setInterior(Rect(0,0,bounds.size.width,SCENE_HEIGHT));
    _worldnode->setConstrained(true);
    scene->addChild(_worldnode);
    
    // Bounds do not matter when constraint is false
    _debugnode = scene2::ScrollPane::allocWithBounds(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    _debugnode->setScale(_scale); // Debug node draws in PHYSICS coordinates
    _debugnode->setPosition(offset);
    scene->addChild(_debugnode);
    
    // TODO this init might be wrong, Nick had _scale/2.0f
    _pMeleeTexture = _assets->get<Texture>(PATTACK_TEXTURE);
    _attacks = std::make_shared<AttackController>();
    _attacks->init(_scale, 1.5, cugl::Vec2::UNIT_Y, cugl::Vec2(0,0.5), 0.5, 1, 0.5, 0.1);

    _ai = AIController();

    _collider = CollisionController();

    setDebug(false);
    buildScene(scene);
    addChild(scene);
    
    // Get font
    _font = assets->get<Font>("marker");
    
    // Create and layout the health meter
    std::string msg = strtool::format("Health %d", (int)_player->getHealth());
    _text = TextLayout::allocWithText(msg, assets->get<Font>("marker"));
    _text->layout();
    
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose()
{
    // Delete all smart pointers
    _logo = nullptr;
//    scene = nullptr;
    _batch = nullptr;
    _assets = nullptr;
    _constants = nullptr;
    _world = nullptr;
    _worldnode = nullptr;
    _debugnode = nullptr;
    
    //TODO: CHECK IF THIS IS RIGHT FOR DISPOSING
//    for (auto it = _enemies.begin(); it != _enemies.end(); ++it) {
//        (*it).~shared_ptr();
//    }
    // This should work because smart pointers free themselves when vector is cleared
    _enemies.clear();
    
    _player = nullptr;
    _attacks = nullptr;

    _ai.dispose();
}

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameScene::update(float timestep)
{

    // Update input controller
    _input.update();
    
    // Update tilt controller
    _tilt.update(_input, SCENE_WIDTH);
    float xPos = _tilt.getXpos();
    _player->setVX(xPos);

    // Debug Mode on/off
    if (_input.getDebugKeyPressed())
    {
        setDebug(!isDebug());
    }

    // FLIPPING LOGIC
    if (xPos > 0)
    {
        _player->setFacingRight(true);
    }
    else if (xPos < 0)
    {
        _player->setFacingRight(false);
    }
    scene2::TexturedNode *image = dynamic_cast<scene2::TexturedNode *>(_player->getSceneNode().get());
    if (image != nullptr)
    {
        image->flipHorizontal(_player->isFacingRight());
    }

    // Enemy AI logic
    // For each enemy
    for (auto it = _enemies.begin(); it != _enemies.end(); ++it) {
        Vec2 direction = _ai.getMovement(*it, _player->getPosition(), timestep);
        (*it)->setVX(direction.x);
        (*it)->setVY(direction.y);
        if ((*it)->isAttacking()) {
            //TODO: Need to variablize attack variables based on enemy type
            (*it)->setIsAttacking(false);
            Vec2 play_p = _player->getPosition();
            Vec2 en_p = (*it)->getPosition();
            Vec2 vel = Vec2(0.5, 0);
            if ((*it)->getName() == "Lost") {
                _attacks->createAttack(Vec2((*it)->getX(), (*it)->getY()) , 1.0f, 0.2f, 1.0f, AttackController::Type::e_melee, vel.rotate((play_p - en_p).getAngle()));
                
            }
            else if ((*it)->getName() == "Specter") {
                _attacks->createAttack(Vec2((*it)->getX(), (*it)->getY()) , 0.5f, 3.0f, 1.0f, AttackController::Type::e_range, (vel.scale(0.5)).rotate((play_p - en_p).getAngle()));
            }
        }
    }

    _swipes.update(_input);
    b2Vec2 playerPos = _player->getBody()->GetPosition();
    _attacks->attackLeft(Vec2(playerPos.x, playerPos.y), _swipes.getLeftSwipe(), _player->isGrounded());
    _attacks->attackRight(Vec2(playerPos.x, playerPos.y), _swipes.getRightSwipe(),_player->isGrounded());
    _world->update(timestep);
    
    for (auto it = _attacks->_pending.begin(); it != _attacks->_pending.end(); ++it) {
        //FIX WHEN TEXTURE EXISTS
        std::shared_ptr<scene2::PolygonNode> attackSprite = scene2::PolygonNode::allocWithTexture(_pMeleeTexture);
        attackSprite->setScale(.85f * (*it)->getRadius());
        (*it)->setDebugColor(Color4::YELLOW);
        addObstacle((*it), attackSprite, true);
    }
    //DO NOT MOVE THIS LINE
    _attacks->update(_player->getPosition(), _player->getBody()->GetLinearVelocity(), timestep);
    if(_swipes.getRightSwipe() == _swipes.upAttack){
        _player->setJumping(true);
    } else {
        _player->setJumping(false);
    }
    if (_player->getVY() > 0 && _player->getPosition().x > 1.0f && _player->getPosition().x < 31.0f && _player->getPosition().y < 17.0f) {
        _player->setSensor(true);
    } else {
        _player->setSensor(false);
    }
    
    _player->applyForce();

    
    //Remove attacks
    auto ait = _attacks->_current.begin();
    while(ait != _attacks->_current.end()) {
        if ((*ait)->isRemoved()) {
            //int log1 = _world->getObstacles().size();
            cugl::physics2::Obstacle* obj = dynamic_cast<cugl::physics2::Obstacle*>(&**ait);
            _world->removeObstacle(obj);
            _worldnode->removeChild(obj->_node);

            //int log2 = _world->getObstacles().size();
            ait = _attacks->_current.erase(ait);
        }
        else {
            ait++;
        }
    }

    //Remove enemies
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
    
    if (_player->isRemoved()) {
        reset();
        _player->markRemoved(false);
    }
    
    // Update the health meter
    _text->setText(strtool::format("Health %d", (int)_player->getHealth()));
    _text->layout();
    
    // Camera following player, with some non-linear smoothing
    float dy = getChild(0)->getContentSize().height / 2 - _worldnode->getPaneTransform().transform(_player->getSceneNode()->getPosition()).y;
    Vec2 pan = Vec2(0, dy);
    pan = pan * pan.length() / 3000;
    _worldnode->applyPan(pan);
    // Copy World's zoom and transform
    _debugnode->applyPan(-_debugnode->getPaneTransform().transform(Vec2()));
    _debugnode->applyPan(_worldnode->getPaneTransform().transform(Vec2()) / _scale);
}

/**
 * The method called to draw the gameplay scene
 */
void GameScene::render(const std::shared_ptr<cugl::SpriteBatch> &batch)
{
    // This takes care of begin/end

    //_scene->render(batch);
    Scene2::render(batch);
    batch->begin(getCamera()->getCombined());
    //_attacks.draw(batch);
    batch->drawText(_text,Vec2(20,getSize().height-_text->getBounds().size.height-10));
    batch->end();
}

void GameScene::createEnemies() {
    Vec2 enemyPos = ENEMY_POS;
    std::shared_ptr<scene2::SceneNode> enemyNode = scene2::SceneNode::alloc();
    std::shared_ptr<Texture> enemyImage = _assets->get<Texture>(ENEMY_TEXTURE);
    std::shared_ptr<Lost> enemy = Lost::alloc(enemyPos, enemyImage->getSize() / _scale / 10, _scale);
    std::shared_ptr<scene2::PolygonNode> enemySprite = scene2::PolygonNode::allocWithTexture(enemyImage);
    enemy->setSceneNode(enemySprite);
    enemy->setDebugColor(Color4::RED);
    enemySprite->setScale(0.15f);
    addObstacle(enemy, enemySprite, true);
    _enemies.push_back(enemy);

    Vec2 enemyPos2 = ENEMY_POS2;
    std::shared_ptr<scene2::SceneNode> specterNode = scene2::SceneNode::alloc();
    std::shared_ptr<Texture> specterImage = _assets->get<Texture>(ENEMY_TEXTURE2);
    std::shared_ptr<Specter> specter = Specter::alloc(enemyPos2, specterImage->getSize() / _scale / 15, _scale);
    std::shared_ptr<scene2::PolygonNode> specterSprite = scene2::PolygonNode::allocWithTexture(specterImage);
    specter->setSceneNode(specterSprite);
    specter->setDebugColor(Color4::BLUE);
    specterSprite->setScale(0.15f);
    addObstacle(specter, specterSprite, true);
    _enemies.push_back(specter);
}

/**
 * Internal helper to build the scene graph.
 *
 * Scene graphs are not required.  You could manage all scenes just like
 * you do in 3152.  However, they greatly simplify scene management, and
 * have become standard in most game engines.
 */
void GameScene::buildScene(std::shared_ptr<scene2::SceneNode> scene)
{
    Size size = Application::get()->getDisplaySize();
    float scale = SCENE_WIDTH / size.width;
    size *= scale;


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
            CULog("Reset");
            reset();
        } });

    // Find the safe area, adapting to the iPhone X
    Rect safe = Application::get()->getSafeBounds();
    safe.origin *= scale;
    safe.size *= scale;

    // Get the right and bottom offsets.
    float bOffset = safe.origin.y;
    float rOffset = (size.width) - (safe.origin.x + safe.size.width);

    // Making the floor -jdg274
    Rect floorRect = Rect(0, 0, DEFAULT_WIDTH, 0.5);
    std::shared_ptr<physics2::PolygonObstacle> floor = physics2::PolygonObstacle::allocWithAnchor(floorRect, Vec2::ANCHOR_CENTER);
    floor->setBodyType(b2_staticBody);
    
    std::shared_ptr<scene2::PolygonNode> floorNode = scene2::PolygonNode::allocWithPoly(floorRect*_scale);
    floorNode->setColor(Color4::BLACK);
    floor->setName("floor");
    addObstacle(floor, floorNode, 1);

    // Making the ceiling -jdg274
    Rect ceilingRect = Rect(0, DEFAULT_HEIGHT - 0.5, DEFAULT_WIDTH, 0.5);
    std::shared_ptr<physics2::PolygonObstacle> ceiling = physics2::PolygonObstacle::allocWithAnchor(ceilingRect, Vec2::ANCHOR_CENTER);
    ceiling->setBodyType(b2_staticBody);
    
    std::shared_ptr<scene2::PolygonNode> ceilingNode = scene2::PolygonNode::allocWithPoly(ceilingRect*_scale);
    ceilingNode->setColor(Color4::BLACK);
    addObstacle(ceiling, ceilingNode, 1);

    // Making the left wall -jdg274
    Rect leftRect = Rect(0, 0, 0.5, DEFAULT_HEIGHT);
    std::shared_ptr<physics2::PolygonObstacle> left = physics2::PolygonObstacle::allocWithAnchor(leftRect, Vec2::ANCHOR_CENTER);
    left->setBodyType(b2_staticBody);
    
    std::shared_ptr<scene2::PolygonNode> leftNode = scene2::PolygonNode::allocWithPoly(leftRect*_scale);
    leftNode->setName("leftwall");
    leftNode->setColor(Color4::BLACK);
    addObstacle(left, leftNode, 1);

    // Making the right wall -jdg274
    Rect rightRect = Rect(DEFAULT_WIDTH-0.5, 0, 0.5, DEFAULT_HEIGHT);
    std::shared_ptr<physics2::PolygonObstacle> right = physics2::PolygonObstacle::allocWithAnchor(rightRect, Vec2::ANCHOR_CENTER);
    right->setBodyType(b2_staticBody);
    std::shared_ptr<scene2::PolygonNode> rightNode = scene2::PolygonNode::allocWithPoly(rightRect*_scale);
    rightNode->setName("rightwall");
    rightNode->setColor(Color4::BLACK);
    addObstacle(right, rightNode, 1);

    // Position the button in the bottom right corner
    button->setAnchor(Vec2::ANCHOR_CENTER);
    button->setPosition(size.width - (bsize.width + rOffset) / 2, (bsize.height + bOffset) / 2);

    createEnemies();

    Vec2 playerPos = PLAYER_POS;
    std::shared_ptr<scene2::SceneNode> node = scene2::SceneNode::alloc();
    std::shared_ptr<Texture> image = _assets->get<Texture>(PLAYER_TEXTURE);
    _player = PlayerModel::alloc(playerPos, image->getSize() / _scale / 8, _scale);
    _player->setMovement(0);
    std::shared_ptr<scene2::PolygonNode> sprite = scene2::PolygonNode::allocWithTexture(image);
    _player->setSceneNode(sprite);
    _player->setDebugColor(Color4::RED);
    sprite->setScale(0.175f);
    addObstacle(_player, sprite, true);

    Vec2 platformPos = Vec2(5.0f, 5.0f);
    Rect platRect = Rect(5.0f, 5.0f, 10, 1);
    std::shared_ptr<scene2::SceneNode> platformNode = scene2::SceneNode::alloc();
    _platforms.push_back(PlatformModel::alloc(platformPos, 10, 1, _scale));
    std::shared_ptr<scene2::PolygonNode> spritePlatform = scene2::PolygonNode::allocWithPoly(platRect * _scale);
    _platformNodes.push_back(spritePlatform);

    platformPos = Vec2(30.0f, 5.0f);
    platRect = Rect(30.0f, 5.0f, 3, 1);
    platformNode = scene2::SceneNode::alloc();
    _platforms.push_back(PlatformModel::alloc(platformPos, 3, 1, _scale));
    spritePlatform = scene2::PolygonNode::allocWithPoly(platRect * _scale);
    _platformNodes.push_back(spritePlatform);

    platformPos = Vec2(10.0f, 10.0f);
    platRect = Rect(10.0f, 10.0f, 5, 1);
    platformNode = scene2::SceneNode::alloc();
    _platforms.push_back(PlatformModel::alloc(platformPos, 5, 1, _scale));
    spritePlatform = scene2::PolygonNode::allocWithPoly(platRect * _scale);
    _platformNodes.push_back(spritePlatform);

    platformPos = Vec2(20.0f, 12.0f);
    platRect = Rect(20.0f, 12.0f, 8, 1);
    platformNode = scene2::SceneNode::alloc();
    _platforms.push_back(PlatformModel::alloc(platformPos, 8, 1, _scale));
    spritePlatform = scene2::PolygonNode::allocWithPoly(platRect * _scale);
    _platformNodes.push_back(spritePlatform);

    // Add platforms to the world
    for (int i = 0; i < _platforms.size(); i++) {
        _platforms[i]->setName("platform");
        _platforms[i]->setSceneNode(_platformNodes[i]);
        _platforms[i]->setDebugColor(Color4::RED);
        _platformNodes[i]->setColor(Color4::BLACK);
        addObstacle(_platforms[i], _platformNodes[i], true);
    }

    // Add the logo and button to the scene graph
    // TODO get rid of this
    scene->addChild(button);

    // We can only activate a button AFTER it is added to a scene
    button->activate();

    // Start the logo countdown and C-style random number generator
    _countdown = TIME_STEP;
    std::srand((int)std::time(0));
}

/**
 * Resets the status of the game so that we can play again.
 */
void GameScene::reset()
{
    _input.reset();
    _swipes.reset();
    _tilt.reset();
    auto ac_it = _attacks->_current.begin();
    while (ac_it != _attacks->_current.end()) {
        //int log1 = _world->getObstacles().size();
        cugl::physics2::Obstacle* obj = dynamic_cast<cugl::physics2::Obstacle*>(&**ac_it);
        _world->removeObstacle(obj);
        _worldnode->removeChild(obj->_node);

        //int log2 = _world->getObstacles().size();
        ac_it = _attacks->_current.erase(ac_it);
    }
    auto ap_it = _attacks->_current.begin();
    while (ap_it != _attacks->_current.end()) {
        //int log1 = _world->getObstacles().size();
        cugl::physics2::Obstacle* obj = dynamic_cast<cugl::physics2::Obstacle*>(&**ap_it);
        _world->removeObstacle(obj);
        _worldnode->removeChild(obj->_node);

        //int log2 = _world->getObstacles().size();
        ac_it = _attacks->_current.erase(ap_it);
    }
    //_attacks->reset();  Shouldn't be needed now
    // Does nothing right now
    _ai.reset();
    
    // Reset player position & health & other member fields
    Vec2 playerPos = PLAYER_POS;
    _player->reset(playerPos);
    
    // Remove all enemies
    auto eit = _enemies.begin();
    while (eit != _enemies.end()) {
        cugl::physics2::Obstacle* obj = dynamic_cast<cugl::physics2::Obstacle*>(&**eit);
        _world->removeObstacle(obj);
        _worldnode->removeChild(obj->_node);

        eit = _enemies.erase(eit);
    }
    
    // Make all enemies
    createEnemies();
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
void GameScene::addObstacle(const std::shared_ptr<cugl::physics2::Obstacle> &obj,
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
    if (obj->getBodyType() == b2_dynamicBody) {
        scene2::SceneNode* weak = node.get(); // No need for smart pointer in callback
        obj->setListener([=](physics2::Obstacle* obs){
            weak->setPosition(obs->getPosition()*_scale);
            weak->setAngle(obs->getAngle());
        });
    }
}
