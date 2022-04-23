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
#include "Phantom.hpp"
#include "Mirror.hpp"

#include "PlayerModel.h"
#include "Platform.hpp"

#include "AttackController.hpp"
#include "AIController.hpp"
#include "CollisionController.hpp"

#include "Glow.hpp"


#include "RRParticle.h"
#include "RRParticleNode.h"
#include "RRParticlePool.h"

// Add support for simple random number generation
#include <cstdlib>
#include <ctime>

// This keeps us from having to write cugl:: all the time
using namespace cugl;

//set to true to disable most of the update loop, for checking sprites
#define DEBUG false

// The number of frames before moving the logo to a new position
#define TIME_STEP 60
/** This is the size of the active portion of the screen */
#define SCENE_WIDTH 1024
// #define SCENE_HEIGHT 576
#define SCENE_HEIGHT 1728

/** Width of the game world in Box2d units */
#define DEFAULT_WIDTH 32.0f
/** Height of the game world in Box2d units */
float DEFAULT_HEIGHT = DEFAULT_WIDTH / SCENE_WIDTH * SCENE_HEIGHT;

/** The constant for gravity in the physics world. */
#define GRAVITY 30
#define PLATFORM_ATT 3
#define PLATFORM_COUNT 4
#define PLATFORM_HEIGHT 0.5
#define PLATFORMTEXTURE "platform"


/** The initial position of the player*/
float PLAYER_POS[] = { 5.0f, 4.0f };

string BIOME = "cave";
float LEVEL_HEIGHT = 54;


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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager>& assets, const std::shared_ptr<SoundController> sound, string json)
{
    _back = false;
    _step = false;
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

    // Get constant values from assets/level.json
    _constants = assets->get<JsonValue>(json);
    BIOME = _constants->getString("biome");
    LEVEL_HEIGHT = _constants->getFloat("level_height");
    PLAYER_POS[0] = _constants->get("start_pos")->get(0)->asFloat();
    PLAYER_POS[1] = _constants->get("start_pos")->get(1)->asFloat();
    auto platformsAttr = _constants->get("platforms")->children();
    _platforms_attr.clear();
    for (auto it = platformsAttr.begin(); it != platformsAttr.end(); ++it) {
        std::shared_ptr<JsonValue> entry = (*it);
        float* attr = new float[3];
        attr[0] = entry->get(0)->asFloat();
        attr[1] = entry->get(1)->asFloat();
        attr[2] = entry->get(2)->asFloat();

        _platforms_attr.push_back(attr);
    }

    // Sound controller
    _sound = sound;

    //Get Particle Info
    _particleInfo = assets->get<JsonValue>("particles");


    auto spawn = _constants->get("spawn_order")->children();
    auto spawnPos = _constants->get("spawn_pos");
    auto spawnTime = _constants->get("spawn_times");
    int index = 0;
    Vec2 pos;
    _spawn_order.clear();
    _spawn_pos.clear();
    _spawn_times.clear();
    for (auto it = spawn.begin(); it != spawn.end(); ++it) {
        std::shared_ptr<JsonValue> entry = (*it);
        std::vector<string> enemies;
        std::vector<Vec2> enemies_pos;
        for (int i = 0; i < entry->size(); i++) {
            enemies.push_back(entry->get(i)->asString());
            pos.x = spawnPos->get(index)->get(i)->get(0)->asFloat();
            pos.y = spawnPos->get(index)->get(i)->get(1)->asFloat();
            enemies_pos.push_back(pos);
        }
        _spawn_order.push_back(enemies);
        _spawn_pos.push_back(enemies_pos);
        _spawn_times.push_back(spawnTime->get(index)->asFloat());
        index++;
    }
    _numWaves = index;
    // Set enemy wave number
    _nextWaveNum = 0;

    //    auto spawnTypes = _constants->get("spawner_types")->children();
    //    for(auto it = spawnTypes.begin(); it != spawnTypes.end(); ++it) {
    //        std::shared_ptr<JsonValue> entry = (*it);
    //        std::unordered_map<string, spawnerEnemy> enemy_types;
    //
    //        for(int i = 0; i < entry->size(); i++) {
    //            string enemy = entry->get(i)->asString();
    //            if(!enemy_types[enemy].max_count) {
    //                enemy_types[enemy].max_count = 1;
    //                enemy_types[enemy].current_count = 0;
    //                enemy_types[enemy].timer = 10.0f;
    //            } else {
    //                enemy_types[enemy].max_count++;
    //            }
    //        }
    //        _spawner_enemy_types.push_back(enemy_types);
    //        _living_spawners.push_back(0);
    //    }
    _spawner_ind = -1;
    _spawnerCount = 0;

    // Create a scene graph the same size as the window
    //_scene = Scene2::alloc(dimen.width, dimen.height);
    auto scene = _assets->get<scene2::SceneNode>("game");
    scene->setContentSize(dimen);
    scene->doLayout();

    // Application::get()->setClearColor(Color4(229, 229, 229, 255));

    // You have to attach the individual loaders for each asset type

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
    _world->onBeginContact = [this](b2Contact* contact)
    {
        _collider.beginContact(contact, _attacks, _timer);
    };
    _world->onEndContact = [this](b2Contact* contact)
    {
        _collider.endContact(contact);
    };

    // Only want to get swipes within safe bounds
    Rect bounds = Application::get()->getSafeBounds();
    _input.init(bounds.getMinX(), bounds.size.width);

    bounds.origin *= boundScale;
    bounds.size *= boundScale;

    _scale = bounds.size.width / DEFAULT_WIDTH;
    Vec2 offset(bounds.getMinX(), 0);

    // Create the scene graph
    // Bounds do not matter when constraint is false
    _worldnode = scene2::ScrollPane::allocWithBounds(bounds.size);
    _worldnode->setPosition(offset);
    _worldnode->setInterior(Rect(0, 0, bounds.size.width, SCENE_HEIGHT));
    _worldnode->setConstrained(true);
    scene->addChild(_worldnode);

    _worldnode2 = scene2::OrderedNode::allocWithOrder(scene2::OrderedNode::Order::ASCEND, bounds.size);
    _worldnode2->setPosition(offset);
    _worldnode->addChild(_worldnode2);

    // Bounds do not matter when constraint is false
    _debugnode = scene2::ScrollPane::allocWithBounds(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    _debugnode->setScale(_scale); // Debug node draws in PHYSICS coordinates
    _debugnode->setPosition(offset);
    scene->addChild(_debugnode);

    // TODO this init might be wrong, Nick had _scale/2.0f
    _pMeleeTexture = _assets->get<Texture>(PATTACK_TEXTURE);
    _attacks = std::make_shared<AttackController>();
    _attacks->init(_scale, _scale * 1.5, 1.5, cugl::Vec2(0, 1.25), cugl::Vec2(0, 0.5), 0.5, 1, 0.35, 0.1, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    _dashTime = 0;
    _dashXVel = 0;
    _dashYVel = 0;

    _ai = AIController();

    _collider = CollisionController();
    _collider.init(sound);

    setDebug(false);
    buildScene(scene);
    addChild(scene);

    // Get font
    _font = assets->get<Font>("marker");

    // Create and layout the health meter
    //std::string msg = strtool::format("Health %d", (int) _player->getHealth());
    //_text = TextLayout::allocWithText(msg, assets->get<Font>("marker"));
    //_text->layout();
    _healthback = scene2::PolygonNode::allocWithPoly((Rect(0, 0, _player->getHealth() / 4.0f, .5) * _scale));
    _healthback->setPriority(9);
    _healthback->setColor(PLAYER_HEALTHBACK_COLOR);
    _healthback->setAnchor(0, 1);
    _healthback->setPosition(Vec2(10, dimen.height - 10));
    scene->addChild(_healthback);
    _health = scene2::PolygonNode::allocWithPoly((Rect(0, 0, _player->getHealth() / 4.0f, .5) * _scale));
    _health->setPriority(10);
    _health->setColor(PLAYER_HEALTH_COLOR);
    _health->setAnchor(0, 1);
    _health->setPosition(Vec2(10, dimen.height - 10));
    scene->addChild(_health);



    _timer = 0.0f;

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
    if (_worldnode)_worldnode->removeAllChildren();
    _worldnode = nullptr;
    if (_worldnode2)_worldnode2->removeAllChildren();
    _worldnode2 = nullptr;
    if (_debugnode) _debugnode->removeAllChildren();
    _debugnode = nullptr;
    _vertbuff = nullptr;
    _sound = nullptr;
    _text = nullptr;
    _font = nullptr;
    _endText = nullptr;

    //TODO: CHECK IF THIS IS RIGHT FOR DISPOSING
//    for (auto it = _enemies.begin(); it != _enemies.end(); ++it) {
//        (*it).~shared_ptr();
//    }
    // This should work because smart pointers free themselves when vector is cleared
    _enemies.clear();
    _player = nullptr;
    _attacks = nullptr;

    _ai.dispose();
    removeAllChildren();
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
    _sound->play_level_music("cave");

    // Update input controller
    _input.update();
    // Debug Mode on/off
    if (_input.getDebugKeyPressed())
    {
        setDebug(!isDebug());
    }

    if (DEBUG) {
        //go ahead and step the time to spawn some enemies, they won't move in debug
        _timer += timestep;

        if (_nextWaveNum < _numWaves && _timer >= _spawn_times[_nextWaveNum]) {
            createEnemies(_nextWaveNum);
            _nextWaveNum += 1;
        }
        //return to stop the rest of the update in DEBUG ONLY
        return;
    }

    // Some Particle Stuff
    // ********* this is how you'd make a new node of particles ***********/
    //if (_meleeArm->getLastType() == AttackController::MeleeState::h1_left) { ///////////////////////////////////////A way to trigger
    //    std::shared_ptr<ParticlePool> pool = ParticlePool::allocPoint(_particleInfo->get("collision"), Vec2(0,0));//Create the pool for the effect based on defined values in json
    //    std::shared_ptr<Texture> text = _assets->get<Texture>(PLAYER_RANGE);////////////////////////////////////////Create a texture or vector of textures to use for the particle effect

    //    std::shared_ptr<ParticleNode> newParts = ParticleNode::alloc(Vec2(10*_scale,10*_scale), text, pool);///////Create a node 
    //    newParts->setVisible(true);////////////////////////////////////////////////////////////////////////////////Make visible (not usually necessary)
    //    _worldnode->addChildWithTag(newParts, 100);///////////////////////////////////////////////////////////////Add to worldnode with 100 tag (necessary)
    //}
     
    ////Update all Particles
    for (std::shared_ptr<scene2::SceneNode> s : _worldnode->getChildren()) {
        if (s->getTag() == 100) {
            if (!s->isVisible()) {
                s->dispose();
            }
            ParticleNode* pn = dynamic_cast<ParticleNode*>(s.get());
            pn->update(timestep);
        }
    };

    // Update tilt controller
    _tilt.update(_input, SCENE_WIDTH);
    float xPos = _tilt.getXpos();
    if (_player->isStunned()) {
        _player->setVX(0);
    }
    else {
        _player->setVX(xPos);
    }

    ///////////////////////////////////////
    // Start Player and Arm Animations ////
    ///////////////////////////////////////
    int nextFrame;
    scene2::SpriteNode* sprite = dynamic_cast<scene2::SpriteNode*>(_player->getSceneNode().get());

    sprite->setAnchor(0.5, 0.3);
    // Player (body) Animations
    if (_player->isStunned()) {

        // Play damaged particles once when stunned
        //if ((sprite->getFrame() != 31 && _player->isFacingRight()) || (sprite->getFrame() != 24 && !_player->isFacingRight())) {
        //    std::shared_ptr<ParticlePool> pool = ParticlePool::allocPoint(_particleInfo->get("damaged"), Vec2(0, 0));
        //    std::shared_ptr<Texture> text = _assets->get<Texture>("star");
        //    std::shared_ptr<ParticleNode> dmgd = ParticleNode::alloc(_player->getPosition() * _scale, text, pool);
        //    dmgd->setColor(Color4::RED);
        //    dmgd->setScale(0.25f);
        //    _worldnode->addChildWithTag(dmgd, 100);
        //}

        // Store the frame being played before stun
        if (sprite->getFrame() != 31 && sprite->getFrame() != 24) {
            _prevFrame = sprite->getFrame();
        }
        if (_player->isFacingRight()) {
            sprite->setFrame(31);
        }
        else {
            sprite->setFrame(24);
        }
    }
    else if (!_player->isGrounded()) {
        if (_player->getJumpAnimationTimer() > 0.03f) {
            if(_player->isMovingUp()) {
                nextFrame = sprite->getFrame();
                if (nextFrame == 31 || nextFrame == 24) {
                    nextFrame = _prevFrame;
                }
                if (_player->isFacingRight()) {
                    if (nextFrame < 20 || nextFrame > 23) {
                        nextFrame = 21;
                    }
                    else if (nextFrame > 20) {
                        nextFrame -= 1;
                    }
                } else {
                    if (nextFrame < 16 || nextFrame > 19) {
                        nextFrame = 18;
                    }
                    else if (nextFrame < 19) {
                        nextFrame += 1;
                    }
                }
            }
            else {
                if (_player->isFacingRight()) {
                    nextFrame = 19;
                    _player->setJustLanded(true);
                }
                else {
                    nextFrame = 20;
                    _player->setJustLanded(true);
                }
            }
            sprite->setFrame(nextFrame);
            _player->setJumpAnimationTimer(0);
        }
        _prevFrame = sprite->getFrame();
    }
    else if (_player->isGrounded() && _player->hasJustLanded()) {
        if (_player->getJumpAnimationTimer() > 0.06f) {
            nextFrame = sprite->getFrame();
            if (nextFrame == 31 || nextFrame == 24) {
                nextFrame = _prevFrame;
            }
            if (_player->isFacingRight()) {
                if (nextFrame > 18) {
                    nextFrame = 18;
                } else {
                    nextFrame -= 1;
                }
                if (nextFrame == 16) {
                    _player->setJustLanded(false);
                }
            }
            else {
                if (nextFrame < 21) {
                    nextFrame = 21;
                } else {
                    nextFrame += 1;
                }
                if (nextFrame == 23) {
                    _player->setJustLanded(false);
                }
            }
            sprite->setFrame(nextFrame);
            _player->setJumpAnimationTimer(0);
        }
        _prevFrame = sprite->getFrame();
    }
    else if (xPos != 0 && _player->getWalkAnimationTimer() > 0.09f) {
        if (!_player->isFacingRight()) {
            nextFrame = (sprite->getFrame() + 1) % 8;
            sprite->setFrame(nextFrame);
        }
        else {
            if (sprite->getFrame() > 7 || sprite->getFrame() == 0) {
                sprite->setFrame(7);
            }
            else {
                sprite->setFrame(sprite->getFrame() - 1);
            }
        }
        _player->setWalkAnimationTimer(0);
        _prevFrame = sprite->getFrame();
    }
    else if (xPos == 0 && ((_player->getIdleAnimationTimer() > 1.f) || !(sprite->getFrame() == 13 || sprite->getFrame() == 8 || sprite->getFrame() == 10 || sprite->getFrame() == 15) && _player->getIdleAnimationTimer() < 0.2f)) {
        if (sprite->getFrame() < 8) {
            if (_player->isFacingRight()) {
                nextFrame = 12;
            }
            else {
                nextFrame = 8;
            }
        }
        else {
            // cause flipHorizontal flips the whole spritesheet >.>
            if (_player->isFacingRight()) {
                nextFrame = ((sprite->getFrame() + 1) % 4) + 12;
            }
            else {
                nextFrame = ((sprite->getFrame() + 1) % 4) + 8;
            }
        }
        sprite->setFrame(nextFrame);
        _player->setIdleAnimationTimer(0);
        _prevFrame = sprite->getFrame();
    }
    else {
        if (_player->isFacingRight()) {
            sprite->setFrame(_prevFrame);
        }
        else {
            sprite->setFrame(_prevFrame);
        }
    }
    
    _player->setJumpAnimationTimer(_player->getJumpAnimationTimer() + timestep);
    _player->setWalkAnimationTimer(_player->getWalkAnimationTimer() + timestep);
    _player->setIdleAnimationTimer(_player->getIdleAnimationTimer() + timestep);
    _rangedArm->setGlowTimer(_rangedArm->getGlowTimer() + timestep);
    _meleeArm->setGlowTimer(_meleeArm->getGlowTimer() + timestep);

    if (sprite->getFrame() == 0 || sprite->getFrame() == 4) {
        if (!_step) {
            _sound->play_player_sound(SoundController::playerSType::step);
        }
        
    } else {
        _step = false;
    }
    
    
    // Arm and Player Flipping
    scene2::TexturedNode *image = dynamic_cast<scene2::TexturedNode *>(_player->getSceneNode().get());
    scene2::TexturedNode* arm1Image = dynamic_cast<scene2::TexturedNode*>(_rangedArm->getSceneNode().get());
    scene2::TexturedNode* arm2Image = dynamic_cast<scene2::TexturedNode*>(_meleeArm->getSceneNode().get());

    if (image != nullptr)
    {
        image->flipHorizontal(_player->isFacingRight());
    }
    if (arm1Image != nullptr) {
        arm1Image->flipHorizontal(_player->isFacingRight());
    }
    if (arm2Image != nullptr) {
        arm2Image->flipHorizontal(_player->isFacingRight());
        if (_meleeArm->getLastType() == AttackController::MeleeState::h1_left || _meleeArm->getLastType() == AttackController::MeleeState::h2_left || _meleeArm->getLastType() == AttackController::MeleeState::h3_left) {
            arm2Image->flipHorizontal(false);
        }
        else if (_meleeArm->getLastType() == AttackController::MeleeState::h1_right || _meleeArm->getLastType() == AttackController::MeleeState::h2_right || _meleeArm->getLastType() == AttackController::MeleeState::h3_right) {
            arm2Image->flipHorizontal(true);
        }
    }

    scene2::SpriteNode* mSprite = dynamic_cast<scene2::SpriteNode*>(_meleeArm->getSceneNode().get());
    scene2::SpriteNode* rSprite = dynamic_cast<scene2::SpriteNode*>(_rangedArm->getSceneNode().get());
    _meleeArm->setAnimeTimer(_meleeArm->getAnimeTimer() + timestep);
    _rangedArm->setAnimeTimer(_rangedArm->getAnimeTimer() + timestep);

    // Ranged Arm
    if (_player->isStunned()) {
        rSprite->setFrame(8);
    }
    else if (_rangedArm->getLastType() == AttackController::MeleeState::cool) {
        if (_player->isFacingRight()) {
            rSprite->setFrame(5);
        }
        else {
            rSprite->setFrame(0);
        }
    }
    else if (_rangedArm->getLastType() == AttackController::MeleeState::first) {
        if (_rangedArm->getAnimeTimer() > 0.06f) {
            if ((rSprite->getFrame() == 5 && !_player->isFacingRight()) ||
                (rSprite->getFrame() == 0 && _player->isFacingRight())) {
                // Attack is finished
                if (_player->isFacingRight()) {
                    rSprite->setFrame(5);
                    rSprite->setAnchor(0.5,0.5);
                    _rangedArm->setAttackAngle(0);
                }
                else {
                    rSprite->setFrame(0);
                    rSprite->setAnchor(0.5,0.5);
                    _rangedArm->setAttackAngle(0);
                }
                _rangedArm->setLastType(AttackController::MeleeState::cool);
                _rangedArm->setAnimeTimer(0);
                arm1Image->flipHorizontal(_player->isFacingRight());
            }
            else {
                if (_player->isFacingRight()) {
                    rSprite->setAnchor(0.8,0.8);
                    if (rSprite->getFrame() == 0) {
                        rSprite->setFrame(5);
                    }
                    else {
                        rSprite->setFrame(rSprite->getFrame() - 1);
                    }
                }
                else {
                    rSprite->setAnchor(0.2,0.8);
                    rSprite->setFrame((rSprite->getFrame() + 1) % 6);
                }
                _rangedArm->setAnimeTimer(0);
            }
        }
    }
    else {
        if (_player->isFacingRight()) {
            rSprite->setFrame(5);
        }
        else {
            rSprite->setFrame(0);
        }
    }

    // Melee Arm
    if (_player->isStunned()) {
        mSprite->setFrame(22);
    }
    else if (_meleeArm->getLastType() == AttackController::MeleeState::cool) {
        if (_player->isFacingRight()) {
            mSprite->setFrame(7);
        }
        else {
            mSprite->setFrame(13);
        }
    }
    else if (_meleeArm->getLastType() == AttackController::MeleeState::h1_left) {
        if (_meleeArm->getAnimeTimer() > 0.05f) {
            if (mSprite->getFrame() == 12) {
                // Attack is finished
                _meleeArm->setLastType(AttackController::MeleeState::cool);
                _meleeArm->setAnimeTimer(0);
            }
            else {
                if (mSprite->getFrame() == 13) {
                    mSprite->setFrame(7);
                }
                else {
                    mSprite->setFrame(((mSprite->getFrame() + 1) % 7) + 7);
                }
                _meleeArm->setAnimeTimer(0);
            }
        }
    }
    else if (_meleeArm->getLastType() == AttackController::MeleeState::h2_left) {
        if (_meleeArm->getAnimeTimer() > 0.06f) {
            if (mSprite->getFrame() == 4) {
                // Attack is finished
                _meleeArm->setLastType(AttackController::MeleeState::cool);
                _meleeArm->setAnimeTimer(0);
            }
            else {
                if (mSprite->getFrame() > 6) {
                    mSprite->setFrame(0);
                }
                else {
                    mSprite->setFrame((mSprite->getFrame() + 1) % 5);
                }
                _meleeArm->setAnimeTimer(0);
            }
        }
    }
    else if (_meleeArm->getLastType() == AttackController::MeleeState::h3_left) {
        if (_meleeArm->getAnimeTimer() > 0.05f) {
            if (mSprite->getFrame() == 20) {
                // Attack is finished
                _meleeArm->setLastType(AttackController::MeleeState::cool);
                _meleeArm->setAnimeTimer(0);
            }
            else {
                if (mSprite->getFrame() < 14) {
                    mSprite->setFrame(14);
                }
                else {
                    mSprite->setFrame(((mSprite->getFrame() + 1) % 7) + 14);
                }
                _meleeArm->setAnimeTimer(0);
            }
        }
    }
    else if (_meleeArm->getLastType() == AttackController::MeleeState::h1_right) {
        if (_meleeArm->getAnimeTimer() > 0.05f) {
            if (mSprite->getFrame() == 8) {
                // Attack is finished
                _meleeArm->setLastType(AttackController::MeleeState::cool);
                _meleeArm->setAnimeTimer(0);
            }
            else {
                if (mSprite->getFrame() <= 7) {
                    mSprite->setFrame(13);
                }
                mSprite->setFrame(mSprite->getFrame() - 1);
                _meleeArm->setAnimeTimer(0);
            }
        }
    }
    else if (_meleeArm->getLastType() == AttackController::MeleeState::h2_right) {
        if (_meleeArm->getAnimeTimer() > 0.06f) {
            if (mSprite->getFrame() == 2) {
                // Attack is finished
                _meleeArm->setLastType(AttackController::MeleeState::cool);
                mSprite->setFrame(7);
                _meleeArm->setAnimeTimer(0);
            }
            else {
                if (mSprite->getFrame() >= 7) {
                    mSprite->setFrame(6);
                }
                mSprite->setFrame(mSprite->getFrame() - 1);
                _meleeArm->setAnimeTimer(0);
            }
        }
    }
    else if (_meleeArm->getLastType() == AttackController::MeleeState::h3_right) {
        if (_meleeArm->getAnimeTimer() > 0.05f) {
            if (mSprite->getFrame() == 14) {
                // Attack is finished
                _meleeArm->setLastType(AttackController::MeleeState::cool);
                _meleeArm->setAnimeTimer(0);
            }
            else {
                if (mSprite->getFrame() < 13) {
                    mSprite->setFrame(20);
                }
                mSprite->setFrame(mSprite->getFrame() - 1);
                _meleeArm->setAnimeTimer(0);
            }
        }
    }
    else {
        if (_player->isFacingRight()) {
            mSprite->setFrame(7);
        }
        else {
            mSprite->setFrame(13);
        }
    }
    

    float offsetArm = -2.7f;
    if (!_player->isFacingRight()) {
        offsetArm = -1 * offsetArm;
    }

    if ((!_player->isFacingRight() && rSprite->getFrame() != 0 && _rangedArm->getAttackAngle() > 90 && _rangedArm->getAttackAngle() < 270) ||
        (_player->isFacingRight() && rSprite->getFrame() != 5 && (_rangedArm->getAttackAngle() > 90 && _rangedArm->getAttackAngle() < 270))) {
        offsetArm = -1 * offsetArm;
    }


    float upDown = _rangedArm->getGlowTimer();
    float spacing = 1.f;
    float upDownY1 = fmod(upDown / 2, spacing);

    if (_player->isFacingRight() && rSprite->getFrame() != 5) {
        if (_rangedArm->getAttackAngle() > 90 && _rangedArm->getAttackAngle() < 270) {
            _rangedArm->setPosition(_player->getPosition().x + offsetArm - 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
        }
        else {
            _rangedArm->setPosition(_player->getPosition().x + offsetArm + 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
        }
    }
    else if (!_player->isFacingRight() && rSprite->getFrame() != 0) {
        if (_rangedArm->getAttackAngle() > 90 && _rangedArm->getAttackAngle() < 270) {
            _rangedArm->setPosition(_player->getPosition().x + offsetArm + 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
        }
        else {
            _rangedArm->setPosition(_player->getPosition().x + offsetArm - 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
        }
    }
    else {
        _rangedArm->setPosition(_player->getPosition().x + offsetArm, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
    }

    ////////////////////////////////////////
    ///////End Player and Arm Animations////
    ////////////////////////////////////////
    
    // Enemy AI logic
    // For each enemy
    std::shared_ptr<ParticlePool> pool = ParticlePool::allocPoint(_particleInfo->get("damaged"), Vec2(0, 0));
    std::shared_ptr<Texture> text = _assets->get<Texture>("star");

    for (auto it = _enemies.begin(); it != _enemies.end(); ++it)
    {
        Vec2 direction = _ai.getMovement(*it, _player->getPosition(), timestep);
        (*it)->setVX(direction.x);
        (*it)->setVY(direction.y);
        (*it)->getGlow()->setPosition((*it)->getPosition());
        (*it)->setInvincibilityTimer((*it)->getInvincibilityTimer() - timestep);

        (*it)->setIdleAnimationTimer((*it)->getIdleAnimationTimer() + timestep);

        scene2::SpriteNode* sprite = dynamic_cast<scene2::SpriteNode*>((*it)->getSceneNode().get());

        // For running idle animations specific (for speed) to enemies
        if ((*it)->getName() == "Phantom") {
            if ((*it)->getIdleAnimationTimer() > 0.1f) {
                sprite->setFrame((sprite->getFrame() + 1) % 7);
                (*it)->setIdleAnimationTimer(0);
            }
        }
        else if ((*it)->getName() == "Glutton") {
            if ((*it)->getInvincibilityTimer() > 0) {
                if (sprite->getFrame() != 7) {
                    std::shared_ptr<ParticleNode> dmgd = ParticleNode::alloc((*it)->getPosition() * _scale, text, pool);
                    dmgd->setScale(0.25f);
                    _worldnode->addChildWithTag(dmgd, 100);
                }
                sprite->setFrame(7);
            } else if ((*it)->getIdleAnimationTimer() > 1.f ||
                (!(sprite->getFrame() == 2) && (*it)->getIdleAnimationTimer() > 0.3f)
                || (!(sprite->getFrame() == 2 || sprite->getFrame() == 5 || sprite->getFrame() == 6) && (*it)->getIdleAnimationTimer() > 0.1f)) {
                sprite->setFrame((sprite->getFrame() + 1) % 7);
                (*it)->setIdleAnimationTimer(0);
            }
        }
        else if ((*it)->getName() == "Lost") {
            if ((*it)->getVX() > 0) {
                sprite->flipHorizontal(false);
            }
            else {
                sprite->flipHorizontal(true);
            }
            if ((*it)->getInvincibilityTimer() > 0) {
                if (sprite->getFrame() != 7 && sprite->getFrame() != 4) {
                    std::shared_ptr<ParticleNode> dmgd = ParticleNode::alloc((*it)->getPosition() * _scale, text, pool);
                    dmgd->setScale(0.25f);
                    _worldnode->addChildWithTag(dmgd, 100);
                }
                if ((*it)->getVX() > 0) {
                    sprite->setFrame(4);
                }
                else {
                    sprite->setFrame(7);
                }
            } // Using idle timer for walking animation since lost has no idle
            else if ((*it)->getIdleAnimationTimer() > .1f && (*it)->getVX() > 0) {
                sprite->setFrame((sprite->getFrame() + 1) % 4);
                (*it)->setIdleAnimationTimer(0);
            }
            else if ((*it)->getIdleAnimationTimer() > .1f && (*it)->getVX() < 0) {
                sprite->setFrame((sprite->getFrame() - 1) % 4);
                (*it)->setIdleAnimationTimer(0);
            }
        }


        if ((*it)->getInvincibilityTimer() <= 0) {
            (*it)->setInvincibility(false);
        }
        if ((*it)->isAttacking()) {
            Vec2 play_p = _player->getPosition();
            Vec2 en_p = (*it)->getPosition();
            Vec2 vel = Vec2(0.5, 0);
            //TODO: Need to variablize attack variables based on enemy type
            if ((*it)->getName() != "Seeker") {
                (*it)->setIsAttacking(false);
            }
            else {
                shared_ptr<Seeker> seeker = dynamic_pointer_cast<Seeker>(*it);
                
                _attacks->createAttack(Vec2((*it)->getX(), (*it)->getY()) , 1.0f, 0.2f, 2.0f, AttackController::Type::e_melee, (vel.scale(0.2)).rotate((play_p - en_p).getAngle()), _timer, SEEKER_ATTACK, 0);
                
            }

            if ((*it)->getName() == "Lost") {
                _attacks->createAttack(Vec2((*it)->getX(), (*it)->getY()) , 1.0f, 0.2f, 1.0f, AttackController::Type::e_melee, vel.rotate((play_p - en_p).getAngle()), _timer, LOST_ATTACK, 0);
                
            }
            else if ((*it)->getName() == "Phantom")
            {
                _attacks->createAttack(Vec2((*it)->getX(), (*it)->getY()), 0.5f, 3.0f, 1.0f, AttackController::Type::e_range, (vel.scale(0.5)).rotate((play_p - en_p).getAngle()), _timer, PHANTOM_ATTACK, PHANTOM_FRAMES);
            }
            else if ((*it)->getName() == "Glutton") {
                _attacks->createAttack(Vec2((*it)->getX(), (*it)->getY()) , 1.5f, 3.0f, 2.0f, AttackController::Type::e_range, (vel.scale(0.25)).rotate((play_p - en_p).getAngle()), _timer, GLUTTON_ATTACK, GLUTTON_FRAMES);
            }

        }
        if (Mirror* mirror = dynamic_cast<Mirror*>((*it).get())) {
            if (mirror->getLinkedEnemy() == nullptr) {
                mirror->setLinkedEnemy(getNearestNonMirror(mirror->getPosition()));
                if (mirror->getLinkedEnemy() == nullptr) {
                    mirror->markRemoved(true);
                }
            }
        }
    }

    // update spawners
    if (_spawnerCount) {
        if (_collider.getSpawnerKilled() != -1) {
            _living_spawners[_collider.getSpawnerKilled()] = 0;
            _spawnerCount--;
            _collider.setSpawnerKilled(-1);
        }
        if (_collider.getIndexSpawner() != -1) {
            int i = _collider.getIndexSpawner();
            string name = _collider.getSpawnerEnemyName();
            _spawner_enemy_types[i][name].current_count = _spawner_enemy_types[i][name].current_count - 1;
            _collider.setIndexSpawner(-1);
        }
    }
    
    // if player is stunned, do not read swipe input
    if(!_player->isStunned()){
        _swipes.update(_input, _player->isGrounded(), timestep);
    }
    
    b2Vec2 playerPos = _player->getBody()->GetPosition();
    if (_player->getInvincibilityTimer() <= 0) {
        _attacks->attackLeft(Vec2(playerPos.x, playerPos.y), _swipes.getLeftSwipe(), _swipes.getLeftAngle(), _player->isGrounded(), _timer, _sound);
        _attacks->attackRight(Vec2(playerPos.x, playerPos.y), _swipes.getRightSwipe(), _swipes.getRightAngle(), _player->isGrounded(), _timer, _sound);
        if (_swipes.getRightSwipe() == SwipeController::chargedRight) {
            _dashXVel = 20;
            _dashYVel = 1;
            _dashTime = 0;
        }
        else if (_swipes.getRightSwipe() == SwipeController::chargedLeft) {
            _dashXVel = -20;
            _dashYVel = 1;
            _dashTime = 0;
        }
        else if (_swipes.getRightSwipe() == SwipeController::chargedUp) {
            _dashYVel = 20;
            _dashTime = 0;
        }
        else if (_swipes.getRightSwipe() == SwipeController::chargedDown) {
            _dashYVel = -23;
            _dashTime = 0;
        }
        // If the dash velocities are set, change player velocity if dash time is not complete
        if (_dashXVel || _dashYVel) {
            if (_dashTime < 0.6f) {
                // Slow down for last 0.25 seconds at the end of right/left dash
                // This might be jank but its 1am
                float slowDownTime = 0.6f - 0.25f;
                if (_dashTime > slowDownTime && _dashXVel > 0) {
                    _dashXVel = 20 - (_dashTime - slowDownTime) * 80;
                }
                else if (_dashTime > slowDownTime && _dashXVel < 0) {
                    _dashXVel = -20 + (_dashTime - slowDownTime) * 80;
                }
                // Set velocity for right/left dash
                if (_dashXVel > 0) {
                    _player->setVX(_dashXVel);
                    _player->setFacingRight(true);
                }
                else if (_dashXVel < 0) {
                    _player->setVX(_dashXVel);
                    _player->setFacingRight(false);
                }
                // Always want to set x velocity to 0 for up/down charge attacks
                // Up down dash is only 0.5 seconds
                if (_dashTime < 0.5f) {
                    if (_dashYVel > 0) {
                        _player->setVY(_dashYVel);
                        _player->setVX(_dashXVel);
                    }
                    else if (_dashYVel < 0 && !_player->isGrounded()) {
                        _player->setVY(_dashYVel);
                        _player->setVX(_dashXVel);
                    }
                }
                // Invincibility, maintain same health throughout dash
                _player->setIsInvincible(true);
                _dashTime += timestep;
            }
            else {
                _dashXVel = 0;
                _dashYVel = 0;
            }
        }
        else {
            // Flipping logic based on tilt
            if (xPos > 0)
            {
                _player->setFacingRight(true);
            }
            else if (xPos < 0)
            {
                _player->setFacingRight(false);
            }
        }
        if (_dashXVel == 0 && _dashYVel == 0 && _player->getInvincibilityTimer() <= 0) {
            _player->setIsInvincible(false);
            _player->setIsStunned(false);
        }
    }

    if (_dashTime > 0 && _dashTime < 0.6f) {
        _player->setDropTime(timestep);
    }

    _player->setInvincibilityTimer(_player->getInvincibilityTimer() - timestep);
    _world->update(timestep);



    for (auto it = _attacks->_pending.begin(); it != _attacks->_pending.end(); ++it)
    {

        AttackController::Type attackType = (*it)->getType();
        std::shared_ptr<scene2::PolygonNode> attackSprite;
        if (attackType == AttackController::Type::p_range) {
            std::shared_ptr<Texture> attackTexture = _assets->get<Texture>(PLAYER_RANGE);
            attackSprite = scene2::SpriteNode::alloc(attackTexture, 1, 1);//this is wrong for sprite sheet
            attackSprite->setScale(.85f * (*it)->getRadius());
            attackSprite->setAngle((*it)->getAngle() * M_PI / 180);
            attackSprite->setPriority(3);
            _rangedArm->setLastType(AttackController::MeleeState::first);
            if (_swipes.getLeftSwipe() == SwipeController::downAttack) {
                _rangedArm->setAttackAngle(270);
            }
            else {
                _rangedArm->setAttackAngle((*it)->getAngle());
            }
            if (_player->isFacingRight()) {
                _rangedArm->setAttackAngle(fmod(_rangedArm->getAttackAngle() + 180, 360));
            }
        }
        else if (attackType == AttackController::Type::p_exp) {
            std::shared_ptr<Texture> attackTexture = _assets->get<Texture>("player_projectile");//this string is wrong
            attackSprite = scene2::SpriteNode::alloc(attackTexture, 1, 1);//this is wrong for sprite sheet
            attackSprite->setScale(.85f * (*it)->getRadius());
            attackSprite->setAngle((*it)->getAngle() * M_PI / 180);
            attackSprite->setColor(Color4::RED);
            attackSprite->setPriority(3);
            _rangedArm->setLastType(AttackController::MeleeState::first);
            if (_swipes.getLeftSwipe() == SwipeController::downAttack) {
                _rangedArm->setAttackAngle(270);
            }
            else {
                _rangedArm->setAttackAngle((*it)->getAngle());
            }
            if (_player->isFacingRight()) {
                _rangedArm->setAttackAngle(fmod(_rangedArm->getAttackAngle() + 180, 360));
            }
        }
        else if (attackType == AttackController::Type::p_exp_package) {
            std::shared_ptr<Texture> attackTexture = _assets->get<Texture>("player_projectile");//this string is wrong
            attackSprite = scene2::SpriteNode::alloc(attackTexture, 1, 1);//this is wrong for sprite sheet
            attackSprite->setScale(.85f * (*it)->getRadius());
            attackSprite->setAngle((*it)->getAngle() * M_PI / 180);
            attackSprite->setColor(Color4::RED);
            attackSprite->setPriority(3);
            _rangedArm->setLastType(AttackController::MeleeState::first);
            if (_swipes.getLeftSwipe() == SwipeController::downAttack) {
                _rangedArm->setAttackAngle(270);
            }
            else {
                _rangedArm->setAttackAngle((*it)->getAngle());
            }
            if (_player->isFacingRight()) {
                _rangedArm->setAttackAngle(fmod(_rangedArm->getAttackAngle() + 180, 360));
            }
        }
        else if (attackType == AttackController::Type::p_melee) {
            AttackController::MeleeState meleeState = (*it)->getMeleeState();

            attackSprite = scene2::PolygonNode::allocWithTexture(_pMeleeTexture);
            attackSprite->setVisible(false);
            attackSprite->setPriority(3);
            _meleeArm->setLastType(meleeState);
        }
        else if (attackType == AttackController::Type::e_range) {
            std::shared_ptr<Texture> attackTexture = _assets->get<Texture>((*it)->getAttackID());
            attackSprite = scene2::SpriteNode::alloc(attackTexture, 1, (*it)->getFrames()); // need to replace with animated texture
            if ((*it)->getAttackID() == PLAYER_RANGE) {
                //this is mirrors
                attackSprite->setScale(.85f * (*it)->getRadius());
                attackSprite->setAngle((*it)->getAngle());
                attackSprite->setColor(Color4::BLUE);
                attackSprite->setPriority(2.1);

            }
            else if ((*it)->getAttackID() == PHANTOM_ATTACK) {
                attackSprite->setScale(0.04*(*it)->getRadius());
                attackSprite->setAngle((*it)->getAngle()+M_PI/2);
                attackSprite->setPriority(2.2);


                dynamic_pointer_cast<scene2::SpriteNode>(attackSprite)->setFrame(0);
            }
            else if((*it)->getAttackID() == GLUTTON_ATTACK) {
                attackSprite->setScale(.25 * (*it)->getRadius());
                attackSprite->setAngle((*it)->getAngle() + M_PI);
                attackSprite->setPriority(2);
            }
        }
        else {
            attackSprite = scene2::PolygonNode::allocWithTexture(_pMeleeTexture);
            attackSprite->setVisible(false);
            attackSprite->setScale(.85f * (*it)->getRadius());
        }

        (*it)->setDebugColor(Color4::YELLOW);
        addObstacle((*it), attackSprite, true);
    }
    _attacks->update(_player->getPosition(), _player->getBody()->GetLinearVelocity(), timestep);
    // DO NOT MOVE THE ABOVE LINE
    if (_swipes.getRightSwipe() == SwipeController::upAttack || _swipes.getLeftSwipe() == SwipeController::jump || _swipes.getRightSwipe() == SwipeController::jump)
    {
        _player->setJumping(true);
        _player->setIsFirstFrame(true);
        if (_player->isGrounded()) {
            _player->setMovingUp(true);
            _player->setJumpAnimationTimer(0);
        }
    }
    else if (_swipes.getRightSwipe() == _swipes.downAttack) {
        //IDK
        _player->setDropTime(0.4f);
    }
    else
    {
        _player->setJumping(false);
    }
    _player->applyForce();

    if (_player->getVY() < -.2 || _player->getVY() > .2)
    {
        _player->setGrounded(false);
    }
    else if (_player->getVY() >= -0.2 && _player->getVY() <= 0.2)
    {
        // check if this is the first "0" velocity frame, as this should not make the player grounded just yet. Might be height of jump.
        if (_player->isFirstFrame())
        {
            _player->setIsFirstFrame(false);
        }
        else
        {
            _player->setGrounded(true);
        }
    }

    if (_player->getVY() < 0) {
        _player->setMovingUp(false);
    }

    // Remove attacks
    auto ait = _attacks->_current.begin();
    while (ait != _attacks->_current.end())
    {
        if ((*ait)->isRemoved())
        {
            // int log1 = _world->getObstacles().size();
            cugl::physics2::Obstacle* obj = dynamic_cast<cugl::physics2::Obstacle*>(&**ait);
            _world->removeObstacle(obj);
            _worldnode2->removeChild(obj->_node);

            // int log2 = _world->getObstacles().size();
            ait = _attacks->_current.erase(ait);
        }
        else
        {
            ait++;
        }
    }

    // Remove enemies
    auto eit = _enemies.begin();
    while (eit != _enemies.end()) {
        if ((*eit)->isRemoved()) {
            //int log1 = _world->getObstacles().size();
            cugl::physics2::Obstacle* glowObj = dynamic_cast<cugl::physics2::Obstacle*>(&*(*eit)->getGlow());
            cugl::physics2::Obstacle* obj = dynamic_cast<cugl::physics2::Obstacle*>(&**eit);
            _world->removeObstacle(glowObj);
            _worldnode2->removeChild(glowObj->_node);
            _world->removeObstacle(obj);
            _worldnode2->removeChild(obj->_node);

            // int log2 = _world->getObstacles().size();
            eit = _enemies.erase(eit);
        }
        else
        {
            eit++;
        }
    }

    // Move wave spawn times up if all enemies killed
    if (_nextWaveNum < _numWaves && !_enemies.size()) {
        float nextSpawnTime = _spawn_times[_nextWaveNum];
        float diff = nextSpawnTime - _timer;
        for (int i = _nextWaveNum; i < _numWaves; i++) {
            _spawn_times[i] -= diff;
        }
    }

    if (_player->isRemoved()) {
        reset();
        _player->markRemoved(false);
    }

    // Update the health meter
    //_text->setText(strtool::format("Health %d", (int)_player->getHealth()));
    //_text->layout();
    _health->setPolygon((Rect(0, 0, _player->getHealth()/ 4.0f, .5) * _scale));
    _health->setPriority(10);
   
    // Camera following player, with some non-linear smoothing
    float dy = getChild(0)->getContentSize().height / 2 - _worldnode->getPaneTransform().transform(_player->getSceneNode()->getPosition()).y;
    Vec2 pan = Vec2(0, dy);
    pan = pan * pan.length() / 3000;
    _worldnode->applyPan(pan);
    // Copy World's zoom and transform
    _debugnode->applyPan(-_debugnode->getPaneTransform().transform(Vec2()));
    _debugnode->applyPan(_worldnode->getPaneTransform().transform(Vec2()) / _scale);

    // Spawn new enemies if time for next wave
    _timer += timestep;

    if (_nextWaveNum < _numWaves && _timer >= _spawn_times[_nextWaveNum]) {
        createEnemies(_nextWaveNum);
        _nextWaveNum += 1;
    }

    int index = 0;
    for (auto it = _spawner_enemy_types.begin(); it != _spawner_enemy_types.end(); ++it) {
        if (_living_spawners[index]) {
            for (auto i : (*it)) {
                auto p = i.second;
                int diff_count = p.max_count - p.current_count;
                string name = i.first;
                if (p.timer <= 0) {

                    while (diff_count != 0) {
                        createSpawnerEnemy(index, name);
                        _spawner_enemy_types[index][name].current_count++;
                        diff_count--;
                    }
                    _spawner_enemy_types[index][name].timer = 10.0f;
                }
                else if (diff_count > 0) {
                    _spawner_enemy_types[index][name].timer -= timestep;

                }
            }
        }
        index++;
    }

    // All waves created and all enemies cleared
    if (_nextWaveNum >= _numWaves && !_enemies.size() && !_spawnerCount) {
        // Create and layout win text
        std::string msg = strtool::format("YOU WIN!");
        _endText = TextLayout::allocWithText(msg, _font);
        _endText->setVerticalAlignment(VerticalAlign::MIDDLE);
        _endText->setHorizontalAlignment(HorizontalAlign::CENTER);
        _endText->layout();
    }

    _playerGlow->setPosition(_player->getPosition());
    ////MELEE ARM MUST STAY AT BOTTOM
    // Determining arm positions and offsets
    float offsetArm2 = -3.1f;
    if (!_player->isFacingRight()) {
        offsetArm = -1 * offsetArm;
    }

    // change based on arm attacks
    if ((!_player->isFacingRight() || // player facing left or attacks left
        (_meleeArm->getLastType() == AttackController::MeleeState::h1_left
            || _meleeArm->getLastType() == AttackController::MeleeState::h2_left
            || _meleeArm->getLastType() == AttackController::MeleeState::h3_left)))// player facing left and attacking right
    {
        offsetArm2 = -1 * offsetArm2;
    }

    if (!_player->isFacingRight() && (_meleeArm->getLastType() == AttackController::MeleeState::h1_right
        || _meleeArm->getLastType() == AttackController::MeleeState::h2_right
        || _meleeArm->getLastType() == AttackController::MeleeState::h3_right)) {
        offsetArm2 = -1 * offsetArm2;
    }

    float upDown2 = _meleeArm->getGlowTimer() + 0.5f;
    float upDownY2 = fmod(upDown2 / 2, spacing);
    if (upDownY1 > spacing / 4 && upDownY1 <= 3 * spacing / 4) {
        upDownY1 = spacing / 2 - upDownY1;
    }
    else if (upDownY1 > 3 * spacing / 4) {
        upDownY1 = -1 * spacing + upDownY1;
    }
    if (upDownY2 > spacing / 4 && upDownY2 <= 3 * spacing / 4) {
        upDownY2 = spacing / 2 - upDownY2;
    }
    else if (upDownY2 > 3 * spacing / 4) {
        upDownY2 = -1 * spacing + upDownY2;
    }
    _meleeArm->setPosition(_player->getPosition().x - offsetArm2, _player->getPosition().y + (upDownY2 / spacing / 3) + 0.1f);

}

std::shared_ptr<BaseEnemyModel> GameScene::getNearestNonMirror(cugl::Vec2 pos) {
    float distance(INT_MAX);
    std::shared_ptr<BaseEnemyModel> savedEnemy = nullptr;
    for (auto it = _enemies.begin(); it != _enemies.end(); ++it) {
        if (Mirror* mirror = dynamic_cast<Mirror*>((*it).get())) {
            //Do nothing, but need to see if it can be casted
        }
        else {
            if (pos.distance((*it)->getPosition()) <= distance) {
                distance = pos.distance((*it)->getPosition());
                savedEnemy = (*it);
            }
        }
    }
    return savedEnemy;
}

/**
 * The method called to draw the gameplay scene
 */
void GameScene::render(const std::shared_ptr<cugl::SpriteBatch>& batch)
{
    // This takes care of begin/end

    //_scene->render(batch);
    if (_player->isInvincible() && !_player->isStunned()) {
        // TODO Change this
        _player->getSceneNode()->setColor(Color4::GREEN);
    }
    else if (_swipes.hasLeftChargedAttack() && _swipes.hasRightChargedAttack()) {
        _player->getSceneNode()->setColor(Color4(125, 0, 255, 255));
    }
    else if (_swipes.hasLeftChargedAttack()) {
        _player->getSceneNode()->setColor(Color4::RED);
    }
    else if (_swipes.hasRightChargedAttack()) {
        _player->getSceneNode()->setColor(Color4::BLUE);
    }
    else {
        _player->getSceneNode()->setColor(Color4::WHITE);
    }
    // Make enemies flash red when invincible
    for (auto it = _enemies.begin(); it != _enemies.end(); ++it) {
        if ((*it)->getInvincibility()) {
            (*it)->getSceneNode()->setColor(Color4::RED);
        }
        else {
            (*it)->getSceneNode()->setColor(Color4::WHITE);
        }
    }
    Scene2::render(batch);
    batch->begin(getCamera()->getCombined());

    //_attacks.draw(batch);
   // batch->drawText(_text, Vec2(20, getSize().height - _text->getBounds().size.height - 10));

    batch->setColor(Color4::GREEN);
    Affine2 trans;
    trans.scale(3);
    trans.translate(Vec2(getSize().width / 2, getSize().height / 2));

    if (_endText && !_endText->getText().compare("YOU WIN!")) {
        batch->setColor(Color4::GREEN);
        Affine2 trans;
        trans.scale(3);
        trans.translate(Vec2(getSize().width / 2, getSize().height / 2));
        batch->drawText(_endText, trans);
    }

    batch->end();
}

void GameScene::createMirror(Vec2 enemyPos, Mirror::Type type, std::string assetName, std::shared_ptr<Glow> enemyGlow) {
    std::shared_ptr<Texture> mirrorImage = _assets->get<Texture>(assetName);
    std::shared_ptr<Texture> mirror_reflectattackImage = _assets->get<Texture>(MIRROR_REFLECT_TEXTURE);
    //shards
    std::shared_ptr<scene2::PolygonNode> mirrorShards[6];
    mirrorShards[0] = scene2::PolygonNode::allocWithTexture(_assets->get<Texture>(MIRROR_SHARD_TEXTURE_1));
    mirrorShards[1] = scene2::PolygonNode::allocWithTexture(_assets->get<Texture>(MIRROR_SHARD_TEXTURE_2));
    mirrorShards[2] = scene2::PolygonNode::allocWithTexture(_assets->get<Texture>(MIRROR_SHARD_TEXTURE_3));
    mirrorShards[3] = scene2::PolygonNode::allocWithTexture(_assets->get<Texture>(MIRROR_SHARD_TEXTURE_4));
    mirrorShards[4] = scene2::PolygonNode::allocWithTexture(_assets->get<Texture>(MIRROR_SHARD_TEXTURE_5));
    mirrorShards[5] = scene2::PolygonNode::allocWithTexture(_assets->get<Texture>(MIRROR_SHARD_TEXTURE_6));


    std::shared_ptr<Mirror> mirror = Mirror::alloc(enemyPos, mirrorImage->getSize(), mirrorImage->getSize() / _scale / 15, _scale, type); // TODO this is not right, fix this to be closest enemy
    std::shared_ptr<scene2::PolygonNode> mirrorSprite = scene2::PolygonNode::allocWithTexture(mirrorImage);
    mirror->setGlow(enemyGlow);
    mirror->setAttackAnimationTimer(0);
    std::shared_ptr<scene2::SpriteNode> attackSprite = scene2::SpriteNode::alloc(mirror_reflectattackImage, MIRROR_REFLECT_ROWS, MIRROR_REFLECT_COLS);
    attackSprite->setFrame(0);
    attackSprite->setScale(1.15f);
    mirror->setAttackSprite(attackSprite);
    mirror->showAttack(false);


    std::shared_ptr<scene2::PolygonNode> mirrorShard1 = mirrorShards[1];
    mirrorShards[rand() % 6]->copy(mirrorShard1);
    std::shared_ptr<scene2::PolygonNode> mirrorShard2 = mirrorShards[2];
    mirrorShards[rand() % 6]->copy(mirrorShard2);
    std::shared_ptr<scene2::PolygonNode> mirrorShard3 = mirrorShards[3];
    mirrorShards[rand() % 6]->copy(mirrorShard3);

    mirror->setThreeShards(mirrorShard1, mirrorShard2, mirrorShard3);
    mirror->setSceneNode(mirrorSprite);
    //mirrorSprite->addChildWithName(mirrorShard1, "shard1");
    //mirrorSprite->addChildWithName(mirrorShard2, "shard2");
    //mirrorSprite->addChildWithName(mirrorShard3, "shard3");
    mirror->setDebugColor(Color4::BLUE);
    mirrorSprite->setScale(0.15f);
    mirrorSprite->setPriority(1.4);
    addObstacle(mirror, mirrorSprite, true);
    _enemies.push_back(mirror);

}

void GameScene::createSpawnerEnemy(int spawnerInd, string enemyName) {
    Vec2 enemyPos;

    enemyPos = _spawner_pos[spawnerInd];

    std::shared_ptr<Texture> enemyGlowImage = _assets->get<Texture>(GLOW_TEXTURE);
    std::shared_ptr<Glow> enemyGlow = Glow::alloc(enemyPos, enemyGlowImage->getSize() / _scale, _scale);
    std::shared_ptr<scene2::PolygonNode> enemyGlowSprite = scene2::PolygonNode::allocWithTexture(enemyGlowImage);
    enemyGlow->setSceneNode(enemyGlowSprite);
    std::shared_ptr<Gradient> grad = Gradient::allocRadial(Color4(255, 255, 255, 85), Color4(111, 111, 111, 0), Vec2(0.5, 0.5), .2f);
    enemyGlowSprite->setGradient(grad);
    enemyGlowSprite->setRelativeColor(false);
    enemyGlowSprite->setScale(.65f);
    addObstacle(enemyGlow, enemyGlowSprite, true);
    // lowercase the enemyName
    std::transform(enemyName.begin(), enemyName.end(), enemyName.begin(),
        [](unsigned char c) { return std::tolower(c); });
    if (!enemyName.compare("lost")) {
        std::shared_ptr<Texture> lostImage = _assets->get<Texture>("lost");
        std::shared_ptr<Lost> lost = Lost::alloc(enemyPos, lostImage->getSize(), lostImage->getSize() / _scale / 10, _scale);
        std::shared_ptr<scene2::PolygonNode> lostSprite = scene2::PolygonNode::allocWithTexture(lostImage);
        lost->setGlow(enemyGlow);
        lost->setSceneNode(lostSprite);
        lost->setDebugColor(Color4::RED);
        lostSprite->setScale(0.15f);
        addObstacle(lost, lostSprite, true);
        _enemies.push_back(lost);
    }
    else if (!enemyName.compare("phantom")) {
        std::shared_ptr<Texture> phantomHitboxImage = _assets->get<Texture>("phantom");
        std::shared_ptr<Texture> phantomImage = _assets->get<Texture>("phantom_ani");
        std::shared_ptr<Phantom> phantom = Phantom::alloc(enemyPos, Vec2(phantomImage->getSize().width / 7, phantomImage->getSize().height), phantomHitboxImage->getSize() / _scale / 10, _scale);
        std::shared_ptr<scene2::SpriteNode> phantomSprite = scene2::SpriteNode::alloc(phantomImage, 1, 7);
        phantom->setSceneNode(phantomSprite);
        phantom->setDebugColor(Color4::BLUE);
        phantom->setGlow(enemyGlow);
        phantomSprite->setScale(0.05f);
        phantomSprite->setFrame(0);
        addObstacle(phantom, phantomSprite, true);
        _enemies.push_back(phantom);
    }
    else if (!enemyName.compare("square")) {
        createMirror(enemyPos, Mirror::Type::square, "squaremirror", enemyGlow);
    }
    else if (!enemyName.compare("triangle")) {
        createMirror(enemyPos, Mirror::Type::triangle, "trianglemirror", enemyGlow);
    }
    else if (!enemyName.compare("circle")) {
        createMirror(enemyPos, Mirror::Type::circle, "circlemirror", enemyGlow);
    }
    else if (!enemyName.compare("seeker")) {
        std::shared_ptr<Texture> seekerImage = _assets->get<Texture>("seeker");
        std::shared_ptr<Seeker> seeker = Seeker::alloc(enemyPos, seekerImage->getSize(), seekerImage->getSize() / _scale / 10, _scale);
        std::shared_ptr<scene2::PolygonNode> seekerSprite = scene2::PolygonNode::allocWithTexture(seekerImage);
        seeker->setSceneNode(seekerSprite);
        seeker->setDebugColor(Color4::GREEN);
        seeker->setGlow(enemyGlow);
        seekerSprite->setScale(0.15f);
        addObstacle(seeker, seekerSprite, true);
        _enemies.push_back(seeker);
    }
    else if (!enemyName.compare("glutton")) {
        std::shared_ptr<Texture> gluttonHitboxImage = _assets->get<Texture>("glutton");
        std::shared_ptr<Texture> gluttonImage = _assets->get<Texture>("glutton_ani");
        std::shared_ptr<Glutton> glutton = Glutton::alloc(enemyPos + Vec2(0, 2), Vec2(gluttonImage->getSize().width / 7, gluttonImage->getSize().height), gluttonHitboxImage->getSize() / _scale / 5, _scale);
        std::shared_ptr<scene2::SpriteNode> gluttonSprite = scene2::SpriteNode::alloc(gluttonImage, 1, 7);
        //fix the anchor slightly for glutton only
        gluttonSprite->setAnchor(.5, .6);
        glutton->setSceneNode(gluttonSprite);
        glutton->setDebugColor(Color4::BLUE);
        glutton->setGlow(enemyGlow);
        gluttonSprite->setScale(0.2f);
        gluttonSprite->setFrame(0);
        addObstacle(glutton, gluttonSprite, true);
        _enemies.push_back(glutton);
    }
}

void GameScene::createEnemies(int wave) {
    std::vector<string> enemies;
    std::vector<cugl::Vec2> positions;
    enemies = _spawn_order.at(wave);
    positions = _spawn_pos.at(wave);

    for (int i = 0; i < enemies.size(); i++) {
        Vec2 enemyPos;

        enemyPos = positions[i];

        std::string enemyName = enemies[i];
        std::shared_ptr<Texture> enemyGlowImage = _assets->get<Texture>(GLOW_TEXTURE);
        std::shared_ptr<Glow> enemyGlow = Glow::alloc(enemyPos, enemyGlowImage->getSize() / _scale, _scale);
        std::shared_ptr<scene2::PolygonNode> enemyGlowSprite = scene2::PolygonNode::allocWithTexture(enemyGlowImage);
        enemyGlow->setSceneNode(enemyGlowSprite);
        std::shared_ptr<Gradient> grad = Gradient::allocRadial(Color4(255, 255, 255, 85), Color4(111, 111, 111, 0), Vec2(0.5, 0.5), .2f);
        enemyGlowSprite->setGradient(grad);
        enemyGlowSprite->setRelativeColor(false);
        enemyGlowSprite->setScale(.65f);
        addObstacle(enemyGlow, enemyGlowSprite, true);
        // lowercase the enemyName
        std::transform(enemyName.begin(), enemyName.end(), enemyName.begin(),
            [](unsigned char c) { return std::tolower(c); });
        if (!enemyName.compare("lost")) {
            std::shared_ptr<Texture> lostHitBoxImage = _assets->get<Texture>("lost");
            std::shared_ptr<Texture> lostImage = _assets->get<Texture>("lost_ani");
            std::shared_ptr<Lost> lost = Lost::alloc(enemyPos, lostHitBoxImage->getSize(), lostHitBoxImage->getSize() / _scale / 10, _scale);
            std::shared_ptr<scene2::SpriteNode> lostSprite = scene2::SpriteNode::alloc(lostImage, 2, 4);
            lostSprite->setFrame(0);
            lostSprite->setAnchor(Vec2(0.5, 0.25));
            lost->setGlow(enemyGlow);
            lost->setSceneNode(lostSprite);
            lost->setDebugColor(Color4::RED);
            lostSprite->setScale(0.15f);
            lostSprite->setPriority(1.3);
            addObstacle(lost, lostSprite, true);
            _enemies.push_back(lost);
        }
        else if (!enemyName.compare("phantom")) {
            std::shared_ptr<Texture> phantomHitboxImage = _assets->get<Texture>("phantom");
            std::shared_ptr<Texture> phantomImage = _assets->get<Texture>("phantom_ani");
            std::shared_ptr<Phantom> phantom = Phantom::alloc(enemyPos, Vec2(phantomImage->getSize().width / 7, phantomImage->getSize().height), phantomHitboxImage->getSize() / _scale / 10, _scale);
            std::shared_ptr<scene2::SpriteNode> phantomSprite = scene2::SpriteNode::alloc(phantomImage, 1, 7);
            phantom->setSceneNode(phantomSprite);
            phantom->setDebugColor(Color4::BLUE);
            phantom->setGlow(enemyGlow);
            phantomSprite->setScale(0.05f);
            phantomSprite->setFrame(0);
            phantomSprite->setPriority(1.2);
            addObstacle(phantom, phantomSprite, true);
            _enemies.push_back(phantom);
        }
        else if (!enemyName.compare("square")) {
            createMirror(enemyPos, Mirror::Type::square, "squaremirror", enemyGlow);
        }
        else if (!enemyName.compare("triangle")) {
            createMirror(enemyPos, Mirror::Type::triangle, "trianglemirror", enemyGlow);
        }
        else if (!enemyName.compare("circle")) {
            createMirror(enemyPos, Mirror::Type::circle, "circlemirror", enemyGlow);
        }
        else if (!enemyName.compare("seeker")) {
            std::shared_ptr<Texture> seekerImage = _assets->get<Texture>("seeker");
            std::shared_ptr<Seeker> seeker = Seeker::alloc(enemyPos, seekerImage->getSize(), seekerImage->getSize() / _scale / 10, _scale);
            std::shared_ptr<scene2::PolygonNode> seekerSprite = scene2::PolygonNode::allocWithTexture(seekerImage);
            seeker->setSceneNode(seekerSprite);
            seeker->setDebugColor(Color4::GREEN);
            seeker->setGlow(enemyGlow);
            seekerSprite->setScale(0.15f);
            seekerSprite->setPriority(1.1);
            addObstacle(seeker, seekerSprite, true);
            _enemies.push_back(seeker);
        }
        else if (!enemyName.compare("glutton")) {
            std::shared_ptr<Texture> gluttonHitboxImage = _assets->get<Texture>("glutton");
            std::shared_ptr<Texture> gluttonImage = _assets->get<Texture>("glutton_ani");
            std::shared_ptr<Glutton> glutton = Glutton::alloc(enemyPos + Vec2(0, 2), Vec2(gluttonHitboxImage->getSize().width, gluttonHitboxImage->getSize().height), gluttonHitboxImage->getSize() / _scale / 5, _scale);
            std::shared_ptr<scene2::SpriteNode> gluttonSprite = scene2::SpriteNode::alloc(gluttonImage, 2, 7);
            //fix the anchor slightly for glutton only
            gluttonSprite->setAnchor(.5, .4);
            glutton->setSceneNode(gluttonSprite);
            glutton->setDebugColor(Color4::BLUE);
            glutton->setGlow(enemyGlow);
            gluttonSprite->setScale(0.2f);
            gluttonSprite->setFrame(0);
            gluttonSprite->setPriority(1);
            addObstacle(glutton, gluttonSprite, true);
            _enemies.push_back(glutton);
        }
        else if (!enemyName.compare("spawner")) {
            _spawner_ind++;
            _spawnerCount++;
            _spawner_pos.push_back(enemyPos);
            std::shared_ptr<Texture> spawnerImage = _assets->get<Texture>("glutton");
            std::shared_ptr<Spawner> spawner = Spawner::alloc(enemyPos, spawnerImage->getSize(), spawnerImage->getSize() / _scale / 10, _scale);
            std::shared_ptr<scene2::PolygonNode> spawnerSprite = scene2::PolygonNode::allocWithTexture(spawnerImage);
            spawner->setSceneNode(spawnerSprite);
            spawner->setDebugColor(Color4::BLACK);
            spawner->setGlow(enemyGlow);
            spawnerSprite->setScale(0.12f);
            spawnerSprite->setPriority(1);
            addObstacle(spawner, spawnerSprite, true);
            _enemies.push_back(spawner);
            auto spawnerEnemiesMap = _spawner_enemy_types.at(_spawner_ind);
            for (auto it = spawnerEnemiesMap.begin(); it != spawnerEnemiesMap.end(); ++it) {
                int index = it->second.max_count;
                string spawnerEnemyName = it->first;
                while (index != 0) {
                    createSpawnerEnemy(_spawner_ind, spawnerEnemyName);
                    _spawner_enemy_types.at(_spawner_ind).at(spawnerEnemyName).current_count++;
                    index--;
                }
            }
            _living_spawners[_spawner_ind] = 1;
            //            auto spawnTypes = _constants->get("spawner_types")->children();
            //            int index = 0;
            //            for(auto it = spawnTypes.begin(); it != spawnTypes.end(); ++it) {
            //                std::shared_ptr<JsonValue> entry = (*it);
            //                std::vector<string> enemies_types;
            //                for(int i = 0; i < entry->size(); i++) {
            //                    enemies_types.push_back(entry->get(i)->asString());
            //                }
            //                _spawner_types.push_back(enemies_types);
            //                index++;
            //            }
            //            _numWavesSpawner = index;
            //            _nextWaveNumSpawner = 1;
        }
        // TODO add more enemy types
        // If the enemy name is incorrect, no enemy will be made
    }
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
    button->addListener([=](const std::string& name, bool down)
        {
            // Only quit when the button is released
            if (!down) {
                _back = true;
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

    std::shared_ptr<scene2::PolygonNode> floorNode = scene2::PolygonNode::allocWithPoly(floorRect * _scale);
    floorNode->setColor(Color4::BLACK);
    floor->setName("bottomwall");
    b2Filter filter = b2Filter();
    filter.categoryBits = 0b1000;
    // filter.maskBits = 0b1100;
    floor->setFilterData(filter);
    addObstacle(floor, floorNode, 1);

    // Making the ceiling -jdg274
    Rect ceilingRect = Rect(0, DEFAULT_HEIGHT - 0.5, DEFAULT_WIDTH, 0.5);
    std::shared_ptr<physics2::PolygonObstacle> ceiling = physics2::PolygonObstacle::allocWithAnchor(ceilingRect, Vec2::ANCHOR_CENTER);
    ceiling->setName("topwall");
    // filter.maskBits = 0b1100;
    ceiling->setFilterData(filter);

    std::shared_ptr<scene2::PolygonNode> ceilingNode = scene2::PolygonNode::allocWithPoly(ceilingRect * _scale);
    ceilingNode->setColor(Color4::CLEAR);
    addObstacle(ceiling, ceilingNode, 1);

    // Making the left wall -jdg274
    Rect leftRect = Rect(0, 0, 0.5, DEFAULT_HEIGHT);
    std::shared_ptr<physics2::PolygonObstacle> left = physics2::PolygonObstacle::allocWithAnchor(leftRect, Vec2::ANCHOR_CENTER);
    left->setBodyType(b2_staticBody);
    left->setFilterData(filter);

    std::shared_ptr<scene2::PolygonNode> leftNode = scene2::PolygonNode::allocWithPoly(leftRect * _scale);
    leftNode->setName("leftwall");
    leftNode->setColor(Color4::CLEAR);
    addObstacle(left, leftNode, 1);

    // Making the right wall -jdg274
    Rect rightRect = Rect(DEFAULT_WIDTH - 0.5, 0, 0.5, DEFAULT_HEIGHT);
    std::shared_ptr<physics2::PolygonObstacle> right = physics2::PolygonObstacle::allocWithAnchor(rightRect, Vec2::ANCHOR_CENTER);
    right->setBodyType(b2_staticBody);
    right->setFilterData(filter);
    std::shared_ptr<scene2::PolygonNode> rightNode = scene2::PolygonNode::allocWithPoly(rightRect * _scale);
    rightNode->setName("rightwall");
    rightNode->setColor(Color4::CLEAR);
    addObstacle(right, rightNode, 1);

    // Position the button in the bottom right corner
    button->setAnchor(Vec2::ANCHOR_CENTER);
    button->setPosition(size.width - (bsize.width + rOffset) / 2, (bsize.height + bOffset) / 2);

    // Add platforms to the world
    Vec2 pos;
    Rect platRect;
    std::shared_ptr<PlatformModel> platform;
    std::shared_ptr<scene2::PolygonNode> platformSprite;
    std::shared_ptr<Texture> platformImage;
    for(int i = 0; i < _platforms_attr.size(); i++) {
        pos.x = _platforms_attr[i][0];
        pos.y = _platforms_attr[i][1];
        float width = _platforms_attr[i][2];
        if (width < DEFAULT_WIDTH / 3) {
            //use small platform
            platformImage = _assets->get<Texture>("forest_small_platform");
        } else if (width < (DEFAULT_WIDTH / 3) * 2) {
            //use medium platform
            platformImage = _assets->get<Texture>("forest_medium_platform");
        } else {
            //use large platform
            platformImage = _assets->get<Texture>("forest_large_platform");
        }
        platformSprite = scene2::PolygonNode::allocWithTexture(platformImage);
        float desiredWidth = width * _scale;
        float scale = desiredWidth / platformSprite->getWidth();
        platformSprite->setScale(scale);
        platformSprite->setAnchor(0.5,1);
        cout << pos.x << " " << pos.y << " " << width << endl;
        platform = PlatformModel::alloc(pos, width, PLATFORM_HEIGHT, _scale);
        _platforms.push_back(platform);
        _platformNodes.push_back(platformSprite);
        platform->setName("platform");
        platform->setSceneNode(_platformNodes[i]);
        platform->setDebugColor(Color4::RED);
        addObstacle(platform, platformSprite, true);
    }

    // Add the logo and button to the scene graph
    // TODO get rid of this
    scene->addChild(button);

    // Create particles
    // TODO: THIS IS BAD AND MAKING A FAKE "PLAYER"
    //createParticles();

    // Glow effect on player
    Vec2 testPos = PLAYER_POS;
    std::shared_ptr<Texture> imaget = _assets->get<Texture>(GLOW_TEXTURE);
    _playerGlow = Glow::alloc(testPos, imaget->getSize() / _scale, _scale);
    std::shared_ptr<scene2::PolygonNode> spritet = scene2::PolygonNode::allocWithTexture(imaget);
    _playerGlow->setSceneNode(spritet);
    std::shared_ptr<Gradient> grad = Gradient::allocRadial(Color4(255, 255, 255, 55), Color4(111, 111, 111, 0), Vec2(0.5, 0.5), .3f);
    spritet->setGradient(grad);
    spritet->setRelativeColor(false);
    spritet->setScale(.65f);
    addObstacle(_playerGlow, spritet, true);

    // Player creation
    Vec2 playerPos = PLAYER_POS;
    std::shared_ptr<scene2::SceneNode> node = scene2::SceneNode::alloc();
    std::shared_ptr<Texture> image = _assets->get<Texture>(PLAYER_WALK_TEXTURE);
    std::shared_ptr<Texture> hitboxImage = _assets->get<Texture>(PLAYER_TEXTURE);
    _player = PlayerModel::alloc(playerPos + Vec2(0, .5), hitboxImage->getSize() / _scale / 8, _scale);
    _player->setIsStunned(false);
    _player->setMovement(0);
    _player->setWalkAnimationTimer(0);
    _player->setIdleAnimationTimer(0);
    _player->setJumpAnimationTimer(0);
    std::shared_ptr<scene2::SpriteNode> sprite = scene2::SpriteNode::alloc(image, 4, 8);
    sprite->setFrame(0);
    _player->setSceneNode(sprite);
    _player->setDebugColor(Color4::BLUE);
    sprite->setScale(0.175f);
    sprite->setPriority(4);
    addObstacle(_player, sprite, true);
    
    // Ranged Arm for the player
    Vec2 rangeArmPos = PLAYER_POS;
    std::shared_ptr<Texture> rangeHitboxImage = _assets->get<Texture>(PLAYER_RANGE_TEXTURE);
    std::shared_ptr<Texture> rangeImage = _assets->get<Texture>("player_range_arm_ani");
    _rangedArm = Glow::alloc(rangeArmPos, rangeHitboxImage->getSize() / _scale, _scale);
    _rangedArm->setAttackAngle(0);
    _rangedArm->setGlowTimer(0);
    _rangedArm->setAnimeTimer(0);
    _rangedArm->setLastType(AttackController::MeleeState::cool);
    std::shared_ptr<scene2::SpriteNode> rangeArmSprite = scene2::SpriteNode::alloc(rangeImage, 2, 6);
    _rangedArm->setSceneNode(rangeArmSprite);
    rangeArmSprite->setFrame(0);
    rangeArmSprite->setScale(0.22);
    rangeArmSprite->setPriority(5);
    addObstacle(_rangedArm, rangeArmSprite, true);

    //Melee Arm for the player
    Vec2 meleeArmPos = PLAYER_POS;
    std::shared_ptr<Texture> meleeHitboxImage = _assets->get<Texture>(PLAYER_MELEE_TEXTURE);
    std::shared_ptr<Texture> meleeImage = _assets->get<Texture>(PLAYER_MELEE_THREE_TEXTURE);
    _meleeArm = Glow::alloc(meleeArmPos, meleeHitboxImage->getSize() / _scale, _scale);
    _meleeArm->setAttackAngle(0);
    _meleeArm->setGlowTimer(0);
    _meleeArm->setLastType(AttackController::MeleeState::cool);
    std::shared_ptr<scene2::SpriteNode> meleeArmSprite = scene2::SpriteNode::alloc(meleeImage, 4, 7);
    meleeArmSprite->setFrame(21);
    _meleeArm->setSceneNode(meleeArmSprite);
    _meleeArm->setAnimeTimer(0);
    meleeArmSprite->setScale(0.22);
    meleeArmSprite->setPriority(6);
    addObstacle(_meleeArm, meleeArmSprite, true);

    // We can only activate a button AFTER it is added to a scene
    button->activate();

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
    while (ac_it != _attacks->_current.end())
    {
        // int log1 = _world->getObstacles().size();
        cugl::physics2::Obstacle* obj = dynamic_cast<cugl::physics2::Obstacle*>(&**ac_it);
        _world->removeObstacle(obj);
        _worldnode2->removeChild(obj->_node);

        // int log2 = _world->getObstacles().size();
        ac_it = _attacks->_current.erase(ac_it);
    }
    auto ap_it = _attacks->_current.begin();
    while (ap_it != _attacks->_current.end())
    {
        // int log1 = _world->getObstacles().size();
        cugl::physics2::Obstacle* obj = dynamic_cast<cugl::physics2::Obstacle*>(&**ap_it);
        _world->removeObstacle(obj);
        _worldnode2->removeChild(obj->_node);

        // int log2 = _world->getObstacles().size();
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
        cugl::physics2::Obstacle* glowObj = dynamic_cast<cugl::physics2::Obstacle*>(&*(*eit)->getGlow());
        _world->removeObstacle(glowObj);
        _worldnode2->removeChild(glowObj->_node);
        _world->removeObstacle(obj);
        _worldnode2->removeChild(obj->_node);

        eit = _enemies.erase(eit);
    }

    // Reset wave spawning
    _timer = 0.0f;
    _nextWaveNum = 0;
    auto spawnTime = _constants->get("spawn_times");
    for (int i = 0; i < _numWaves; i++) {
        _spawn_times[i] = spawnTime->get(i)->asFloat();
    }

    _endText = nullptr;
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
void GameScene::addObstacle(const std::shared_ptr<cugl::physics2::Obstacle>& obj,
    const std::shared_ptr<cugl::scene2::SceneNode>& node,
    bool useObjPosition)
{
    _world->addObstacle(obj);
    obj->setDebugScene(_debugnode);

    // Position the scene graph node (enough for static objects)
    if (useObjPosition)
    {
        node->setPosition(obj->getPosition() * _scale);
    }
    _worldnode2->addChild(node);
    obj->setNode(node);

    // Dynamic objects need constant updating
    if (obj->getBodyType() == b2_dynamicBody)
    {
        scene2::SceneNode* weak = node.get(); // No need for smart pointer in callback
        obj->setListener([=](physics2::Obstacle* obs)
            {
                weak->setPosition(obs->getPosition() * _scale);
                weak->setAngle(node->getAngle());
            });
    }
}
