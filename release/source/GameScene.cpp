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

/** This is the size of the active portion of the screen */
#define SCENE_WIDTH 1024
// #define SCENE_HEIGHT 576
#define SCENE_HEIGHT 1728

/** Width of the game world in Box2d units */
#define DEFAULT_WIDTH 32.0f

/** The scale of the wavebar*/
#define WAVEBAR_SCALE .8f

/** Height of the game world in Box2d units */
float DEFAULT_HEIGHT = DEFAULT_WIDTH / SCENE_WIDTH * SCENE_HEIGHT;

/** The constant for gravity in the physics world. */
#define GRAVITY 50
#define PLATFORM_ATT 3
#define PLATFORM_COUNT 4
#define PLATFORM_HEIGHT 0.5
#define PLATFORMTEXTURE "platform"
#define TUTORIAL_INIT_TIMER 2
#define TUTORIAL_READING_TIMER 5
/** The initial position of the player*/
float PLAYER_POS[] = {1.0f, 1.0f};

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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager> &assets, const std::shared_ptr<SoundController> sound, string biome, int stageNum, int tutorial)
{
    _back = false;
    _levelselect = false;
    _restart = false;
    _step = false;
    _winInit = true;
    _lose = false;
    _next = false;
    _pause = false;
    _options = false;
    _tutorial = tutorial;
    _initTutorial = tutorial;
    _tutorialTimer = TUTORIAL_INIT_TIMER;
    _spawnParticleTimer = 0.0f;
    _tutorialActionDone = false;
    _tutorialInd = 0;
    _chargeSoundCueM = true;
    _chargeSoundCueR = true;
    std::shared_ptr<JsonReader> reader = JsonReader::alloc(Application::get()->getSaveDirectory() + "savedGame.json");
    std::shared_ptr<JsonValue> save = reader->readJson();
    _progress = save->get("progress");
    std::shared_ptr<JsonValue> settings = save->get("settings");
    reader->close();
    _swap = settings->get("swap")->asInt();
    _sfx = settings->get("sfx")->asInt();
    _music = settings->get("music")->asInt();

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
    _constants = assets->get<JsonValue>(biome + to_string(stageNum));
    _biome = _constants->getString("biome");
    _stageNum = stageNum;
    LEVEL_HEIGHT = _constants->getFloat("level_height");
    PLAYER_POS[0] = _constants->get("start_pos")->get(0)->asFloat();
    PLAYER_POS[1] = _constants->get("start_pos")->get(1)->asFloat();
    auto platformsAttr = _constants->get("platforms")->children();
    _platforms_attr.clear();
    for (auto it = platformsAttr.begin(); it != platformsAttr.end(); ++it)
    {
        std::shared_ptr<JsonValue> entry = (*it);
        float *attr = new float[3];
        attr[0] = entry->get(0)->asFloat();
        attr[1] = entry->get(1)->asFloat();
        attr[2] = entry->get(2)->asFloat();

        _platforms_attr.push_back(attr);
    }

    // Sound controller
    _sound = sound;

    // Get Particle Info
    _particleInfo = assets->get<JsonValue>("particles");

    auto spawn = _constants->get("spawn_order")->children();
    auto spawnPos = _constants->get("spawn_pos");
    auto spawnTime = _constants->get("spawn_times");
    int index = 0;
    Vec2 pos;
    _spawn_order.clear();
    _spawn_pos.clear();
    _spawn_times.clear();
    for (auto it = spawn.begin(); it != spawn.end(); ++it)
    {
        std::shared_ptr<JsonValue> entry = (*it);
        std::vector<string> enemies;
        std::vector<Vec2> enemies_pos;
        for (int i = 0; i < entry->size(); i++)
        {
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
    _spawner_enemy_types.clear();
    _living_spawners.clear();
    _spawnParticlesDone = false;
    _spawner_pos.clear();
    if (_constants->get("spawner_types"))
    {
        auto spawnTypes = _constants->get("spawner_types")->children();
        for (auto it = spawnTypes.begin(); it != spawnTypes.end(); ++it)
        {
            std::shared_ptr<JsonValue> entry = (*it);
            std::unordered_map<string, spawnerEnemy> enemy_types;

            for (int i = 0; i < entry->size(); i++)
            {
                string enemy = entry->get(i)->asString();
                std::transform(enemy.begin(), enemy.end(), enemy.begin(),
                               [](unsigned char c)
                               { return std::tolower(c); });
                if (!enemy_types[enemy].max_count)
                {
                    enemy_types[enemy].max_count = 1;
                    enemy_types[enemy].current_count = 0;
                    enemy_types[enemy].timer = 10.0f;
                }
                else
                {
                    enemy_types[enemy].max_count++;
                }
            }
            _spawner_enemy_types.push_back(enemy_types);
            _living_spawners.push_back(0);
        }
    }
    _spawner_ind = -1;
    _spawnerCount = 0;

    // Create a scene graph the same size as the window
    //_scene = Scene2::alloc(dimen.width, dimen.height);
    // default scene is forest for now
    auto scene = (_assets->get<scene2::SceneNode>("forest"));
    if (!_biome.compare("cave"))
    {
        scene = _assets->get<scene2::SceneNode>("cave");
    }
    else if (!_biome.compare("shroom"))
    {
        scene = _assets->get<scene2::SceneNode>("shroom");
    }
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
    _world->onBeginContact = [this](b2Contact *contact)
    {
        _collider.beginContact(contact, _attacks, _timer);
    };
    _world->onEndContact = [this](b2Contact *contact)
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
    _worldnode->setColor(Color4::WHITE);
    _worldnode2 = scene2::OrderedNode::allocWithOrder(scene2::OrderedNode::Order::ASCEND, bounds.size);
    _worldnode2->setPosition(Vec2(0, 0));
    _worldnode->addChild(_worldnode2);

    // Bounds do not matter when constraint is false
    _debugnode = scene2::ScrollPane::allocWithBounds(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    _debugnode->setScale(_scale); // Debug node draws in PHYSICS coordinates
    _debugnode->setPosition(offset);
    scene->addChild(_debugnode);

    // TODO this init might be wrong, Nick had _scale/2.0f
    _pMeleeTexture = _assets->get<Texture>(PATTACK_TEXTURE);
    _attacks = std::make_shared<AttackController>();
    _attacks->init(_scale, _scale * 1.5, 3, cugl::Vec2(0, 1.25), cugl::Vec2(0, 0.5), 0.8, 1, 0.25, 0.1, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    _dashTime = 0;
    _dashXVel = 0;
    _dashYVel = 0;
    _cancelDash = false;

    _ai = AIController();

    _collider = CollisionController();
    _collider.init(sound);

    setDebug(false);
    buildScene(scene);
    addChildWithName(scene, "scene");

    // Get font
    _font = assets->get<Font>("marker");

    // Grab HUD elements
    _healthbar = std::dynamic_pointer_cast<scene2::ProgressBar>(assets->get<scene2::SceneNode>("HUD_healthbar"));
    _wavebar = std::dynamic_pointer_cast<scene2::ProgressBar>(assets->get<scene2::SceneNode>("HUD_wavebar"));
    _wavebar->setScale(WAVEBAR_SCALE);
    _melee_charge = std::dynamic_pointer_cast<scene2::ProgressBar>(assets->get<scene2::SceneNode>("HUD_melee_charge"));
    _melee_charge->setAngle(M_PI_2);
    _range_charge = std::dynamic_pointer_cast<scene2::ProgressBar>(assets->get<scene2::SceneNode>("HUD_range_charge"));
    _range_charge->setAngle(M_PI_2);
    _dmg2 = std::dynamic_pointer_cast<scene2::SceneNode>(assets->get<scene2::SceneNode>("HUD_dmg_two"));
    _dmg3 = std::dynamic_pointer_cast<scene2::SceneNode>(assets->get<scene2::SceneNode>("HUD_dmg_three"));
    auto HUD = assets->get<scene2::SceneNode>("HUD");
    HUD->setContentSize(dimen);
    HUD->doLayout();

    scene->addChildWithName(HUD, "HUD");
    
    //set wave marker positions
    std::shared_ptr<cugl::Texture> wave_marker = _assets->get<cugl::Texture>("wave_bar_checkpoint");
    
    int total_time = _spawn_times[_numWaves - 1];
    float wave_start_offset = 5;//45;
    float wave_offset = wave_start_offset*2;
    float wave_width = _wavebar->getWidth()-wave_offset;
//    CULog("width: %f", wave_width);
    for(int i = 0; i < _numWaves; i++){
        std::shared_ptr<scene2::PolygonNode> marker = scene2::PolygonNode::allocWithTexture(wave_marker);
        marker->setTag(i+1);
        float percent = _spawn_times[i]/total_time;
        
        //marker->setAnchor(Vec2(percent, 0));
        
        marker->setPositionX((percent*wave_width + wave_start_offset)/WAVEBAR_SCALE);
        //CULog("pos x: %f", percent*wave_width);
        _wavebar->addChild(marker);
    }

    _pauseScene = _assets->get<scene2::SceneNode>("pauseScene");

    _pauseScene->setContentSize(dimen);
    _pauseScene->doLayout();

    float buttonScale = _scale / 32.0f;

    addChildWithName(_pauseScene, "pauseButton");
    _returnButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pauseScene_resume"));
    _returnButton->clearListeners();
    _returnButton->addListener([=](const std::string &name, bool down)
                               {
            // Only quit when the button is released
            if (!down) {
                _pause = false;
            } });
    _returnButton->setScale(.35 * buttonScale);

    _homeButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pauseScene_home"));
    _homeButton->clearListeners();
    _homeButton->addListener([=](const std::string &name, bool down)
                             {
            // Only quit when the button is released
            if (!down) {
                _back = true;
            } });
    _homeButton->setScale(.35 *buttonScale);

    _optionButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pauseScene_options"));
    _optionButton->clearListeners();
    _optionButton->addListener([=](const std::string &name, bool down)
                               {
            // Only quit when the button is released
            if (!down) {
                _options = true;
            } });
    _optionButton->setScale(.35 * buttonScale);


    _restartButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("pauseScene_restart"));
    _restartButton->clearListeners();
    _restartButton->addListener([=](const std::string& name, bool down)
        {
            // Only quit when the button is released
            if (!down) {
                _restart = true;
            } });
    _restartButton->setScale(.35 * buttonScale);

    _optionScene = _assets->get<scene2::SceneNode>("optionScene");
    _optionScene->setContentSize(dimen);
    _optionScene->doLayout();

    addChildWithName(_optionScene, "options");

    addOptionsButtons(buttonScale);

    // lose screen
    _loseScene = _assets->get<scene2::SceneNode>("loseScene");

    _loseScene->setContentSize(dimen);
    _loseScene->doLayout();

    addChildWithName(_loseScene, "lose");
    _loseRestartButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("loseScene_restart"));
    _loseRestartButton->clearListeners();
    _loseRestartButton->addListener([=](const std::string &name, bool down)
                                    {
            // Only quit when the button is released
            if (!down) {
                _restart = true;
            } });
    _loseRestartButton->setScale(.4 * buttonScale);

    _loseHomeButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("loseScene_home"));
    _loseHomeButton->clearListeners();
    _loseHomeButton->addListener([=](const std::string &name, bool down)
                                 {
            // Only quit when the button is released
            if (!down) {
                _back = true;
            } });
    _loseHomeButton->setScale(.4 * buttonScale);

    _loseLevelButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("loseScene_level"));
    _loseLevelButton->clearListeners();
    _loseLevelButton->addListener([=](const std::string &name, bool down)
                                  {
            // Only quit when the button is released
            if (!down) {
                _levelselect = true;
            } });
    _loseLevelButton->setScale(.4 * buttonScale);
    
    // tutorial screen
    if(tutorial == 1) {
        _tutorialSceneFirst = _assets->get<scene2::SceneNode>("tutorialTiltScene");
        _tutorialSceneFirst->setContentSize(dimen);
        _tutorialSceneFirst->doLayout();
        scene->addChildWithName(_tutorialSceneFirst, "tutorialtilt");
        
        _tutorialSceneSecond = _assets->get<scene2::SceneNode>("tutorialMeleeScene");
        _tutorialSceneSecond->setContentSize(dimen);
        _tutorialSceneSecond->doLayout();
        scene->addChildWithName(_tutorialSceneSecond, "tutorialmelee");
        
        _tutorialSceneSecond->setVisible(false);
        _tutorialSceneFirst->setVisible(false);
    } else if (tutorial == 2) {
        _tutorialSceneFirst = _assets->get<scene2::SceneNode>("tutorialJumpScene");
        _tutorialSceneFirst->setContentSize(dimen);
        _tutorialSceneFirst->doLayout();
        scene->addChildWithName(_tutorialSceneFirst, "tutorialjump");
        
        _tutorialSceneSecond = _assets->get<scene2::SceneNode>("tutorialJumpAttackScene");
        _tutorialSceneSecond->setContentSize(dimen);
        _tutorialSceneSecond->doLayout();
        scene->addChildWithName(_tutorialSceneSecond, "tutorialjumpattack");
        
        _tutorialSceneThird = _assets->get<scene2::SceneNode>("tutorialJumpDownScene");
        _tutorialSceneThird->setContentSize(dimen);
        _tutorialSceneThird->doLayout();
        scene->addChildWithName(_tutorialSceneThird, "tutorialjumpdown");
        
        _tutorialSceneThird->setVisible(false);
        _tutorialSceneSecond->setVisible(false);
        _tutorialSceneFirst->setVisible(false);
    } else if (tutorial == 3) {
        _tutorialSceneFirst = _assets->get<scene2::SceneNode>("tutorialRangeScene");
        _tutorialSceneFirst->setContentSize(dimen);
        _tutorialSceneFirst->doLayout();
        scene->addChildWithName(_tutorialSceneFirst, "tutorialrange");
        
        _tutorialSceneSecond = _assets->get<scene2::SceneNode>("tutorialRangeDirScene");
        _tutorialSceneSecond->setContentSize(dimen);
        _tutorialSceneSecond->doLayout();
        scene->addChildWithName(_tutorialSceneSecond, "tutorialrangedir");
        
        _tutorialSceneSecond->setVisible(false);
        _tutorialSceneFirst->setVisible(false);
    } else if (tutorial == 4) {
        _tutorialSceneFirst = _assets->get<scene2::SceneNode>("tutorialChargedRangeScene");
        _tutorialSceneFirst->setContentSize(dimen);
        _tutorialSceneFirst->doLayout();
        scene->addChildWithName(_tutorialSceneFirst, "tutorialchargedrange");
        
        _tutorialSceneSecond = _assets->get<scene2::SceneNode>("tutorialExplosiveDirScene");
        _tutorialSceneSecond->setContentSize(dimen);
        _tutorialSceneSecond->doLayout();
        scene->addChildWithName(_tutorialSceneSecond, "tutorialexplosivedir");
        
        _tutorialSceneThird = _assets->get<scene2::SceneNode>("tutorialExplosiveCooldownScene");
        _tutorialSceneThird->setContentSize(dimen);
        _tutorialSceneThird->doLayout();
        scene->addChildWithName(_tutorialSceneThird, "tutorialexplosivecooldown");
        
        _tutorialSceneThird->setVisible(false);
        _tutorialSceneSecond->setVisible(false);
        _tutorialSceneFirst->setVisible(false);
    } else if (tutorial == 5) {
        _tutorialSceneFirst = _assets->get<scene2::SceneNode>("tutorialChargedMeleeScene");
        _tutorialSceneFirst->setContentSize(dimen);
        _tutorialSceneFirst->doLayout();
        scene->addChildWithName(_tutorialSceneFirst, "tutorialchargedmelee");
        
        _tutorialSceneSecond = _assets->get<scene2::SceneNode>("tutorialDashDirScene");
        _tutorialSceneSecond->setContentSize(dimen);
        _tutorialSceneSecond->doLayout();
        scene->addChildWithName(_tutorialSceneSecond, "tutorialdashdir");
        
        _tutorialSceneThird = _assets->get<scene2::SceneNode>("tutorialDashCooldownScene");
        _tutorialSceneThird->setContentSize(dimen);
        _tutorialSceneThird->doLayout();
        scene->addChildWithName(_tutorialSceneThird, "tutorialdashcooldown");
        
        _tutorialSceneThird->setVisible(false);
        _tutorialSceneSecond->setVisible(false);
        _tutorialSceneFirst->setVisible(false);
    }
    
    _optionScene->setVisible(false);
    _pauseScene->setVisible(false);
    _loseScene->setVisible(false);
    _optionReturnButton->deactivate();
    _swapHandsButton->deactivate();
    _returnButton->deactivate();
    _homeButton->deactivate();
    _optionButton->deactivate();
    _pauseButton->setVisible(true);
    _pauseButton->activate();
    _loseHomeButton->deactivate();
    _loseLevelButton->deactivate();
    _loseRestartButton->deactivate();

    std::string msg = strtool::format("Wave: %d / %d", _nextWaveNum, _numWaves);
    _text = TextLayout::allocWithText(msg, assets->get<Font>("marker"));
    _text->layout();

    int duration = (int)_spawn_times[_nextWaveNum] - (int)_timer;
    std::string timer = strtool::format("Next Wave In: %d", duration);
    _timer_text = TextLayout::allocWithText(msg, assets->get<Font>("marker"));
    _timer_text->layout();

    //Number Texture getting

    _numberTextures.push_back(_assets->get<Texture>("zero"));
    _numberTextures.push_back(_assets->get<Texture>("one"));
    _numberTextures.push_back(_assets->get<Texture>("two"));
    _numberTextures.push_back(_assets->get<Texture>("three"));
    _numberTextures.push_back(_assets->get<Texture>("four"));
    _numberTextures.push_back(_assets->get<Texture>("five"));
    _numberTextures.push_back(_assets->get<Texture>("six"));
    _numberTextures.push_back(_assets->get<Texture>("seven"));
    _numberTextures.push_back(_assets->get<Texture>("eight"));
    _numberTextures.push_back(_assets->get<Texture>("nine"));

    //Mirror shared getting (reducing _assets->get overhead)
        
     _mirrorShardList.push_back(_assets->get<Texture>(MIRROR_SHARD_TEXTURE_1));
     _mirrorShardList.push_back(_assets->get<Texture>(MIRROR_SHARD_TEXTURE_2));
     _mirrorShardList.push_back(_assets->get<Texture>(MIRROR_SHARD_TEXTURE_3));
     _mirrorShardList.push_back(_assets->get<Texture>(MIRROR_SHARD_TEXTURE_4));
     _mirrorShardList.push_back(_assets->get<Texture>(MIRROR_SHARD_TEXTURE_5));
     _mirrorShardList.push_back(_assets->get<Texture>(MIRROR_SHARD_TEXTURE_6));

     //Death Particle getting
     _deathParticleList.push_back(_assets->get<Texture>("death_particle1"));
     _deathParticleList.push_back(_assets->get<Texture>("death_particle2"));
     _deathParticleList.push_back(_assets->get<Texture>("death_particle3"));
     _deathParticleList.push_back(_assets->get<Texture>("death_particle4"));

     //Melee Hit Particle getting
     _meleeParticleList.push_back(_assets->get<Texture>("melee_attack_particle1"));
     _meleeParticleList.push_back(_assets->get<Texture>("melee_attack_particle2"));
     _meleeParticleList.push_back(_assets->get<Texture>("melee_attack_particle3"));
     _meleeParticleList.push_back(_assets->get<Texture>("attack_particle1"));
     _meleeParticleList.push_back(_assets->get<Texture>("attack_particle2"));
     _meleeParticleList.push_back(_assets->get<Texture>("attack_particle3"));
     _meleeParticleList.push_back(_assets->get<Texture>("attack_particle4"));

     //Range Hit Particle getting
     _rangeParticleList.push_back(_assets->get<Texture>("range_attack_particle1"));
     _rangeParticleList.push_back(_assets->get<Texture>("range_attack_particle2"));
     _rangeParticleList.push_back(_assets->get<Texture>("range_attack_particle3"));
     _rangeParticleList.push_back(_assets->get<Texture>("attack_particle1"));
     _rangeParticleList.push_back(_assets->get<Texture>("attack_particle2"));
     _rangeParticleList.push_back(_assets->get<Texture>("attack_particle3"));
     _rangeParticleList.push_back(_assets->get<Texture>("attack_particle4"));

    _timer = 0.0f;
    _worldnode->setColor(Color4::WHITE);
    _healthbar->setColor(Color4::WHITE);
    _pauseButton->setColor(Color4::WHITE);
    _range_charge->setColor(Color4::WHITE);
    _melee_charge->setColor(Color4::WHITE);
    _frameIncrement = 1;
    this->setColor(Color4::WHITE);
    return true;
}

void GameScene::addOptionsButtons(float buttonScale) {

    _optionReturnButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("optionScene_return"));
    _optionReturnButton->clearListeners();
    _optionReturnButton->addListener([=](const std::string& name, bool down)
        {
            // Only quit when the button is released
            if (!down) {
                _options = false;
                _pause = true;
            } });
    _optionReturnButton->setScale(.4 * buttonScale);

    _leftText = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("optionScene_text_left"));
    _rightText = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("optionScene_text_right"));


    _swapHandsButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("optionScene_swap"));
    _swapHandsButton->clearListeners();
    _swapHandsButton->addListener([=](const std::string& name, bool down)
        {
            // Only quit when the button is released
            if (!down) {
                _swap = !_swap;
                this->save();
            } });
    _swapHandsButton->setScale(.4 * buttonScale);
    addMusicButtons(buttonScale);
    addSFXButtons(buttonScale);
}

void GameScene::addMusicButtons(float buttonScale) {
    _musicButtons.clear();
    for (int i = 1; i <= 10; i++) {
        std::shared_ptr<scene2::Button> button = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("optionScene_musicButton" + std::to_string(i)));
        button->setScale(.4 * buttonScale);

        // Create a callback function for the button
        button->setName("music" + i);
        button->clearListeners();
        button->addListener([=](const std::string& name, bool down)
            {
                // Only quit when the button is released
                if (!down) {
                    _music = i;
                    _sound->set_music_volume(i / 10.0f);
                    this->save();
                } });
        _musicButtons.push_back(button);

    }
}

void GameScene::addSFXButtons(float buttonScale) {
    _sfxButtons.clear();
    for (int i = 1; i <= 10; i++) {
        std::shared_ptr<scene2::Button> button = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("optionScene_sfxButton" + std::to_string(i)));
        button->setScale(.4 * buttonScale);

        // Create a callback function for the button
        button->setName("sfx" + i);
        button->clearListeners();
        button->addListener([=](const std::string& name, bool down)
            {
                // Only quit when the button is released
                if (!down) {
                    _sfx = i;
                    _sound->set_sfx_volume(i / 10.0f);
                    this->save();
                } });
        _sfxButtons.push_back(button);

    }
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose()
{
    // Resetting input stuff because dispose is currently being used for switching levels
    _input.reset();
    _swipes.reset();
    _tilt.reset();

    if(_loseHomeButton)
        _loseHomeButton->deactivate();
    _loseHomeButton = nullptr;
    if(_loseLevelButton)
        _loseLevelButton->deactivate();
    _loseLevelButton = nullptr;
    if(_loseRestartButton)
        _loseRestartButton->deactivate();
    _loseRestartButton = nullptr;
    if(_optionReturnButton)
        _optionReturnButton->deactivate();
    _optionReturnButton = nullptr;
    if(_swapHandsButton)
        _swapHandsButton->deactivate();
    _swapHandsButton = nullptr;
    if(_returnButton)
        _returnButton->deactivate();
    _returnButton = nullptr;
    if(_homeButton)
        _homeButton->deactivate();
    _homeButton = nullptr;
    if(_optionButton)
        _optionButton->deactivate();
    _optionButton = nullptr;
    if(_pauseButton)
        _pauseButton->deactivate();
    _pauseButton = nullptr;
    if (_restartButton)
        _restartButton->deactivate();
    _restartButton = nullptr;
    
    _chargeSoundCueM = true;
    _chargeSoundCueR = true;

    // Delete all smart pointers
    _logo = nullptr;
    //    scene = nullptr;
    _batch = nullptr;
    _assets = nullptr;
    _constants = nullptr;
    _world = nullptr;
    if (_worldnode)
        _worldnode->removeAllChildren();
    _worldnode = nullptr;
    if (_worldnode2)
        _worldnode2->removeAllChildren();
    _worldnode2 = nullptr;
    if (_debugnode)
        _debugnode->removeAllChildren();
    _debugnode = nullptr;
    _vertbuff = nullptr;
    _sound = nullptr;
    _text = nullptr;
    _timer_text = nullptr;
    _font = nullptr;
    _endText = nullptr;
    _healthbar = nullptr;
    _range_charge = nullptr;
    _melee_charge = nullptr;
    _numberTextures.clear();
    _mirrorShardList.clear();
    _deathParticleList.clear();
    _rangeParticleList.clear();
    _meleeParticleList.clear();
    if(_wavebar){
    //added i+1 to tag because tags are auto set to 0
        for(int i = 0; i <_numWaves; i++){
            _wavebar->getChildByTag(i+1) = nullptr;
            _wavebar->removeChildByTag(i+1);
        }
    _wavebar = nullptr;
    }
    _lose = false;
    
    // TODO: CHECK IF THIS IS RIGHT FOR DISPOSING
    //    for (auto it = _enemies.begin(); it != _enemies.end(); ++it) {
    //        (*it).~shared_ptr();
    //    }
    // This should work because smart pointers free themselves when vector is cleared
    _enemies.clear();
    _platforms.clear();
    _spawners.clear();
    _spawner_pos.clear();
    if (_attacks)
    {
        _attacks->_current.clear();
        _attacks->_pending.clear();
    }
    _platformNodes.clear();
    _player = nullptr;
    _attacks = nullptr;

    _ai.dispose();
    if (auto scene = getChildByName("scene"))
    {
        //scene->removeChildByName("HUD");
        scene->removeAllChildren();
    }
    _musicButtons.clear();
    _sfxButtons.clear();
    removeAllChildren();
}

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 * @param unlockCount The amount of unlocks the player has.
 *                  1 - ranged attack
 *                  2 - charged ranged attack
 *                  3 - attack upgrade 1
 *                  4 - charged melee attack
 *                  5 - attack upgrade 2
 */
void GameScene::update(float timestep, int unlockCount)
{
    if (_options)
    {
        _optionScene->setVisible(true);
        _pauseScene->setVisible(false);
        _loseScene->setVisible(false);
        _optionReturnButton->activate();
        _swapHandsButton->activate();

        _returnButton->deactivate();
        _homeButton->deactivate();
        _optionButton->deactivate();
        _restartButton->deactivate();
        _pauseButton->setVisible(false);
        _pauseButton->deactivate();
        int counter = 1;
        for (auto it = _musicButtons.begin(); it != _musicButtons.end(); ++it) {
            (*it)->activate();
            (*it)->setVisible(true);
            if (counter <= _music) {
                (*it)->setColor(Color4(255, 255, 255));
            }
            else {
                (*it)->setColor(Color4(150, 150, 150));
            }
            counter++;
        }
        counter = 1;
        for (auto it = _sfxButtons.begin(); it != _sfxButtons.end(); ++it) {
            (*it)->activate();
            (*it)->setVisible(true);
            if (counter <= _sfx) {
                (*it)->setColor(Color4(255, 255, 255));
            }
            else {
                (*it)->setColor(Color4(150, 150, 150));
            }
            counter++;
        }
        if (!_swap) {
            _leftText->setText("range");
            _rightText->setText("melee");
        }
        else {
            _leftText->setText("melee");
            _rightText->setText("range");
        }
        std::vector<bool> e = std::vector<bool>(5);

        for (auto it = _enemies.begin(); it != _enemies.end(); ++it)
        {
            string n = (*it)->getName();
            if (n == "Glutton")
            {
                e[0] = true;
            }
            else if (n == "Phantom")
            {
                e[1] = true;
            }
            else if (n == "Mirror")
            {
                e[2] = true;
            }
            else if (n == "Spawner")
            {
                e[3] = true;
            }
            else if (n == "Seeker")
            {
                e[4] = true;
            }
        }
        _sound->play_level_music(_biome, e);
        return;
    }
    else
    {
        _optionScene->setVisible(false);
        _pauseScene->setVisible(false);
        _optionReturnButton->deactivate();
        _swapHandsButton->deactivate();
        for (auto it = _musicButtons.begin(); it != _musicButtons.end(); ++it) {
            (*it)->deactivate();
        }
        for (auto it = _sfxButtons.begin(); it != _sfxButtons.end(); ++it) {
            (*it)->deactivate();
        }
    }

    if (_pause)
    {
        _pauseScene->setVisible(true);
        _optionScene->setVisible(false);
        _loseScene->setVisible(false);
        _returnButton->activate();
        _homeButton->activate();
        _optionButton->activate();
        _restartButton->activate();

        _pauseButton->setVisible(false);
        _pauseButton->deactivate();
        return;
    }
    else
    {
        _pauseScene->setVisible(false);
        _optionScene->setVisible(false);
        _returnButton->deactivate();
        _homeButton->deactivate();
        _optionButton->deactivate();
        _pauseButton->setVisible(true);
        _pauseButton->activate();
        _restartButton->deactivate();
    }

    if (_lose)
    {
        _sound->level_transition();
        _pauseScene->setVisible(false);
        _optionScene->setVisible(false);
        _loseScene->setVisible(true);
        _loseHomeButton->activate();
        _loseLevelButton->activate();
        _loseRestartButton->activate();
        _pauseButton->setVisible(true);
        _pauseButton->deactivate();
        return;
    }
    else
    {
        _loseScene->setVisible(false);
        _loseHomeButton->deactivate();
        _loseLevelButton->deactivate();
        _loseRestartButton->deactivate();
        _pauseButton->setVisible(true);
        _pauseButton->activate();
    }

    updateSoundInputParticlesAndTilt(timestep);

    if (updateWin())
    {
        _sound->level_transition();
        // let player fall through platforms
        if (_winInit)
        {
            b2Filter filter = _player->getFilterData();
            filter.maskBits = 0b101000;
            _player->setFilterData(filter);
            // set tilt xpos to be constant for moving towards the portal
            _tilt.winTime();
            _winInit = false;
            _winFadeTimer = 0;
        }
        
            this->setColor(Color4(255 - _winFadeTimer * 255 / 1.5, 255 - _winFadeTimer * 255 / 1.5, 255 - _winFadeTimer * 255 / 1.5, 255));
            _winFadeTimer = _winFadeTimer + timestep <= 1.5 ? _winFadeTimer + timestep : 1.5;
            if (_winFadeTimer == 1.5 && _player->getX()>=30)
            {
                _tilt.reset();
                _next = true;
            }
        _player->setVX(_tilt.getXpos());
        _player->setFacingRight(true);

        // perform necessary update loop
        // stop dashing when you win
        _player->setIsDashing(false);
        _meleeArm->setLastType(Glow::MeleeState::cool);
        _rangedArm->setLastType(Glow::MeleeState::cool);

        // remove attacks
        auto ait = _attacks->_current.begin();
        while (ait != _attacks->_current.end())
        {
            (*ait)->markRemoved(true);
            ait++;
        }
        updateAnimations(timestep, unlockCount, SwipeController::noAttack, SwipeController::noAttack);
        updateRemoveDeletedAttacks();
        _world->update(timestep);
        updateCamera();
        updateMeleeArm(timestep);
        return;
    } else {
        std::vector<bool> e = std::vector<bool>(5);

        for (auto it = _enemies.begin(); it != _enemies.end(); ++it)
        {
            string n = (*it)->getName();
            if (n == "Glutton")
            {
                e[0] = true;
            }
            else if (n == "Phantom")
            {
                e[1] = true;
            }
            else if (n == "Mirror")
            {
                e[2] = true;
            }
            else if (n == "Spawner")
            {
                e[3] = true;
            }
            else if (n == "Seeker")
            {
                e[4] = true;
            }
        }
        
        _sound->play_level_music(_biome, e);
    }

    updateTilt();

    if (!_player->isStunned())
    {
        _swipes.update(_input, _player->isGrounded(), _player->isFloored(), timestep, unlockCount);
    }

    SwipeController::SwipeAttack left = updateLeftSwipe(unlockCount);
    SwipeController::SwipeAttack right = updateRightSwipe(unlockCount);
    
    if (_collider.getMeleeReduction() > 0) {
        _swipes.coolMelee(_collider.getMeleeReduction());
        _collider.resetMelee();
    }
    
    if (_collider.getRangeReduction() > 0) {
        _swipes.coolRange(_collider.getRangeReduction());
        _collider.resetRange();
    }
    //Air Stall (doesn't work properly, don't use it)
//    if (_collider.getStall()) {
//        _collider.resetStall();
//        _player->applyAerialSustain();
//    }

    updateAnimations(timestep, unlockCount, left, right);

    updateEnemies(timestep);

    updateAttacks(timestep, unlockCount, left, right);
    updateRemoveDeletedAttacks();

    updateRemoveDeletedEnemies();
    
    updateMeleeArm(timestep);
    
    updateCamera();
    
    updateHUD(unlockCount);
    if (_tutorial) {
        if (_tutorial == 1 || _tutorial == 3) {
            updateTutorialv1(timestep, _tutorialInd);
            return;
        }
        else if (_tutorial == 2 || _tutorial == 4 || _tutorial == 5) {
            updateTutorialv2(timestep, _tutorialInd);
            return;
        }

    }

    updateText();

    updateSpawnTimes();

    updateRemoveDeletedPlayer();

    updateSpawnEnemies(timestep);

}

void GameScene::updateSoundInputParticlesAndTilt(float timestep)
{
    

    // Update input controller
    _input.update(_swap);
    // Debug Mode on/off
    if (_input.getDebugKeyPressed())
    {
        setDebug(!isDebug());
    }

    for (std::shared_ptr<scene2::SceneNode> s : _worldnode2->getChildren()) {
        //update portals
        if (s->getTag() == 69) {
            scene2::PolygonNode* p = dynamic_cast<scene2::PolygonNode*>(s.get());
            p->setAngle(fmod(p->getAngle() - 0.06f, 6.28f));
            if (_spawnParticleTimer > 3.75f) {
                p->removeFromParent();
            }
        }
    }

    ////Update all Particles and Death Animations
    for (std::shared_ptr<scene2::SceneNode> s : _worldnode->getChildren())
    {
        if (s->getTag() == 100)
        {
            ParticleNode *pn = dynamic_cast<ParticleNode *>(s.get());
            pn->update(timestep);
            if (pn->getPool()->isComplete())
            {
                s->removeFromParent();
            }
        }
        else if (s->getTag() == 200) {
            
            scene2::SpriteNode* sp = dynamic_cast<scene2::SpriteNode*>(s.get());
            if (sp->getFrame() == 4) {
                sp->setVisible(false);
            }
            else if (rand() % 100 < 25) { // dead bodies decay at random rate
                sp->setFrame(sp->getFrame() + 1);
            }
            
        }
        else if (s->getTag() == 201) {
            scene2::SpriteNode* sp = dynamic_cast<scene2::SpriteNode*>(s.get());
            if (sp->getFrame() == 5) {
                sp->setVisible(false);
            }
            else if (rand() % 100 < 25) { // dead bodies decay at random rate
                sp->setFrame(sp->getFrame() + 1);
            }
        }
        else if (s->getTag() == 202) {
            scene2::SpriteNode* sp = dynamic_cast<scene2::SpriteNode*>(s.get());
            if (sp->getFrame() == 4) {
                sp->setVisible(false);
            }
            else if (rand() % 100 < 10) { // dead bodies decay at random rate
                sp->setFrame(sp->getFrame() + 1);
            }
        }
        else if (s->getTag() == 203) {
            scene2::SpriteNode* sp = dynamic_cast<scene2::SpriteNode*>(s.get());
            if (sp->getFrame() == 4) {
                sp->setVisible(false);
            }
            else if (rand() % 100 < 25) { // dead bodies decay at random rate
                sp->setFrame(sp->getFrame() + 1);
            }
        }
    };
}

void GameScene::updateTilt()
{
    // Update tilt controller
    _tilt.update(_input, SCENE_WIDTH);
    float xPos = _tilt.getXpos();
    if (_player->isStunned())
    {
        _player->setVX(0);
    }
    else
    {
        _player->setVX(xPos);
        if(_tutorial == 1 && _tutorialInd == 0 && xPos > 0) {
            _tutorialActionDone = true;
        }
    }
}

void GameScene::updateAnimations(float timestep, int unlockCount, SwipeController::SwipeAttack left, SwipeController::SwipeAttack right)
{
    ///////////////////////////////////////
    // Start Player and Arm Animations ////
    ///////////////////////////////////////
    float xPos = _tilt.getXpos();
    int nextFrame;
    scene2::SpriteNode *sprite = dynamic_cast<scene2::SpriteNode *>(_player->getSceneNode().get());

    _rangedArm->getSceneNode()->setVisible(unlockCount >= 1);

    sprite->setAnchor(0.5, 0.3);
    // Player (body) Animations
    if (_player->isStunned())
    {
        // Store the frame being played before stun
        if (sprite->getFrame() != 31 && sprite->getFrame() != 24)
        {
            _prevFrame = sprite->getFrame();
        }
        if (_player->isFacingRight())
        {
            sprite->setFrame(31);
        }
        else
        {
            sprite->setFrame(24);
        }
    }
    else if (_player->isDashing()) {
        switch (_dashDir) {
            case SwipeController::chargedUp:
            case SwipeController::chargedDown:
            {
                if (_player->isFacingRight()) {
                    sprite->setFrame(38);
                }
                else {
                    sprite->setFrame(33);
                }
                break;
            }
            case SwipeController::chargedRight:
                sprite->setFrame(39);
                break;
            case SwipeController::chargedLeft:
                sprite->setFrame(32);
                break;
            case SwipeController::chargedNortheast:
                sprite->setFrame(37);
                break;
            case SwipeController::chargedNorthwest:
                sprite->setFrame(34);
                break;
            case SwipeController::chargedSoutheast:
                sprite->setFrame(36);
                break;
            case SwipeController::chargedSouthwest:
                sprite->setFrame(35);
                break;
            default:
                break;
        }
    }
    else if (!_player->isGrounded())
    {
        if (_player->getJumpAnimationTimer() > 0.03f)
        {
            if (_player->isMovingUp())
            {
                nextFrame = sprite->getFrame();
                if (nextFrame == 31 || nextFrame == 24)
                {
                    nextFrame = _prevFrame;
                }
                if (_player->isFacingRight())
                {
                    if (nextFrame < 20 || nextFrame > 23)
                    {
                        nextFrame = 21;
                    }
                    else if (nextFrame > 20)
                    {
                        nextFrame -= 1;
                    }
                }
                else
                {
                    if (nextFrame < 16 || nextFrame > 19)
                    {
                        nextFrame = 18;
                    }
                    else if (nextFrame < 19)
                    {
                        nextFrame += 1;
                    }
                }
            }
            else
            {
                if (_player->isFacingRight())
                {
                    nextFrame = 19;
                    _player->setJustLanded(true);
                }
                else
                {
                    nextFrame = 20;
                    _player->setJustLanded(true);
                }
            }
            sprite->setFrame(nextFrame);
            _player->setJumpAnimationTimer(0);
        }
        _prevFrame = sprite->getFrame();
    }
    else if (_player->isGrounded() && _player->hasJustLanded())
    {
        if (_player->getJumpAnimationTimer() > 0.06f)
        {
            nextFrame = sprite->getFrame();
            if (nextFrame == 31 || nextFrame == 24)
            {
                nextFrame = _prevFrame;
            }
            if (_player->isFacingRight())
            {
                if (nextFrame > 18 || nextFrame < 16)
                {
                    nextFrame = 18;
                }
                else
                {
                    nextFrame -= 1;
                }
                if (nextFrame == 16)
                {
                    _player->setJustLanded(false);
                }
            }
            else
            {
                if (nextFrame < 21 || nextFrame > 23)
                {
                    nextFrame = 21;
                }
                else
                {
                    nextFrame += 1;
                }
                if (nextFrame == 23)
                {
                    _player->setJustLanded(false);
                }
            }
            sprite->setFrame(nextFrame);
            _player->setJumpAnimationTimer(0);
        }
        _prevFrame = sprite->getFrame();
    }
    else if (xPos != 0 && _player->getWalkAnimationTimer() > 0.09f)
    {
        if (!_player->isFacingRight())
        {
            nextFrame = (sprite->getFrame() + 1) % 8;
            sprite->setFrame(nextFrame);
        }
        else
        {
            if (sprite->getFrame() > 7 || sprite->getFrame() == 0)
            {
                sprite->setFrame(7);
            }
            else
            {
                sprite->setFrame(sprite->getFrame() - 1);
            }
        }
        _player->setWalkAnimationTimer(0);
        _prevFrame = sprite->getFrame();
    }
    else if (xPos == 0 && ((_player->getIdleAnimationTimer() > 1.f) || (!(sprite->getFrame() == 13 || sprite->getFrame() == 8 || sprite->getFrame() == 10 || sprite->getFrame() == 15) && _player->getIdleAnimationTimer() < 0.2f)))
    {
        if (sprite->getFrame() < 8)
        {
            if (_player->isFacingRight())
            {
                nextFrame = 12;
            }
            else
            {
                nextFrame = 8;
            }
        }
        else
        {
            // cause flipHorizontal flips the whole spritesheet >.>
            if (_player->isFacingRight())
            {
                nextFrame = ((sprite->getFrame() + 1) % 4) + 12;
            }
            else
            {
                nextFrame = ((sprite->getFrame() + 1) % 4) + 8;
            }
        }
        sprite->setFrame(nextFrame);
        _player->setIdleAnimationTimer(0);
        _prevFrame = sprite->getFrame();
    }
    else
    {
        if (_player->isFacingRight())
        {
            sprite->setFrame(_prevFrame);
        }
        else
        {
            sprite->setFrame(_prevFrame);
        }
    }

    _player->setJumpAnimationTimer(_player->getJumpAnimationTimer() + timestep);
    _player->setWalkAnimationTimer(_player->getWalkAnimationTimer() + timestep);
    _player->setIdleAnimationTimer(_player->getIdleAnimationTimer() + timestep);
    _rangedArm->setGlowTimer(_rangedArm->getGlowTimer() + timestep);
    _rangedArmCharge->setGlowTimer(_rangedArmCharge->getGlowTimer() + timestep);
    _meleeArm->setGlowTimer(_meleeArm->getGlowTimer() + timestep);
    _meleeArmDash->setGlowTimer(_meleeArmDash->getGlowTimer() + timestep);

    if (sprite->getFrame() == 0 || sprite->getFrame() == 4)
    {
        if (!_step)
        {
            _sound->play_player_sound(SoundController::playerSType::step);
            _step = true;
        }
    }
    else
    {
        _step = false;
    }

    // Arm and Player Flipping
    scene2::TexturedNode *image = dynamic_cast<scene2::TexturedNode *>(_player->getSceneNode().get());
    scene2::TexturedNode *arm1Image = dynamic_cast<scene2::TexturedNode *>(_rangedArm->getSceneNode().get());
    scene2::TexturedNode *armChargeImage = dynamic_cast<scene2::TexturedNode *>(_rangedArmCharge->getSceneNode().get());
    scene2::TexturedNode *arm2Image = dynamic_cast<scene2::TexturedNode *>(_meleeArm->getSceneNode().get());
    scene2::TexturedNode *armDashImage = dynamic_cast<scene2::TexturedNode *>(_meleeArmDash->getSceneNode().get());

    if (image != nullptr)
    {
        image->flipHorizontal(_player->isFacingRight());
    }
    if (arm1Image != nullptr)
    {
        arm1Image->flipHorizontal(_player->getRangedAttackRight());
    }
    if (armChargeImage != nullptr)
    {
        armChargeImage->flipHorizontal(_player->getRangedAttackRight());
    }
    if (arm2Image != nullptr)
    {
        arm2Image->flipHorizontal(_player->isFacingRight());
        if (_meleeArm->getLastType() == Glow::MeleeState::h1_left || _meleeArm->getLastType() == Glow::MeleeState::h2_left || _meleeArm->getLastType() == Glow::MeleeState::h3_left)
        {
            arm2Image->flipHorizontal(false);
        }
        else if (_meleeArm->getLastType() == Glow::MeleeState::h1_right || _meleeArm->getLastType() == Glow::MeleeState::h2_right || _meleeArm->getLastType() == Glow::MeleeState::h3_right)
        {
            arm2Image->flipHorizontal(true);
        }
    }
    if (armDashImage != nullptr)
    {
        armDashImage->flipHorizontal(_player->isFacingRight());
    }

    scene2::SpriteNode *mSprite = dynamic_cast<scene2::SpriteNode *>(_meleeArm->getSceneNode().get());
    scene2::SpriteNode *mdSprite = dynamic_cast<scene2::SpriteNode *>(_meleeArmDash->getSceneNode().get());
    scene2::SpriteNode *rSprite = dynamic_cast<scene2::SpriteNode *>(_rangedArm->getSceneNode().get());
    scene2::SpriteNode *rcSprite = dynamic_cast<scene2::SpriteNode *>(_rangedArmCharge->getSceneNode().get());
    _meleeArm->setAnimeTimer(_meleeArm->getAnimeTimer() + timestep);
    _meleeArmDash->setAnimeTimer(_meleeArmDash->getAnimeTimer() + timestep);
    _rangedArm->setAnimeTimer(_rangedArm->getAnimeTimer() + timestep);
    _rangedArmCharge->setAnimeTimer(_rangedArmCharge->getAnimeTimer() + timestep);

    if (unlockCount >= 2) {
        if (_player->isChargeFiring())
        {
            _rangedArm->getSceneNode()->setVisible(false);
            _rangedArmCharge->getSceneNode()->setVisible(true);
        }
        else
        {
            _rangedArm->getSceneNode()->setVisible(true);
            _rangedArmCharge->getSceneNode()->setVisible(false);;
            if (_player->isFacingRight()) {
                rcSprite->setFrame(7);
            } else {
                rcSprite->setFrame(0);
            }
            
        }
    }
    
    // Ranged Arm
    if (_player->isStunned())
    {
        if(_player->getRangedAttackRight()) {
            rSprite->setFrame(5);
        } else {
            rSprite->setFrame(9);
        }
        rSprite->setAnchor(0.5, 0.5);
        _rangedArm->setAttackAngle(0);
        _rangedArm->setLastType(Glow::MeleeState::cool);
        _rangedArm->setAnimeTimer(0);
    }
    else if (_player->isChargeFiring()) {
        if (_rangedArmCharge->getAnimeTimer() > 0.049f)
        {
            if ((rcSprite->getFrame() == 7 && !_player->getRangedAttackRight()) ||
                (rcSprite->getFrame() == 0 && _player->getRangedAttackRight()))
            {
                // Attack is finished
                if (_player->getRangedAttackRight())
                {
                    rcSprite->setFrame(7);
                    rcSprite->setAnchor(0.5, 0.5);
                    _rangedArmCharge->setAttackAngle(0);
                }
                else
                {
                    rcSprite->setFrame(0);
                    rcSprite->setAnchor(0.5, 0.5);
                    _rangedArmCharge->setAttackAngle(0);
                }
                _rangedArmCharge->setLastType(Glow::MeleeState::cool);
                _rangedArmCharge->setAnimeTimer(0);
                armChargeImage->flipHorizontal(_player->isFacingRight());
                _player->setRangedAttackRight(_player->isFacingRight());
                _player->setIsChargeFiring(false);
            }
            else
            {
                if (_player->getRangedAttackRight())
                {
                    rcSprite->setAnchor(0.8, 0.45);
                    if (rcSprite->getFrame() == 0)
                    {
                        rcSprite->setFrame(8);
                    }
                    else
                    {
                        rcSprite->setFrame(rcSprite->getFrame() - 1);
                    }
                }
                else
                {
                    rcSprite->setAnchor(0.2, 0.45);
                    rcSprite->setFrame((rcSprite->getFrame() + 1) % 8);
                }
                _rangedArmCharge->setAnimeTimer(0);
            }
        }
    }
    else if (_rangedArm->getLastType() == Glow::MeleeState::cool)
    {
        if (_swipes.getLeftChargingTime() >= 100 && _swipes.getLeftChargingTime() < 100 + ((CHARGE_TIME - 100) / 2) && unlockCount >= 2)
        {
            if (_player->getRangedAttackRight())
            {
                rSprite->setFrame(8);
            }
            else
            {
                rSprite->setFrame(6);
            }
        }
        else if (_swipes.getLeftChargingTime() >= 100 + ((CHARGE_TIME - 100) / 2) && _swipes.getLeftChargingTime() < CHARGE_TIME && unlockCount >= 2)
        {
            rSprite->setFrame(7);
        }
        else if (_swipes.getLeftChargingTime() >= CHARGE_TIME && unlockCount >= 2)
        {
            if (_player->getRangedAttackRight())
            {
                rSprite->setFrame(6);
            }
            else
            {
                rSprite->setFrame(8);
            }
        }
        else
        {
            if (_player->getRangedAttackRight())
            {
                rSprite->setFrame(4);
            }
            else
            {
                rSprite->setFrame(0);
            }
        }
        rSprite->setAnchor(0.5, 0.5);
        _rangedArm->setAttackAngle(0);
        _player->setRangedAttackRight(_player->isFacingRight());
    }
    else if (_rangedArm->getLastType() == Glow::MeleeState::first)
    {
        if (_rangedArm->getAnimeTimer() > 0.044f)
        {
            if ((rSprite->getFrame() == 4 && !_player->getRangedAttackRight()) ||
                (rSprite->getFrame() == 0 && _player->getRangedAttackRight()))
            {
                // Attack is finished
                if (_player->getRangedAttackRight())
                {
                    rSprite->setFrame(4);
                    rSprite->setAnchor(0.5, 0.5);
                    _rangedArm->setAttackAngle(0);
                }
                else
                {
                    rSprite->setFrame(0);
                    rSprite->setAnchor(0.5, 0.5);
                    _rangedArm->setAttackAngle(0);
                }
                _rangedArm->setLastType(Glow::MeleeState::cool);
                _rangedArm->setAnimeTimer(0);
                arm1Image->flipHorizontal(_player->isFacingRight());
                _player->setRangedAttackRight(_player->isFacingRight());
            }
            else
            {
                if (_player->getRangedAttackRight())
                {
                    rSprite->setAnchor(0.8, 0.8);
                    if (rSprite->getFrame() == 0)
                    {
                        rSprite->setFrame(4);
                    }
                    else
                    {
                        rSprite->setFrame(rSprite->getFrame() - 1);
                    }
                }
                else
                {
                    rSprite->setAnchor(0.2, 0.8);
                    rSprite->setFrame((rSprite->getFrame() + 1) % 5);
                }
                _rangedArm->setAnimeTimer(0);
            }
        }
    }

    if (_player->isDashing() || _meleeArm->getLastType() == Glow::MeleeState::jump_attack)
    {
        _meleeArm->getSceneNode()->setVisible(false);
        _meleeArmDash->getSceneNode()->setVisible(true);
    }
    else
    {
        _meleeArm->getSceneNode()->setVisible(true);
        _meleeArmDash->getSceneNode()->setVisible(false);
        if (_player->isFacingRight()) {
            mdSprite->setFrame(6);
        } else {
            mdSprite->setFrame(0);
        }
        
    }

    _meleeArm->setAttackAngle(0);
    _meleeArmDash->setAttackAngle(0);
    // Melee Arm
    if (_player->isStunned())
    {
        _meleeArm->getSceneNode()->setVisible(true);
        _meleeArmDash->getSceneNode()->setVisible(false);
        if (_player->isFacingRight())
        {
            mSprite->setFrame(21);
        }
        else
        {
            mSprite->setFrame(27);
        }
        _meleeArm->setLastType(Glow::MeleeState::cool);
        _meleeArm->setAnimeTimer(0);
    }
    else if (_player->isDashing())
    {
        if (_player->isFacingRight())
        {
            _meleeArmDash->setAttackAngle(_player->getDashAngle());
        }
        else
        {
            _meleeArmDash->setAttackAngle(fmod(_player->getDashAngle() + 180, 360));
        }
        if (!_player->dashingLastFrame())
        {
            if (_player->isFacingRight())
            {
                mdSprite->setFrame(6);
            }
            else
            {
                mdSprite->setFrame(0);
            }
            _meleeArmDash->setAnimeTimer(0);
            _player->setDashingLastFrame(true);
        }
        else
        {
            if (_meleeArmDash->getAnimeTimer() > (DASHTIME / 7))
            {
                if (_player->isFacingRight())
                {
                    int nextFrame = mdSprite->getFrame() - 1;
                    if (nextFrame < 0)
                    {
                        nextFrame = 6;
                    }
                    mdSprite->setFrame(nextFrame);
                }
                else
                {
                    int nextFrame = mdSprite->getFrame() + 1;
                    if (nextFrame > 6)
                    {
                        nextFrame = 0;
                    }
                    mdSprite->setFrame(nextFrame);
                }
                _meleeArmDash->setAnimeTimer(0);
            }
        }
    }
    else if (_meleeArm->getLastType() == Glow::MeleeState::cool)
    {
        if (_swipes.getRightChargingTime() >= 100 && _swipes.getRightChargingTime() < 100 + ((CHARGE_TIME - 100) / 2) && unlockCount >= 4)
        {
            if (_player->isFacingRight())
            {
                mSprite->setFrame(26);
            }
            else
            {
                mSprite->setFrame(22);
            }
        }
        else if (_swipes.getRightChargingTime() >= 100 + ((CHARGE_TIME - 100) / 2) && _swipes.getRightChargingTime() < CHARGE_TIME && unlockCount >= 4)
        {
            if (_player->isFacingRight())
            {
                mSprite->setFrame(25);
            }
            else
            {
                mSprite->setFrame(23);
            }
        }
        else if (_swipes.getRightChargingTime() >= CHARGE_TIME && unlockCount >= 4)
        {
            mSprite->setFrame(24);
        }
        else
        {
            if (_player->isFacingRight())
            {
                mSprite->setFrame(7);
            }
            else
            {
                mSprite->setFrame(13);
            }
        }
    }
    else if (_meleeArm->getLastType() == Glow::MeleeState::jump_attack)
    {
        if (_player->isFacingRight())
        {
            _meleeArmDash->setAttackAngle(90);
        }
        else
        {
            _meleeArmDash->setAttackAngle(270);
        }
        if (_player->isFacingRight()){
            if (mdSprite->getFrame() > 2) {
                mdSprite->setFrame(0);
                _meleeArmDash->setAnimeTimer(0);
                _frameIncrement = 1;
            }
            else {
                if (_meleeArmDash->getAnimeTimer() > 0.06f) {
                    nextFrame = mdSprite->getFrame() + _frameIncrement;
                    if (nextFrame > 2) {
                        nextFrame = 2;
                        _frameIncrement = -1;
                    }
                    if (nextFrame < 0) {
                        nextFrame = 0;
                    }
                    mdSprite->setFrame(nextFrame);
                    _meleeArmDash->setAnimeTimer(0);
                }
            }
        }
        else {
            if (mdSprite->getFrame() < 4) {
                mdSprite->setFrame(6);
                _meleeArmDash->setAnimeTimer(0);
                _frameIncrement = 1;
            }
            else {
                if (_meleeArmDash->getAnimeTimer() > 0.06f) {
                    nextFrame = mdSprite->getFrame() - _frameIncrement;
                    if (nextFrame < 4) {
                        nextFrame = 4;
                        _frameIncrement = -1;
                    }
                    if (nextFrame > 6) {
                        nextFrame = 6;
                    }
                    mdSprite->setFrame(nextFrame);
                    _meleeArmDash->setAnimeTimer(0);
                }
            }
        }
    }
    else if (_meleeArm->getLastType() == Glow::MeleeState::h1_left)
    {
        if (_meleeArm->getAnimeTimer() > 0.05f)
        {
            if (mSprite->getFrame() == 12)
            {
                // Attack is finished
                _meleeArm->setLastType(Glow::MeleeState::cool);
                _meleeArm->setAnimeTimer(0);
            }
            else
            {
                if (mSprite->getFrame() == 13)
                {
                    mSprite->setFrame(7);
                }
                else
                {
                    mSprite->setFrame(((mSprite->getFrame() + 1) % 7) + 7);
                }
                _meleeArm->setAnimeTimer(0);
            }
        }
    }
    else if (_meleeArm->getLastType() == Glow::MeleeState::h2_left)
    {
        if (_meleeArm->getAnimeTimer() > 0.06f)
        {
            if (mSprite->getFrame() == 4)
            {
                // Attack is finished
                _meleeArm->setLastType(Glow::MeleeState::cool);
                _meleeArm->setAnimeTimer(0);
            }
            else
            {
                if (mSprite->getFrame() > 6)
                {
                    mSprite->setFrame(0);
                }
                else
                {
                    mSprite->setFrame((mSprite->getFrame() + 1) % 5);
                }
                _meleeArm->setAnimeTimer(0);
            }
        }
    }
    else if (_meleeArm->getLastType() == Glow::MeleeState::h3_left)
    {
        if (_meleeArm->getAnimeTimer() > 0.05f)
        {
            if (mSprite->getFrame() == 20)
            {
                // Attack is finished
                _meleeArm->setLastType(Glow::MeleeState::cool);
                _meleeArm->setAnimeTimer(0);
            }
            else
            {
                if (mSprite->getFrame() < 14)
                {
                    mSprite->setFrame(14);
                }
                else
                {
                    mSprite->setFrame(((mSprite->getFrame() + 1) % 7) + 14);
                }
                _meleeArm->setAnimeTimer(0);
            }
        }
    }
    else if (_meleeArm->getLastType() == Glow::MeleeState::h1_right)
    {
        if (_meleeArm->getAnimeTimer() > 0.05f)
        {
            if (mSprite->getFrame() == 8)
            {
                // Attack is finished
                _meleeArm->setLastType(Glow::MeleeState::cool);
                _meleeArm->setAnimeTimer(0);
            }
            else
            {
                if (mSprite->getFrame() <= 7)
                {
                    mSprite->setFrame(13);
                }
                mSprite->setFrame(mSprite->getFrame() - 1);
                _meleeArm->setAnimeTimer(0);
            }
        }
    }
    else if (_meleeArm->getLastType() == Glow::MeleeState::h2_right)
    {
        if (_meleeArm->getAnimeTimer() > 0.06f)
        {
            if (mSprite->getFrame() == 2)
            {
                // Attack is finished
                _meleeArm->setLastType(Glow::MeleeState::cool);
                mSprite->setFrame(7);
                _meleeArm->setAnimeTimer(0);
            }
            else
            {
                if (mSprite->getFrame() >= 7)
                {
                    mSprite->setFrame(6);
                }
                mSprite->setFrame(mSprite->getFrame() - 1);
                _meleeArm->setAnimeTimer(0);
            }
        }
    }
    else if (_meleeArm->getLastType() == Glow::MeleeState::h3_right)
    {
        if (_meleeArm->getAnimeTimer() > 0.05f)
        {
            if (mSprite->getFrame() == 14)
            {
                // Attack is finished
                _meleeArm->setLastType(Glow::MeleeState::cool);
                _meleeArm->setAnimeTimer(0);
            }
            else
            {
                if (mSprite->getFrame() < 13)
                {
                    mSprite->setFrame(20);
                }
                mSprite->setFrame(mSprite->getFrame() - 1);
                _meleeArm->setAnimeTimer(0);
            }
        }
    }
    else
    {
        if (_player->isFacingRight())
        {
            mSprite->setFrame(7);
        }
        else
        {
            mSprite->setFrame(13);
        }
    }

    float offsetArm = -2.6f;
    if (_player->isChargeFiring()) {
        offsetArm = -3.1f;
    }
    else if (_rangedArm->getLastType() != Glow::MeleeState::cool)
    {
        offsetArm = -3.0f;
    }

    if (!_player->getRangedAttackRight())
    {
        offsetArm = -1 * offsetArm;
    }
    
    if ((_player->isChargeFiring() && !_player->getRangedAttackRight() && _rangedArmCharge->getAttackAngle() > 90 && _rangedArmCharge->getAttackAngle() < 270) ||
        (_player->isChargeFiring() && _player->getRangedAttackRight() && (_rangedArmCharge->getAttackAngle() > 90 && _rangedArmCharge->getAttackAngle() < 270)))
    {
        offsetArm = -1 * offsetArm;
    }
    else if ((!_player->getRangedAttackRight() && rSprite->getFrame() != 0 && _rangedArm->getAttackAngle() > 90 && _rangedArm->getAttackAngle() < 270) ||
        (_player->getRangedAttackRight() && rSprite->getFrame() != 4 && (_rangedArm->getAttackAngle() > 90 && _rangedArm->getAttackAngle() < 270)))
    {
        offsetArm = -1 * offsetArm;
    }

    float upDown = _rangedArm->getGlowTimer();
    float spacing = 1.f;
    float upDownY1 = fmod(upDown / 2, spacing);

    if (upDownY1 > spacing / 4 && upDownY1 <= 3 * spacing / 4)
    {
        upDownY1 = spacing / 2 - upDownY1;
    }
    else if (upDownY1 > 3 * spacing / 4)
    {
        upDownY1 = -1 * spacing + upDownY1;
    }

    if (_player->isChargeFiring()) {
        float upOffset = 0.3f;
        if (_player->getRangedAttackRight())
        {
            if (_rangedArmCharge->getAttackAngle() > 90 && _rangedArmCharge->getAttackAngle() < 270)
            {
                _rangedArm->setPosition(_player->getPosition().x + offsetArm - 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
                _rangedArmCharge->setPosition(_player->getPosition().x + offsetArm - 2, _player->getPosition().y + (upDownY1 / spacing / 3) + upOffset);
            }
            else
            {
                _rangedArm->setPosition(_player->getPosition().x + offsetArm + 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
                _rangedArmCharge->setPosition(_player->getPosition().x + offsetArm + 2, _player->getPosition().y + (upDownY1 / spacing / 3) + upOffset);
            }
        }
        else
        {
            if (_rangedArmCharge->getAttackAngle() > 90 && _rangedArmCharge->getAttackAngle() < 270)
            {
                _rangedArm->setPosition(_player->getPosition().x + offsetArm + 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
                _rangedArmCharge->setPosition(_player->getPosition().x + offsetArm + 2, _player->getPosition().y + (upDownY1 / spacing / 3) + upOffset);
            }
            else
            {
                _rangedArm->setPosition(_player->getPosition().x + offsetArm - 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
                _rangedArmCharge->setPosition(_player->getPosition().x + offsetArm - 2, _player->getPosition().y + (upDownY1 / spacing / 3) + upOffset);
            }
        }
    }
    else {
        if (_player->getRangedAttackRight() && (rSprite->getFrame() != 4 && rSprite->getFrame() != 6 && rSprite->getFrame() != 7 && rSprite->getFrame() != 8))
        {
            if (_rangedArm->getAttackAngle() > 90 && _rangedArm->getAttackAngle() < 270)
            {
                _rangedArm->setPosition(_player->getPosition().x + offsetArm - 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
                _rangedArmCharge->setPosition(_player->getPosition().x + offsetArm - 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
            }
            else
            {
                _rangedArm->setPosition(_player->getPosition().x + offsetArm + 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
                _rangedArmCharge->setPosition(_player->getPosition().x + offsetArm + 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
            }
        }
        else if (!_player->getRangedAttackRight() && (rSprite->getFrame() != 0 && rSprite->getFrame() != 6 && rSprite->getFrame() != 7 && rSprite->getFrame() != 8))
        {
            if (_rangedArm->getAttackAngle() > 90 && _rangedArm->getAttackAngle() < 270)
            {
                _rangedArm->setPosition(_player->getPosition().x + offsetArm + 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
                _rangedArmCharge->setPosition(_player->getPosition().x + offsetArm + 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
            }
            else
            {
                _rangedArm->setPosition(_player->getPosition().x + offsetArm - 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
                _rangedArmCharge->setPosition(_player->getPosition().x + offsetArm - 2, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
            }
        }
        else
        {
            _rangedArm->setPosition(_player->getPosition().x + offsetArm, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
            _rangedArmCharge->setPosition(_player->getPosition().x + offsetArm, _player->getPosition().y + (upDownY1 / spacing / 3) + 0.2f);
        }
    }

    ////////////////////////////////////////
    ///////End Player and Arm Animations////
    ////////////////////////////////////////

    // TODO: MAKE SURE MOVING THIS FROM BOTTOM OF UPDATE LOOP DOES NOT BREAK ANYTHING
    _playerGlow->setPosition(_player->getPosition());
}

void GameScene::updateMeleeArm(float timestep)
{
    ////MELEE ARM MUST STAY AT BOTTOM
    // Determining arm positions and offsets
    float offsetArm2 = -3.2f;
    if (_player->isDashing() || _meleeArm->getLastType() == Glow::MeleeState::jump_attack)
    {
        offsetArm2 = -1.0f;
    }

    // change based on arm attacks
    if ((!_player->isFacingRight() ||                                                                                                                                                // player facing left or attacks left
         (_meleeArm->getLastType() == Glow::MeleeState::h1_left || _meleeArm->getLastType() == Glow::MeleeState::h2_left || _meleeArm->getLastType() == Glow::MeleeState::h3_left))) // player facing left and attacking right
    {
        offsetArm2 = -1 * offsetArm2;
    }

    if (!_player->isFacingRight() && (_meleeArm->getLastType() == Glow::MeleeState::h1_right || _meleeArm->getLastType() == Glow::MeleeState::h2_right || _meleeArm->getLastType() == Glow::MeleeState::h3_right))
    {
        offsetArm2 = -1 * offsetArm2;
    }
    float spacing = 1.f;
    float upDown2 = _meleeArm->getGlowTimer() + 0.5f;
    float upDownY2 = fmod(upDown2 / 2, spacing);
    if (upDownY2 > spacing / 4 && upDownY2 <= 3 * spacing / 4)
    {
        upDownY2 = spacing / 2 - upDownY2;
    }
    else if (upDownY2 > 3 * spacing / 4)
    {
        upDownY2 = -1 * spacing + upDownY2;
    }
    if (_player->isDashing())
    {
        _meleeArm->setPosition(_player->getPosition().x - offsetArm2, _player->getPosition().y + (upDownY2 / spacing / 3) + 0.6f);
        _meleeArmDash->setPosition(_player->getPosition().x - offsetArm2, _player->getPosition().y + (upDownY2 / spacing / 3) + 0.6f);
    }
    else if (_meleeArm->getLastType() == Glow::MeleeState::jump_attack) {
        _meleeArm->setPosition(_player->getPosition().x - offsetArm2, _player->getPosition().y + (upDownY2 / spacing / 3) + 0.5f);
        _meleeArmDash->setPosition(_player->getPosition().x - offsetArm2, _player->getPosition().y + (upDownY2 / spacing / 3) + 0.5f);
    }
    else
    {
        _meleeArm->setPosition(_player->getPosition().x - offsetArm2, _player->getPosition().y + (upDownY2 / spacing / 3) + 0.2f);
        _meleeArmDash->setPosition(_player->getPosition().x - offsetArm2, _player->getPosition().y + (upDownY2 / spacing / 3) + 0.2f);
    }
}

void GameScene::updateEnemies(float timestep)
{
    // Enemy AI logic
    // For each enemy
    std::shared_ptr<Texture> melee_impact = _assets->get<Texture>("melee_impact");
    std::shared_ptr<Texture> ranged_impact = _assets->get<Texture>("ranged_impact");
    for (auto it = _enemies.begin(); it != _enemies.end(); ++it)
    {
        Vec2 direction = _ai.getMovement(*it, _player->getPosition(), timestep, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT);

        (*it)->setVX(direction.x);
        if ((*it)->getName() == "Lost")
        {
            float distance = _player->getPosition().distance((*it)->getPosition());
            if (distance < 8 && _player->getY() - _player->getHeight() / 2 > (*it)->getY() - (*it)->getHeight() / 2 + 0.5 && _player->isGrounded())
            {
                if ((*it)->isGrounded() && abs((*it)->getVY()) < 0.01)
                {
                    (*it)->setVY(25);
                    (*it)->setJumping(true);
                }
                else if ((*it)->isJumping() && abs((*it)->getVY()) < 0.01)
                {
                    (*it)->setJumping(false);
                    (*it)->setFalling(true);
                }
                else if ((*it)->isFalling() && abs((*it)->getVY()) < 0.01)
                {
                    (*it)->setFalling(false);
                    (*it)->setGrounded(true);
                }
            }
        }
        else
        {
            (*it)->setVY(direction.y);
        }
        (*it)->getGlow()->setPosition((*it)->getPosition());
        (*it)->setAttackAnimationTimer((*it)->getAttackAnimationTimer() + timestep);
        (*it)->setInvincibilityTimer((*it)->getInvincibilityTimer() - timestep);
        (*it)->setIdleAnimationTimer((*it)->getIdleAnimationTimer() + timestep);

        scene2::SpriteNode *sprite = dynamic_cast<scene2::SpriteNode *>((*it)->getSceneNode().get());

        float devilScale;
        if ((*it)->getName() == "Glutton" || (*it)->getName() == "Spawner")
        {
            devilScale = 0.2;
        }
        else
        {
            devilScale = 0.1;
        }

        if ((*it)->getInvincibilityTimer() > 0 && !(*it)->getPlayedDamagedParticle())
        {
            (*it)->setPlayedDamagedParticle(true);
            float damageParticleScale;
            if ((*it)->getName() == "Spawner") {
                damageParticleScale = 0.15;
            }
            else if ((*it)->getName() == "Glutton") {
                damageParticleScale = 0.2;
            }
            else {
                damageParticleScale = 0.1;
            }
            if ((*it)->getLastDamagedBy() == BaseEnemyModel::AttackType::p_melee || (*it)->getLastDamagedBy() == BaseEnemyModel::AttackType::p_dash)
            {
                createParticles(melee_impact, (*it)->getPosition() * _scale, "devil", Color4::WHITE, Vec2(0, 0), damageParticleScale);
                createParticles(_meleeParticleList, (*it)->getPosition() * _scale, "sparks", Color4::WHITE, Vec2(0, 0), damageParticleScale*2, false, Vec2(), 7);
            }
            else
            {
                createParticles(ranged_impact, (*it)->getPosition() * _scale, "devil", Color4::WHITE, Vec2(0, 0), damageParticleScale);
                createParticles(_rangeParticleList, (*it)->getPosition() * _scale, "sparks", Color4::WHITE, Vec2(0, 0), damageParticleScale*2, false, Vec2(), 7);
            }
            if ((*it)->getLastDamageAmount() < 10)
            {
                std::vector<std::shared_ptr<Texture> > num;
                num.push_back(_numberTextures[(*it)->getLastDamageAmount()]);
                createParticles(num, (*it)->getPosition() * _scale, "number", Color4::WHITE, Vec2(0, 10), 0.1f, true, Vec2(), 0);
            }
            else
            {
                std::vector<std::shared_ptr<Texture> > num = getTexturesFromNumber((*it)->getLastDamageAmount());
                createParticles(num, (*it)->getPosition() * _scale, "number", Color4::WHITE, Vec2(0, 10), 0.1f, true, Vec2(-10, 0), 0);
            }
        }

        // For running idle animations specific (for speed) to enemies
        if ((*it)->getName() == "Phantom")
        {

            if ((*it)->getInvincibilityTimer() > 0)
            {
                // Hurt takes priority over everything else
                sprite->setFrame(11);
            }
            else if ((*it)->isAttacking()) {
                // First frame of attack
                if (sprite->getFrame() < 7 || sprite->getFrame() >= 11) {
                    sprite->setFrame(7);
                    (*it)->setAttackAnimationTimer(0);
                }
                else if ((*it)->getAttackAnimationTimer() > .33f) {
                    // Attack Animation up to release of attack
                    if (sprite->getFrame() != 9) {
                        sprite->setFrame(((sprite->getFrame() + 1) % 4) + 8);
                    }
                    (*it)->setAttackAnimationTimer(0);
                }
            }
            else {

                //start idle is post attack part of animation done, or if coming out of hurt
                if (sprite->getFrame() == 11 || (sprite->getFrame() == 10 && (*it)->getIdleAnimationTimer() > .1f)) {
                    sprite->setFrame(0);
                }

                //finish post attack animation, then start idle otherwise
                if (sprite->getFrame() == 9 && (*it)->getAttackAnimationTimer() > .33f) {
                    sprite->setFrame(10);
                    (*it)->setAttackAnimationTimer(0);
                    (*it)->setIdleAnimationTimer(-0.1);
                }
                else if (((*it)->getIdleAnimationTimer() > .1f || sprite->getFrame() == 11))
                {
                    sprite->setFrame(((sprite->getFrame() + 1) % 7));
                    (*it)->setIdleAnimationTimer(0);
                }
            }
        }
        else if ((*it)->getName() == "Glutton")
        {
            if ((*it)->getInvincibilityTimer() > 0)
            {

                if (!sprite->isFlipHorizontal())
                {
                    sprite->setFrame(14);
                }
                else {
                    sprite->setFrame(20);
                }
            }
            else if ((*it)->isAttacking()) {
                if (sprite->getFrame() < 21) {
                    if (!sprite->isFlipHorizontal()) {
                        sprite->setFrame(21);
                    }
                    else {
                        sprite->setFrame(27);
                    }
                    (*it)->setAttackAnimationTimer(-0.4f);
                }
                else if ((*it)->getAttackAnimationTimer() > 1.f) {
                    if (!sprite->isFlipHorizontal()) {
                        if (sprite->getFrame() != 25) {
                            sprite->setFrame(((sprite->getFrame() + 1)% 7) + 21);
                        }
                        if (sprite->getFrame() == 22) {
                            (*it)->setAttackAnimationTimer(0.6f);
                        }
                        else {
                            (*it)->setAttackAnimationTimer(0.9f);
                        }
                    }
                    else {
                        if (sprite->getFrame() != 23) {
                            sprite->setFrame(((sprite->getFrame() - 1) % 7) + 21);
                        }
                        if (sprite->getFrame() == 26) {
                            (*it)->setAttackAnimationTimer(0.6f);
                        }
                        else {
                            (*it)->setAttackAnimationTimer(0.9f);
                        }
                    }
                }
            }
            else {

                if (sprite->getFrame() == 14 || (sprite->getFrame() == 27 && (*it)->getAttackAnimationTimer() > 1.f)) {
                    sprite->setFrame(0);
                }
                else if (sprite->getFrame() == 20 || (sprite->getFrame() == 21 && (*it)->getAttackAnimationTimer() > 1.f)) {
                    sprite->setFrame(6);
                }

                if (sprite->getFrame() > 21 && (*it)->getAttackAnimationTimer() > 1.f) {
                    if (!sprite->isFlipHorizontal()) {
                        sprite->setFrame(sprite->getFrame() + 1);
                    }
                    else {
                        sprite->setFrame(sprite->getFrame() - 1);
                    }
                    (*it)->setIdleAnimationTimer(0);
                    (*it)->setAttackAnimationTimer(0.9);
                }
                if ((*it)->getX() > _player->getX() && !(sprite->getFrame() > 21)) {
                    sprite->flipHorizontal(false);
                }
                else if ((*it)->getX() < _player->getX() && !(sprite->getFrame() > 21)) {
                    sprite->flipHorizontal(true);
                }
                if (((*it)->getIdleAnimationTimer() > .1f || sprite->getFrame() == 14 || sprite->getFrame() == 20) && !(sprite->getFrame() > 21))
                {
                    sprite->setFrame((sprite->getFrame() + 1) % 7);
                    (*it)->setIdleAnimationTimer(0);
                }
                else if (((*it)->getIdleAnimationTimer() > .1f || sprite->getFrame() == 14 || sprite->getFrame() == 20) && !(sprite->getFrame() > 21))
                {
                    sprite->setFrame((sprite->getFrame() - 1) % 7);
                    (*it)->setIdleAnimationTimer(0);
                }
            }
        }
        else if ((*it)->getName() == "Lost")
        {
            if ((*it)->getInvincibilityTimer() > 0)
            {
                if (!sprite->isFlipHorizontal())
                {
                    sprite->setFrame(4);
                }
                else
                {
                    sprite->setFrame(7);
                }
            } else if ((*it)->isAttacking()) {
                if (sprite->getFrame() < 8) {
                    if (!sprite->isFlipHorizontal()) {
                        sprite->setFrame(8);
                    }
                    else {
                        sprite->setFrame(11);
                    }
                    (*it)->setAttackAnimationTimer(0.0f);
                }
                else if ((*it)->getAttackAnimationTimer() > .2f) {
                    if (!sprite->isFlipHorizontal()) {
                        sprite->setFrame(9);
                    }
                    else {
                        sprite->setFrame(10);
                    }
                }
            }
            else
            {
                if (sprite->getFrame() == 4 || sprite->getFrame() == 9) {
                    sprite->setFrame(0);
                }
                else if (sprite->getFrame() == 7 || sprite->getFrame() == 10) {
                    sprite->setFrame(3);
                }
                if ((*it)->getVX() > 0)
                {
                    sprite->flipHorizontal(false);
                }
                else if ((*it)->getVX() < 0)
                {
                    sprite->flipHorizontal(true);
                }

                // Using idle timer for walking animation since lost has no idle
                if (((*it)->getVX() > 0) && ((*it)->getIdleAnimationTimer() > .1f || sprite->getFrame() == 4 || sprite->getFrame() == 7))
                {
                    sprite->setFrame((sprite->getFrame() + 1) % 4);
                    (*it)->setIdleAnimationTimer(0);
                }
                else if (((*it)->getVX() < 0) && ((*it)->getIdleAnimationTimer() > .1f || sprite->getFrame() == 4 || sprite->getFrame() == 7))
                {
                    sprite->setFrame((sprite->getFrame() - 1) % 4);
                    (*it)->setIdleAnimationTimer(0);
                }
            }
        }
        else if ((*it)->getName() == "Seeker")
        {
        if ((*it)->getInvincibilityTimer() > 0)
        {
            sprite->setFrame(12);
        }
        else if ((*it)->isAttacking()) {
            if (sprite->getFrame() < 6 || sprite->getFrame() >= 12) {
                sprite->setFrame(6);
                (*it)->setAttackAnimationTimer(0);
            }
            else if ((*it)->getAttackAnimationTimer() > .06f) {
                if (sprite->getFrame() != 9) {
                    sprite->setFrame(((sprite->getFrame() + 1) % 4) + 6);
                }
                if (sprite->getFrame() == 7) {
                    (*it)->setAttackAnimationTimer(0);
                }
                else if (sprite->getFrame() == 2) {
                    (*it)->setAttackAnimationTimer(.05f);
                }
            }
        }
        else {

            if (sprite->getFrame() == 12 || sprite->getFrame() == 9) {
                sprite->setFrame(0);
            }

            if (((*it)->getIdleAnimationTimer() > .1f || sprite->getFrame() == 12 || sprite->getFrame() == 9))
            {
                sprite->setFrame(((sprite->getFrame() + 1) % 6));
                if (sprite->getFrame() == 1) {
                    (*it)->setIdleAnimationTimer(-0.2);
                }
                else if (sprite->getFrame() == 4) {
                   (*it)->setIdleAnimationTimer(-0.3);
                }
                else {
                    (*it)->setIdleAnimationTimer(0);
                }
            }
        }
        }
        else if ((*it)->getName() == "Spawner")
        {
            if ((*it)->getSpawned() || sprite->getFrame() != 0)
            {
                // Using idle animation timer for spawning animation (not sure if it will have an idle)
                if ((*it)->getIdleAnimationTimer() > 0.05f)
                {
                    sprite->setFrame((sprite->getFrame() + 1) % 21);
                    (*it)->setIdleAnimationTimer(0);
                    (*it)->setSpawned(false);
                }
            }
        }

        if ((*it)->getInvincibilityTimer() <= 0)
        {
            (*it)->setInvincibility(false);
        }
        if (!(*it)->attackIsCompleted())
        {
            Vec2 play_p = _player->getPosition();
            Vec2 en_p = (*it)->getPosition();
            Vec2 vel = Vec2(0.5, 0);

            (*it)->setAttackCompleted(true);
            // TODO: Need to variablize attack variables based on enemy type
            if ((*it)->getName() == "Seeker")
            {
                shared_ptr<Seeker> seeker = dynamic_pointer_cast<Seeker>(*it);
                (*it)->setAttackCompleted(true);

                _attacks->createAttack(Vec2((*it)->getX(), (*it)->getY()), 1.0f, 0.2f, seeker->getAttackDamage(), AttackController::Type::e_melee, (vel.scale(0.2)).rotate((play_p - en_p).getAngle()), _timer, SEEKER_ATTACK, 0);
                _sound->play_enemy_sound(SoundController::enemy::seeker, SoundController::etype::attack);
            }

            if ((*it)->getName() == "Lost")
            {
                _attacks->createAttack(Vec2((*it)->getX(), (*it)->getY()), 1.0f, 0.2f, (*it)->getAttackDamage(), AttackController::Type::e_melee, vel.rotate((play_p - en_p).getAngle()), _timer, LOST_ATTACK, 0);
                _sound->play_enemy_sound(SoundController::enemy::lost, SoundController::etype::attack);
            }
            else if ((*it)->getName() == "Phantom")
            {
                _attacks->createAttack(Vec2((*it)->getX(), (*it)->getY()), 0.5f, 3.0f, (*it)->getAttackDamage(), AttackController::Type::e_range, (vel.scale(0.5)).rotate((play_p - en_p).getAngle()), _timer, PHANTOM_ATTACK, PHANTOM_FRAMES);
                _sound->play_enemy_sound(SoundController::enemy::phantom, SoundController::etype::attack);
            }
            else if ((*it)->getName() == "Glutton")
            {
                _attacks->createAttack(Vec2((*it)->getX(), (*it)->getY()), 1.5f, 10.0f, (*it)->getAttackDamage(), AttackController::Type::e_range, (vel.scale(0.25)).rotate((play_p - en_p).getAngle()), _timer, GLUTTON_ATTACK, GLUTTON_FRAMES);
                _sound->play_enemy_sound(SoundController::enemy::glutton, SoundController::etype::attack);
            }
        }
        if (std::shared_ptr<Mirror> mirror = dynamic_pointer_cast<Mirror>(*it))
        {
            if (!mirror->isRemoved() && mirror->getLinkedEnemy() == nullptr)
            {
                mirror->setLinkedEnemy(getNearestNonMirror(mirror->getPosition()));
                if (mirror->getLinkedEnemy() == nullptr)
                {
                    mirror->setHurt();
                    mirror->markRemoved(true);
                    //_sound->play_death_sound(true);
                }
            }
        }
    }
    
    // update spawners
    if (_spawnerCount)
    {
        if (_collider.getSpawnerKilled() != -1)
        {
            _living_spawners[_collider.getSpawnerKilled()] = 0;
            _spawnerCount--;
            _collider.setSpawnerKilled(-1);
        }
        if (_collider.getIndexSpawner() != -1)
        {
            int i = _collider.getIndexSpawner();
            string name = _collider.getSpawnerEnemyName();
            std::transform(name.begin(), name.end(), name.begin(),
                           [](unsigned char c)
                           { return std::tolower(c); });
            _spawner_enemy_types[i][name].current_count = _spawner_enemy_types[i][name].current_count - 1;
            _collider.setIndexSpawner(-1);
        }
    }
}

std::vector<std::shared_ptr<Texture> > GameScene::getTexturesFromNumber(int num)
{
    std::vector<std::shared_ptr<Texture> > nums;

    while (num > 0)
    {
        int digit = num % 10;
        nums.push_back(_numberTextures[digit]);
        num = num / 10;
    }

    return nums;
}

void GameScene::createParticles(std::shared_ptr<Texture> texture, Vec2 pos, string poolName, Color4 tint, Vec2 pointOffset, float scale)
{
    std::shared_ptr<ParticleNode> pn;
    std::shared_ptr<ParticlePool> pool = ParticlePool::allocPoint(_particleInfo->get(poolName), pointOffset);
    pn = ParticleNode::alloc(pos, texture, pool);
    pn->setScale(scale);
    pn->setColor(tint);
    _worldnode->addChildWithTag(pn, 100);
}

void GameScene::createParticles(std::vector<std::shared_ptr<Texture> > textures, Vec2 pos, string poolName, Color4 tint, Vec2 pointOffset, float scale, bool hasMultipleLinkedTextures, Vec2 linkOffset, int numTex)
{
    std::shared_ptr<ParticleNode> pn;
    std::shared_ptr<ParticlePool> pool;
    if (!hasMultipleLinkedTextures) {
        pool = ParticlePool::allocPoint(_particleInfo->get(poolName), pointOffset, numTex);
    }
    else {
        pool = ParticlePool::allocPoint(_particleInfo->get(poolName), pointOffset);
    }
    pn = ParticleNode::alloc(pos, textures, pool, hasMultipleLinkedTextures, linkOffset);
    pn->setScale(scale);
    pn->setColor(tint);
    _worldnode->addChildWithTag(pn, 100);
}

SwipeController::SwipeAttack GameScene::updateLeftSwipe(int unlockCount)
{
    SwipeController::SwipeAttack left = _swipes.getLeftSwipe();

    switch (left)
    {
    case SwipeController::upAttack:
    case SwipeController::rightAttack:
    case SwipeController::downAttack:
    case SwipeController::leftAttack:
        if (unlockCount < 1)
        {
            left = SwipeController::noAttack;
        }
        if (_tutorial == 3 && (_tutorialInd == 0 || _tutorialInd == 1)) {
            _tutorialActionDone = true;
        }
        break;
    case SwipeController::chargedUp:
        if (_tutorial == 4 && _tutorialInd == 0) {
            _tutorialActionDone = true;
        }
        switch (unlockCount)
        {
        case 0:
            left = SwipeController::noAttack;
            break;
        case 1:
            left = SwipeController::upAttack;
            break;
        default:
            break;
        }
        break;
    case SwipeController::chargedRight:
        if (_tutorial == 4 && _tutorialInd == 0) {
            _tutorialActionDone = true;
        }
        switch (unlockCount)
        {
        case 0:
            left = SwipeController::noAttack;
            break;
        case 1:
            left = SwipeController::rightAttack;
            break;
        default:
            break;
        }
        break;
    case SwipeController::chargedDown:
        if (_tutorial == 4 && _tutorialInd == 0) {
            _tutorialActionDone = true;
        }
        switch (unlockCount)
        {
        case 0:
            left = SwipeController::noAttack;
            break;
        case 1:
            left = SwipeController::downAttack;
            break;
        default:
            break;
        }
        break;
    case SwipeController::chargedLeft:
        if (_tutorial == 4 && _tutorialInd == 0) {
            _tutorialActionDone = true;
        }
        switch (unlockCount)
        {
        case 0:
            left = SwipeController::noAttack;
            break;
        case 1:
            left = SwipeController::leftAttack;
            break;
        default:
            break;
        }
        break;
    case SwipeController::chargedNortheast:
    case SwipeController::chargedSoutheast:
    case SwipeController::chargedNorthwest:
    case SwipeController::chargedSouthwest:
        if (_tutorial == 4 && _tutorialInd == 0) {
            _tutorialActionDone = true;
        }
        break;
    default:
        break;
    }

    return left;
}

SwipeController::SwipeAttack GameScene::updateRightSwipe(int unlockCount)
{
    SwipeController::SwipeAttack right = _swipes.getRightSwipe();

    switch (right)
    {
    case SwipeController::rightAttack:
    case SwipeController::leftAttack:
        if (_tutorial == 1 && _tutorialInd == 1) {
            _tutorialActionDone = true;
        }
        break;
    case SwipeController::chargedUp:
        if (_tutorial == 5 && _tutorialInd == 0) {
            _tutorialActionDone = true;
        }
        if (unlockCount < 4)
        {
            right = SwipeController::upAttack;
        }
        break;
    case SwipeController::chargedRight:
        if (_tutorial == 5 && _tutorialInd == 0) {
            _tutorialActionDone = true;
        }
        if (unlockCount < 4)
        {
            right = SwipeController::rightAttack;
        }
        break;
    case SwipeController::chargedDown:
        if (_tutorial == 5 && _tutorialInd == 0) {
            _tutorialActionDone = true;
        }
        if (unlockCount < 4)
        {
            right = SwipeController::downAttack;
        }
        break;
    case SwipeController::chargedLeft:
        if (_tutorial == 5 && _tutorialInd == 0) {
            _tutorialActionDone = true;
        }
        if (unlockCount < 4)
        {
            right = SwipeController::leftAttack;
        }
        break;
    case SwipeController::chargedNortheast:
    case SwipeController::chargedSoutheast:
    case SwipeController::chargedNorthwest:
    case SwipeController::chargedSouthwest:
        if (_tutorial == 5 && _tutorialInd == 0) {
            _tutorialActionDone = true;
        }
    default:
        break;
    }

    return right;
}

void GameScene::updateAttacks(float timestep, int unlockCount, SwipeController::SwipeAttack left, SwipeController::SwipeAttack right)
{
    // if player is stunned, do not read swipe input
    float xPos = _tilt.getXpos();

    b2Vec2 playerPos = _player->getBody()->GetPosition();

    if (!_player->isStunned())
    {
        _attacks->attackLeft(Vec2(playerPos.x, playerPos.y), left, _swipes.getLeftAngle(), _player->isGrounded(), _timer, _sound);
        if (left == SwipeController::chargedLeft || left == SwipeController::chargedRight || left == SwipeController::chargedUp || left == SwipeController::chargedDown) {
            _player->setIsChargeFiring(true);
        }
        _attacks->attackRight(Vec2(playerPos.x, playerPos.y), right, _swipes.getRightAngle(), _player->isGrounded(), _player->isFacingRight(), _timer, _sound);
        if (right == SwipeController::chargedRight)
        {
            _dashXVel = DASHX + 3;
            _dashYVel = 1;
            _dashTime = 0;
            _player->setIsDashing(true);
            _player->setDashingLastFrame(false);
            _player->setDashAngle(0);
            _dashDir = right;
        }
        else if (right == SwipeController::chargedLeft)
        {
            _dashXVel = -DASHX - 3;
            _dashYVel = 1;
            _dashTime = 0;
            _player->setIsDashing(true);
            _player->setDashingLastFrame(false);
            _player->setDashAngle(180);
            _dashDir = right;
        }
        else if (right == SwipeController::chargedUp)
        {
            _dashYVel = DASHY + 3;
            _dashTime = 0;
            _player->setIsDashing(true);
            _player->setDashingLastFrame(false);
            _player->setDashAngle(90);
            _dashDir = right;
        }
        else if (right == SwipeController::chargedDown)
        {
            _dashYVel = -DASHY - 3;
            _dashTime = 0;
            _player->setIsDashing(true);
            _player->setDashingLastFrame(false);
            _player->setDashAngle(270);
            _dashDir = right;
        }
        else if (right == SwipeController::chargedNortheast)
        {
            _dashYVel = DASHX;
            _dashXVel = DASHY;
            _dashTime = 0;
            _player->setIsDashing(true);
            _player->setDashingLastFrame(false);
            _player->setDashAngle(45);
            _dashDir = right;
        }
        else if (right == SwipeController::chargedNorthwest)
        {
            _dashYVel = DASHX;
            _dashXVel = -DASHY;
            _dashTime = 0;
            _player->setIsDashing(true);
            _player->setDashingLastFrame(false);
            _player->setDashAngle(135);
            _dashDir = right;
        }
        else if (right == SwipeController::chargedSouthwest)
        {
            _dashYVel = -DASHX;
            _dashXVel = -DASHY;
            _dashTime = 0;
            _player->setIsDashing(true);
            _player->setDashingLastFrame(false);
            _player->setDashAngle(225);
            _dashDir = right;
        }
        else if (right == SwipeController::chargedSoutheast)
        {
            _dashYVel = -DASHX;
            _dashXVel = DASHY;
            _dashTime = 0;
            _player->setIsDashing(true);
            _player->setDashingLastFrame(false);
            _player->setDashAngle(315);
            _dashDir = right;
        }
        // If the dash velocities are set, change player velocity if dash time is not complete
        if (_dashXVel || _dashYVel)
        {
            // Cancel dash with melee swipe
            if (right == SwipeController::rightAttack || right == SwipeController::upAttack ||
                right == SwipeController::leftAttack || right == SwipeController::downAttack ||
                right == SwipeController::jump || (_player->isFloored() && _player->getDashAngle() > 180)) {
                _dashXVel = 0;
                _dashYVel = 0;
                _player->setIsDashing(false);
                _player->setVX(0);
                _player->setVY(0);
                _dashTime = 0.7f;
                _cancelDash = true;
            }
            else {
                if (_dashTime < DASHTIME)
                {
                    // Slow down for last 0.25 seconds
                    float slowDownTime = DASHTIME - 0.25f;
                    if (_dashTime > slowDownTime && _dashXVel > 0)
                    {
                        _dashXVel = DASHX - ((_dashTime - slowDownTime) * (DASHX / 0.25));
                    }
                    else if (_dashTime > slowDownTime && _dashXVel < 0)
                    {
                        _dashXVel = -DASHX + ((_dashTime - slowDownTime) * (DASHX / 0.25));
                    }
                    if (_dashTime > slowDownTime && _dashYVel > 0 && _dashYVel != 1)
                    {
                        _dashYVel = DASHY - ((_dashTime - slowDownTime) * (DASHY / 0.25));
                    }
                    else if (_dashTime > slowDownTime && _dashYVel < 0 && _dashYVel != -1)
                    {
                        _dashYVel = -DASHY + ((_dashTime - slowDownTime) * (DASHY / 0.25));
                    }
                    
                    // Set velocity for x dash
                    if (_dashXVel > 0)
                    {
                        _player->setVX(_dashXVel);
                        _player->setFacingRight(true);
                    }
                    else if (_dashXVel < 0)
                    {
                        _player->setVX(_dashXVel);
                        _player->setFacingRight(false);
                    }
                    // Set velocity for y dash
                    if (_dashYVel > 0) {
                        _player->setVY(_dashYVel);
                    }
                    else if (_dashYVel < 0 && !_player->isGrounded())
                    {
                        _player->setVY(_dashYVel);
                    }
                    // Invincibility, maintain same health throughout dash
                    _player->setIsInvincible(true);
                    _dashTime += timestep;
                }
                else
                {
                    _dashXVel = 0;
                    _dashYVel = 0;
                    _player->setIsDashing(false);
                    _player->setVX(0);
                    _player->setVY(0);
                }
            }
        }
        else
        {
            // Flipping logic based on tilt
            if (xPos > 0)
            {
                _player->setFacingRight(true);
            }
            else if (xPos < 0)
            {
                _player->setFacingRight(false);
            }
            _player->setIsDashing(false);
        }
        if (_dashXVel == 0 && _dashYVel == 0 && _player->getInvincibilityTimer() <= 0)
        {
            _player->setIsInvincible(false);
        }
    }

    if (_player->getInvincibilityTimer() <= .5f)
    {
        _player->setIsStunned(false);
    }

    if (_player->getInvincibilityTimer() > 0 && _player->getPostStunnedInvincibilityTimer() >= 0.1 && !_player->isStunned())
    {
        //_player->getSceneNode()->setVisible(!_player->getSceneNode()->isVisible());
        int a = _player->getSceneNode()->getColor().a;
        if (a == 255)
        {
            a = 255 / 2;
        }
        else
        {
            a = 255;
        }
        _player->getSceneNode()->setColor(Color4(255, 255, 255, a));
        _player->setPostStunnedInvincibilityTimer(0);
    }

    if (_player->getInvincibilityTimer() < 0)
    {
        _player->getSceneNode()->setVisible(true);
        _player->getSceneNode()->setColor(Color4(255, 255, 255, 255));
    }

    if (_dashTime > 0 && _dashTime < 0.6f)
    {
        _player->setDropTime(timestep);
    }

    _player->setInvincibilityTimer(_player->getInvincibilityTimer() - timestep);
    _player->setPostStunnedInvincibilityTimer(_player->getPostStunnedInvincibilityTimer() + timestep);
    _world->update(timestep);

    for (auto it = _attacks->_pending.begin(); it != _attacks->_pending.end(); ++it)
    {

        AttackController::Type attackType = (*it)->getType();
        std::shared_ptr<scene2::PolygonNode> attackSprite;
        if (attackType == AttackController::Type::p_range)
        {
            std::shared_ptr<Texture> attackTexture = _assets->get<Texture>(PLAYER_RANGE);
            attackSprite = scene2::SpriteNode::alloc(attackTexture, 1, 1); // this is wrong for sprite sheet
            attackSprite->setScale(.85f * (*it)->getRadius());
            attackSprite->setAngle((*it)->getAngle() * M_PI / 180);
            attackSprite->setPriority(3);
            _rangedArm->setLastType(Glow::MeleeState::first);
            _player->setRangedAttackRight(_player->isFacingRight());
            if (left == SwipeController::downAttack)
            {
                _rangedArm->setAttackAngle(270);
            }
            else
            {
                _rangedArm->setAttackAngle((*it)->getAngle());
            }
            if (_player->isFacingRight())
            {
                _rangedArm->setAttackAngle(fmod(_rangedArm->getAttackAngle() + 180, 360));
            }
        }
        else if (attackType == AttackController::Type::p_exp_package)
        {
            std::shared_ptr<Texture> attackTexture = _assets->get<Texture>(PLAYER_EXP_PKG);
            attackSprite = scene2::SpriteNode::alloc(attackTexture, 1, 5);
            attackSprite->setAnchor(0.5, 0.5);
            attackSprite->setScale(.10f * (*it)->getRadius());
            dynamic_pointer_cast<scene2::SpriteNode>(attackSprite)->setFrame(0);
            attackSprite->setAngle((*it)->getAngle() * M_PI / 180);
            attackSprite->setPriority(3);
            _rangedArmCharge->setLastType(Glow::MeleeState::first);
            _player->setRangedAttackRight(_player->isFacingRight());
            if (left == SwipeController::downAttack)
            {
                _rangedArmCharge->setAttackAngle(270);
            }
            else
            {
                _rangedArmCharge->setAttackAngle((*it)->getAngle());
            }
            if (_player->isFacingRight())
            {
                _rangedArmCharge->setAttackAngle(fmod(_rangedArmCharge->getAttackAngle() + 180, 360));
            }
        }
        else if (attackType == AttackController::Type::p_exp)
        {
            std::shared_ptr<Texture> attackTexture = _assets->get<Texture>("player_explosion");
            attackSprite = scene2::SpriteNode::alloc(attackTexture, 1, 6);
            attackSprite->setAnchor(0.5, 0.5);
            attackSprite->setScale(.25f * (*it)->getRadius());
            dynamic_pointer_cast<scene2::SpriteNode>(attackSprite)->setFrame(0);
            attackSprite->setPriority(3);
        }
        else if (attackType == AttackController::Type::p_melee)
        {
            AttackController::MeleeState meleeState = (*it)->getMeleeState();

            attackSprite = scene2::PolygonNode::allocWithTexture(_pMeleeTexture);
            attackSprite->setVisible(false);
            attackSprite->setPriority(3);
            // it just works
            switch (meleeState)
            {
            case (AttackController::MeleeState::cool):
                _meleeArm->setLastType(Glow::MeleeState::cool);
                break;
            case (AttackController::MeleeState::first):
                _meleeArm->setLastType(Glow::MeleeState::first);
                break;
            case (AttackController::MeleeState::jump_attack):
                _meleeArm->setLastType(Glow::MeleeState::jump_attack);
                break;
            case (AttackController::MeleeState::h1_left):
                _meleeArm->setLastType(Glow::MeleeState::h1_left);
                break;
            case (AttackController::MeleeState::h2_left):
                _meleeArm->setLastType(Glow::MeleeState::h2_left);
                break;
            case (AttackController::MeleeState::h3_left):
                _meleeArm->setLastType(Glow::MeleeState::h3_left);
                break;
            case (AttackController::MeleeState::h1_right):
                _meleeArm->setLastType(Glow::MeleeState::h1_right);
                break;
            case (AttackController::MeleeState::h2_right):
                _meleeArm->setLastType(Glow::MeleeState::h2_right);
                break;
            case (AttackController::MeleeState::h3_right):
                _meleeArm->setLastType(Glow::MeleeState::h3_right);
                break;
            }
        }
        else if (attackType == AttackController::Type::e_range)
        {
            std::shared_ptr<Texture> attackTexture = _assets->get<Texture>((*it)->getAttackID());
            attackSprite = scene2::SpriteNode::alloc(attackTexture, 1, (*it)->getFrames()); // need to replace with animated texture
            if ((*it)->getAttackID() == PLAYER_RANGE)
            {
                // this is mirrors
                attackSprite->setScale(.85f * (*it)->getRadius());
                attackSprite->setAngle((*it)->getAngle());
                attackSprite->setColor(Color4::GRAY);
                attackSprite->setPriority(2.1);
            }
            else if ((*it)->getAttackID() == PHANTOM_ATTACK)
            {
                attackSprite->setScale(0.3 * (*it)->getRadius());
                attackSprite->setAngle((*it)->getAngle() + M_PI / 2);
                attackSprite->setPriority(2.2);

                dynamic_pointer_cast<scene2::SpriteNode>(attackSprite)->setFrame(0);
            }
            else if ((*it)->getAttackID() == GLUTTON_ATTACK)
            {
                attackSprite->setScale(.25 * (*it)->getRadius());
                attackSprite->setAngle((*it)->getAngle() + M_PI);
                attackSprite->setPriority(2);
            }
            else if ((*it)->getAttackID() == "seed")
            {
                attackSprite->setScale(0.5 * (*it)->getRadius());
                // attackSprite->setAngle()
                attackSprite->setVisible(true);
                attackSprite->setPriority(2);
            }
        }
        else
        {
            attackSprite = scene2::PolygonNode::allocWithTexture(_pMeleeTexture);
            attackSprite->setVisible(false);
            attackSprite->setScale(.85f * (*it)->getRadius());
        }

        (*it)->setDebugColor(Color4::YELLOW);
        addObstacle((*it), attackSprite, true);
    }
    _attacks->update(_player->getPosition(), _player->getBody()->GetLinearVelocity(), timestep, _enemies);
    // DO NOT MOVE THE ABOVE LINE
    if (!_cancelDash && (right == SwipeController::upAttack || left == SwipeController::jump || right == SwipeController::jump))
    {
        _player->setJumping(true);
        if (_tutorial == 2 && _tutorialInd == 0) {
            if (left == SwipeController::jump || right == SwipeController::jump) {
                _tutorialActionDone = true;
            }
        }
        else if (_tutorial == 2 && _tutorialInd == 1) {
            if (right == SwipeController::upAttack) {
                _tutorialActionDone = true;
            }
        }
        _player->setIsFirstFrame(true);
        if (_player->isGrounded())
        {
            _player->setMovingUp(true);
            _player->setJumpAnimationTimer(0);
            if (right == SwipeController::upAttack) {
                _sound->play_player_sound(SoundController::playerSType::jumpAttack);
            } else {
                _sound->play_player_sound(SoundController::playerSType::jump);
            }
            
        }
    }
    else if (right == _swipes.downAttack)
    {
        // IDK
        _player->setDropTime(0.4f);
        if(_player->getY() >= 5 && _player->isGrounded() && _tutorial == 2 && _tutorialInd == 2) {
            _tutorialActionDone = true;
        }
    }
    else
    {
        _player->setJumping(false);
    }
    _player->applyForce();

    if (_player->getVY() < -.2 || _player->getVY() > .2)
    {
        _player->setGrounded(false);
        _player->setFloored(false);
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

    if (_player->getVY() < 0)
    {
        _player->setMovingUp(false);
        if (_meleeArm->getLastType() == Glow::MeleeState::jump_attack) {
            _meleeArm->setLastType(Glow::MeleeState::cool);
        }
    }
}

void GameScene::updateRemoveDeletedAttacks()
{
    // Remove attacks
    auto ait = _attacks->_current.begin();
    while (ait != _attacks->_current.end())
    {
        if ((*ait)->isRemoved())
        {
            // int log1 = _world->getObstacles().size();
            cugl::physics2::Obstacle *obj = dynamic_cast<cugl::physics2::Obstacle *>(&**ait);
            _world->removeObstacle(obj);
            _worldnode2->removeChild(obj->_node);

            // int log2 = _world->getObstacles().size();
            ait = _attacks->_current.erase(ait);
        }
        // Delete dash attack if dash cancelled
        else if ((*ait)->getType() == AttackController::Type::p_dash && _cancelDash) {
            cugl::physics2::Obstacle *obj = dynamic_cast<cugl::physics2::Obstacle *>(&**ait);
            _world->removeObstacle(obj);
            _worldnode2->removeChild(obj->_node);

            ait = _attacks->_current.erase(ait);
            _cancelDash = false;
        }
        else
        {
            ait++;
        }
    }
}

void GameScene::updateRemoveDeletedEnemies()
{
    // Remove enemies
    auto eit = _enemies.begin();
    while (eit != _enemies.end())
    {
        bool bypass = false;
        if (std::shared_ptr<Mirror> mirror = dynamic_pointer_cast<Mirror>(*eit))
        {
            if (mirror->isHurt())
            {
                bypass = true; // don't remove until after hurt animation
            }
        }
        if (!bypass && (*eit)->isRemoved())
        {

            float damageParticleScale;
            if ((*eit)->getName() == "Spawner") {
                damageParticleScale = 0.15;
            }
            else if ((*eit)->getName() == "Glutton") {
                damageParticleScale = 0.2;
            }
            else {
                damageParticleScale = 0.1;
            }
            //Plays damage particles then death particles shortly after (they fade-in on a delay)
            if ((*eit)->getLastDamagedBy() == BaseEnemyModel::AttackType::p_melee || (*eit)->getLastDamagedBy() == BaseEnemyModel::AttackType::p_dash) {
                createParticles(_assets->get<Texture>("melee_impact"), (*eit)->getPosition() * _scale, "devil", Color4::WHITE, Vec2(0, 0), damageParticleScale);
                createParticles(_meleeParticleList, (*eit)->getPosition() * _scale, "sparks", Color4::WHITE, Vec2(0, 0), damageParticleScale*2, false, Vec2(), 7);
            }
            else {
                createParticles(_assets->get<Texture>("ranged_impact"), (*eit)->getPosition() * _scale, "devil", Color4::WHITE, Vec2(0, 0), damageParticleScale);
                createParticles(_rangeParticleList, (*eit)->getPosition() * _scale, "sparks", Color4::WHITE, Vec2(0, 0), damageParticleScale*2, false, Vec2(), 7);
            }
            
            if ((*eit)->getLastDamageAmount() < 10)
            {
                std::vector<std::shared_ptr<Texture> > num;
                num.push_back(_numberTextures[(*eit)->getLastDamageAmount()]);
                createParticles(num, (*eit)->getPosition() * _scale, "number", Color4::WHITE, Vec2(0, 10), 0.1f, true, Vec2(), 0);
            }
            else
            {
                std::vector<std::shared_ptr<Texture> > num = getTexturesFromNumber((*eit)->getLastDamageAmount());
                createParticles(num, (*eit)->getPosition() * _scale, "number", Color4::WHITE, Vec2(0, 10), 0.1f, true, Vec2(-10, 0), 0);
            }

            //Mirror shard breaking
            if (std::shared_ptr<Mirror> mirror = dynamic_pointer_cast<Mirror>(*eit)) {
                createParticles(_mirrorShardList, (*eit)->getPosition() * _scale, "mirror_death", Color4::WHITE, Vec2(0, 10), 0.05f, false, Vec2(), 6);
                _sound->play_death_sound(true);
            }
            else if (std::shared_ptr<Lost> lost = dynamic_pointer_cast<Lost>(*eit)) {
                createAndAddDeathAnimationObstacle("lost_death", (*eit)->getPosition(), 0.125f, 5, 200);
                createParticles(_deathParticleList, (*eit)->getPosition() * _scale, "lost_death", Color4::WHITE, Vec2(0, -20), 0.15f, false, Vec2(), 4);
            }
            else if (std::shared_ptr<Phantom> phantom = dynamic_pointer_cast<Phantom>(*eit)) {
                createAndAddDeathAnimationObstacle("phantom_death", (*eit)->getPosition(), 0.2f, 6, 201);
                createParticles(_deathParticleList, (*eit)->getPosition() * _scale, "lost_death", Color4::WHITE, Vec2(0, -20), 0.25f, false, Vec2(), 4);
            } 
            else if (std::shared_ptr<Glutton> glutton = dynamic_pointer_cast<Glutton>(*eit)) {
                createAndAddDeathAnimationObstacle("glutton_death", (*eit)->getPosition(), 0.2f, 5, 202);
                createParticles(_deathParticleList, (*eit)->getPosition() * _scale, "big_death", Color4::WHITE, Vec2(0, -20), 0.4f, false, Vec2(), 4);
            }
            else if (std::shared_ptr<Seeker> seeker = dynamic_pointer_cast<Seeker>(*eit)) {
                createAndAddDeathAnimationObstacle("seeker_death", (*eit)->getPosition(), 0.125f, 6, 203);
                createParticles(_deathParticleList, (*eit)->getPosition() * _scale, "lost_death", Color4::WHITE, Vec2(0, -20), 0.35f, false, Vec2(), 4);
            }
            else if (std::shared_ptr<Spawner> spawner = dynamic_pointer_cast<Spawner>(*eit)) {
                createParticles(_deathParticleList, (*eit)->getPosition() * _scale, "big_death", Color4::WHITE, Vec2(0, -20), 0.35f, false, Vec2(), 4);
            }
            
            // int log1 = _world->getObstacles().size();
            cugl::physics2::Obstacle *glowObj = dynamic_cast<cugl::physics2::Obstacle *>(&*(*eit)->getGlow());
            cugl::physics2::Obstacle *obj = dynamic_cast<cugl::physics2::Obstacle *>(&**eit);
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
}

void GameScene::createAndAddDeathAnimationObstacle(string textureName, Vec2 startPos, float scale, int frames, int tag) {
    std::shared_ptr<Texture> image = _assets->get<Texture>(textureName);
    std::shared_ptr<Glow> glow = Glow::alloc(startPos, image->getSize() / _scale, _scale);
    std::shared_ptr<scene2::SpriteNode> sprite = scene2::SpriteNode::alloc(image, 1, frames);
    glow->setSceneNode(sprite);
    glow->setAnimeTimer(0.f);
    glow->setGlowTimer(0.f);
    sprite->setPriority(1.29);
    sprite->setFrame(0);
    sprite->setRelativeColor(false);
    sprite->setScale(scale);
    _worldnode->addChildWithTag(glow->getSceneNode(), tag);
}


void GameScene::updateText()
{
    _text->setText(strtool::format("Wave: %d / %d", _nextWaveNum, _numWaves));
    _text->layout();

    int duration = _nextWaveNum < _spawn_times.size() ? (int)_spawn_times[_nextWaveNum] - (int)_timer : -1;
    _timer_text->setText(strtool::format("Next Wave In: %d", duration > 0 ? duration : 0));
    _timer_text->layout();
}

/**
 Updates the timer if all enemies are killed in this wave.
 If the next wave is within three seconds when all the enemies are killed, does nothing.
 Requires: spawns enemies of wave i after three seconds from _spawn_times[i]
 */
void GameScene::updateSpawnTimes()
{
    
    if (_nextWaveNum < _numWaves && !_enemies.size())
    {
        float nextSpawnTime = _spawn_times[_nextWaveNum];
        float nextTime = nextSpawnTime - 3;
        _timer = _timer > nextTime ? _timer : nextTime;
    }
}

void GameScene::updateRemoveDeletedPlayer()
{
    if (_player->isRemoved())
    {
        _lose = true;
        _player->getSceneNode()->setVisible(false);
        _rangedArm->getSceneNode()->setVisible(false);
        _meleeArm->getSceneNode()->setVisible(false);
        //        reset();
        //        _player->markRemoved(false);
    }
}

void GameScene::updateHUD(int unlockCount)
{
    // Update the health meter
    // left offset is additive, makes progress end at most leftOff*rightOff from left edge
    float leftOff = .2;
    // right offset is multiplicative, scales down the progress to make the right edge start at correct spot
    float rightOff = 0.74;
    float progress = _player->getHealth() / _player->getMaxHealth();
    float prog = (progress + leftOff) * rightOff;
    if (prog != _healthbar->getProgress())
    {
        _healthbar->setProgress(prog);
    }

    switch (unlockCount)
    {
        case 0:
        case 1:
            _range_charge->setVisible(false);
            _melee_charge->setVisible(false);
            _dmg2->setVisible(false);
            _dmg3->setVisible(false);
            break;
        case 2:
            _range_charge->setVisible(true);
            _melee_charge->setVisible(false);
            _dmg2->setVisible(false);
            _dmg3->setVisible(false);
            break;
        case 3:
            _range_charge->setVisible(true);
            _melee_charge->setVisible(false);
            _dmg2->setVisible(true);
            _dmg3->setVisible(false);
            break;
        case 4:
            _dmg2->setVisible(true);
            _dmg3->setVisible(false);
            _range_charge->setVisible(true);
            _melee_charge->setVisible(true);
            break;
        default:
            _range_charge->setVisible(true);
            _melee_charge->setVisible(true);
            _dmg2->setVisible(false);
            _dmg3->setVisible(true);
    }
    
    //Set wavebar progress
    float time = _timer / _spawn_times[_numWaves - 1];
    _wavebar->setProgress(time > 1 ? 1 : time);
    //_wavebar->setVisible(false);
    
    if (_chargeSoundCueM && (_swipes.getRightChargingTime() > 150) && !_swipes.hasRightChargedAttack()) {
        _sound->play_player_sound(SoundController::playerSType::charge);
        _chargeSoundCueM = false;
    } else if (!_chargeSoundCueM && _swipes.hasRightChargedAttack()) {
        _chargeSoundCueM = true;
    }
        
    if (_chargeSoundCueR && (_swipes.getLeftChargingTime() > 150) && !_swipes.hasLeftChargedAttack()) {
        _sound->play_player_sound(SoundController::playerSType::charge);
        _chargeSoundCueR = false;
    } else if (!_chargeSoundCueR && _swipes.hasLeftChargedAttack()) {
        _chargeSoundCueR = true;
    }
    
    _melee_charge->setProgress(_swipes.getMeleeCharge());
    _range_charge->setProgress(_swipes.getRangeCharge());
}

void GameScene::updateCamera()
{
    // Camera following player, with some non-linear smoothing
    float dy = getChild(0)->getContentSize().height / 2 - _worldnode->getPaneTransform().transform(_player->getSceneNode()->getPosition()).y;
    Vec2 pan = Vec2(0, dy);
    pan = pan * pan.length() / 3000;
    _worldnode->applyPan(pan);
    // Copy World's zoom and transform
    _debugnode->applyPan(-_debugnode->getPaneTransform().transform(Vec2()));
    _debugnode->applyPan(_worldnode->getPaneTransform().transform(Vec2()) / _scale);
}

void GameScene::updateSpawnEnemies(float timestep)
{
    // Spawn new enemies if time for next wave
    _timer += timestep;
    _spawnParticleTimer += timestep;
    if (_nextWaveNum < _numWaves && _timer >= _spawn_times[_nextWaveNum] - 3 && !_spawnParticlesDone)
    {
        for (std::shared_ptr<scene2::SceneNode> s : _worldnode2->getChildren()) {
            if (s->getTag() == 69) {
                s->removeFromParent();
            }
        }
        createSpawnParticles();
        _spawnParticlesDone = true;
        _spawnParticleTimer = 0.0f;
    }

    if (_nextWaveNum < _numWaves && _timer >= _spawn_times[_nextWaveNum])
    {
        createEnemies(_nextWaveNum);
        _nextWaveNum += 1;
        _spawnParticlesDone = false;
    }

    int index = 0;
    for (auto it = _spawner_enemy_types.begin(); it != _spawner_enemy_types.end(); ++it)
    {
        if (_living_spawners[index])
        {
            for (auto i : (*it))
            {
                string name = i.first;
                float timer = _spawner_enemy_types[index][name].timer;
                int diff_count = _spawner_enemy_types[index][name].max_count - _spawner_enemy_types[index][name].current_count;

                if (timer <= 0)
                {
                    while (diff_count != 0)
                    {
                        _spawners.at(index)->setSpawned(true);
                        createSpawnerEnemy(index, name);
                        _sound->play_enemy_sound(SoundController::enemy::spawner, SoundController::etype::attack);

                        _spawner_enemy_types[index][name].current_count++;
                        diff_count--;
                    }
                    _spawner_enemy_types[index][name].timer = 10.0f;
                }
                else if (diff_count > 0)
                {
                    _spawner_enemy_types[index][name].timer -= timestep;
                }
            }
        }
        index++;
    }
}

void GameScene::createSpawnParticles()
{
    std::vector<cugl::Vec2> positions;
    positions = _spawn_pos.at(_nextWaveNum);
    std::vector <std::string> enemyNames;
    enemyNames = _spawn_order.at(_nextWaveNum);
    std::shared_ptr<Texture> portal = _assets->get<Texture>("enemy_swirl");

    for (int i = 0; i < positions.size(); i++)
    {
        std::shared_ptr<scene2::PolygonNode> portalSprite = scene2::PolygonNode::allocWithTexture(portal);
        
        if (!enemyNames[i].compare("glutton")) {
            portalSprite->setPosition(Vec2(positions[i].x * _scale, (positions[i].y + 1.25) * _scale));
            portalSprite->setScale(0.9f);
            portalSprite->setPriority(0.96);
        }
        else if (!enemyNames[i].compare("spawner")) {
            portalSprite->setPosition(Vec2(positions[i].x * _scale, (positions[i].y + 0.85) * _scale));
            portalSprite->setScale(0.6f);
            portalSprite->setPriority(0.97);
        }
        else if (!enemyNames[i].compare("seeker")) {
            portalSprite->setPosition(Vec2(positions[i].x * _scale, (positions[i].y + 0.77) * _scale));
            portalSprite->setScale(0.45f);
            portalSprite->setPriority(0.98);
        }
        else {
            portalSprite->setPosition(Vec2(positions[i].x * _scale, (positions[i].y + 0.75) * _scale));
            portalSprite->setScale(0.35f);
            portalSprite->setPriority(0.99);
        }
        _worldnode2->addChildWithTag(portalSprite, 69);
    }
}

bool GameScene::updateWin()
{
    // All waves created and all enemies cleared
    if (_nextWaveNum >= _numWaves && !_enemies.size())
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::shared_ptr<BaseEnemyModel> GameScene::getNearestNonMirror(cugl::Vec2 pos)
{
    float distance(INT_MAX);
    std::shared_ptr<BaseEnemyModel> savedEnemy = nullptr;
    for (auto it = _enemies.begin(); it != _enemies.end(); ++it)
    {
        if (Mirror *mirror = dynamic_cast<Mirror *>((*it).get()))
        {
            // Do nothing, but need to see if it can be casted
        }
        else
        {
            if (pos.distance((*it)->getPosition()) <= distance)
            {
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
void GameScene::render(const std::shared_ptr<cugl::SpriteBatch> &batch)
{
    // This takes care of begin/end

    //_scene->render(batch);
    if (_player->isInvincible() && !_player->isStunned())
    {
    }

    if (_swipes.hasLeftChargedAttack())
    {
        int flip = 1;
        if (_player->isFacingRight())
        {
            flip = -1;
        }
        createParticles(_rangeParticleList, (_rangedArm->getPosition() - Vec2(1.25 * flip, 0))*_scale, "charged", Color4::BLUE, Vec2(0, 0), 0.2f, false, Vec2(), 7);
    }

    if (_swipes.hasRightChargedAttack())
    {
        int flip = 1;
        if (_player->isFacingRight())
        {
            flip = -1;
        }

        createParticles(_meleeParticleList, (_meleeArm->getPosition() - Vec2(-1.5 * flip, 0))*_scale, "charged", Color4::RED, Vec2(0, 0), 0.2f, false, Vec2(), 7);
    }

    Scene2::render(batch);
    batch->begin(getCamera()->getCombined());

    //_attacks.draw(batch);
    if (_debug){
        batch->drawText(_text, Vec2(getSize().width / 2 - _text->getBounds().size.width / 2, getSize().height - _text->getBounds().size.height - 10));
    
        if (_nextWaveNum < _spawn_times.size())
        batch->drawText(_timer_text, Vec2(getSize().width - _timer_text->getBounds().size.width - 20, getSize().height - _timer_text->getBounds().size.height - 50));
    }

    batch->setColor(Color4::GREEN);
    Affine2 trans;
    trans.scale(3);
    trans.translate(Vec2(getSize().width / 2, getSize().height / 2));

    batch->end();
}

void GameScene::createMirror(Vec2 enemyPos, Mirror::Type type, std::string assetName, std::shared_ptr<Glow> enemyGlow)
{
    std::shared_ptr<Texture> mirrorImage = _assets->get<Texture>(assetName);
    std::shared_ptr<Texture> mirrorHurtImage = _assets->get<Texture>(assetName + "_hurt");
    std::shared_ptr<Texture> mirror_reflectattackImage = _assets->get<Texture>(MIRROR_REFLECT_TEXTURE);
    // shards
    std::shared_ptr<scene2::PolygonNode> mirrorShards[6];
    mirrorShards[0] = scene2::PolygonNode::allocWithTexture(_mirrorShardList[0]);
    mirrorShards[1] = scene2::PolygonNode::allocWithTexture(_mirrorShardList[1]);
    mirrorShards[2] = scene2::PolygonNode::allocWithTexture(_mirrorShardList[2]);
    mirrorShards[3] = scene2::PolygonNode::allocWithTexture(_mirrorShardList[3]);
    mirrorShards[4] = scene2::PolygonNode::allocWithTexture(_mirrorShardList[4]);
    mirrorShards[5] = scene2::PolygonNode::allocWithTexture(_mirrorShardList[5]);

    std::shared_ptr<Mirror> mirror = Mirror::alloc(enemyPos, mirrorImage->getSize(), mirrorImage->getSize() / _scale / 15, _scale, type); // TODO this is not right, fix this to be closest enemy
    std::shared_ptr<scene2::PolygonNode> mirrorSprite = scene2::PolygonNode::allocWithTexture(mirrorImage);
    std::shared_ptr<scene2::PolygonNode> mirrorHurtSprite = scene2::PolygonNode::allocWithTexture(mirrorHurtImage);
    mirror->setGlow(enemyGlow);
    mirror->setAttackAnimationTimer(0);
    mirror->setHurtSprite(mirrorHurtSprite);
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
    // mirrorSprite->addChildWithName(mirrorShard1, "shard1");
    // mirrorSprite->addChildWithName(mirrorShard2, "shard2");
    // mirrorSprite->addChildWithName(mirrorShard3, "shard3");
    mirror->setDebugColor(Color4::BLUE);
    mirrorSprite->setScale(0.15f);
    mirrorSprite->setPriority(1.4);
    addObstacle(mirror, mirrorSprite, true);
    _enemies.push_back(mirror);
}

void GameScene::createSpawnerEnemy(int spawnerInd, string enemyName)
{
    Vec2 enemyPos;

    enemyPos = _spawner_pos[spawnerInd];
    // lowercase the enemyName
    std::transform(enemyName.begin(), enemyName.end(), enemyName.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });
    createEnemy(enemyName, enemyPos, spawnerInd);
}

void GameScene::createEnemy(string enemyName, Vec2 enemyPos, int spawnerInd) {

    std::shared_ptr<Texture> enemyGlowImage = _assets->get<Texture>(GLOW_TEXTURE);
    std::shared_ptr<Glow> enemyGlow = Glow::alloc(enemyPos, enemyGlowImage->getSize() / _scale, _scale);
    std::shared_ptr<scene2::PolygonNode> enemyGlowSprite = scene2::PolygonNode::allocWithTexture(enemyGlowImage);
    enemyGlow->setSceneNode(enemyGlowSprite);
    std::shared_ptr<Gradient> grad = Gradient::allocRadial(Color4(255, 255, 255, 85), Color4(111, 111, 111, 0), Vec2(0.5, 0.5), .2f);
    enemyGlowSprite->setGradient(grad);
    enemyGlowSprite->setRelativeColor(false);
    enemyGlowSprite->setScale(.65f);
    addObstacle(enemyGlow, enemyGlowSprite, true);
    if (!enemyName.compare("lost"))
    {
        std::shared_ptr<Texture> lostHitBoxImage = _assets->get<Texture>("lost");
        std::shared_ptr<Texture> lostImage = _assets->get<Texture>("lost_ani");
        std::shared_ptr<Lost> lost = Lost::alloc(enemyPos, Size(lostImage->getSize().width / 4.0f, lostImage->getSize().height / 3.0f), lostHitBoxImage->getSize() / _scale / 10, _scale);
        std::shared_ptr<scene2::SpriteNode> lostSprite = scene2::SpriteNode::alloc(lostImage, 3, 4);
        lostSprite->setFrame(0);
        lostSprite->setAnchor(Vec2(0.5, 0.25));
        lost->setGlow(enemyGlow);
        lost->setSceneNode(lostSprite);
        lost->setDebugColor(Color4::RED);
        lost->setPlayedDamagedParticle(false);
        if (spawnerInd > -1) {
            lost->setSpawnerInd(spawnerInd);
        }
        lostSprite->setScale(0.15f);
        lostSprite->setPriority(1.3);
        addObstacle(lost, lostSprite, true);
        _enemies.push_back(lost);
    }
    else if (!enemyName.compare("phantom"))
    {
        std::shared_ptr<Texture> phantomHitboxImage = _assets->get<Texture>("phantom");
        std::shared_ptr<Texture> phantomImage = _assets->get<Texture>("phantom_ani");
        std::shared_ptr<Phantom> phantom = Phantom::alloc(enemyPos, Vec2(phantomImage->getSize().width / 7, phantomImage->getSize().height/2), phantomHitboxImage->getSize() / _scale / 10, _scale);
        std::shared_ptr<scene2::SpriteNode> phantomSprite = scene2::SpriteNode::alloc(phantomImage, 2, 7);
        phantom->setSceneNode(phantomSprite);
        phantom->setDebugColor(Color4::BLUE);
        phantom->setGlow(enemyGlow);
        phantom->setPlayedDamagedParticle(false);
        if (spawnerInd > -1) {
            phantom->setSpawnerInd(spawnerInd);
        }
        phantomSprite->setScale(0.2f);
        phantomSprite->setFrame(0);
        phantomSprite->setPriority(1.2);
        addObstacle(phantom, phantomSprite, true);
        _enemies.push_back(phantom);
    }
    else if (!enemyName.compare("square"))
    {
        createMirror(enemyPos, Mirror::Type::square, "squaremirror", enemyGlow);
    }
    else if (!enemyName.compare("triangle"))
    {
        createMirror(enemyPos, Mirror::Type::triangle, "trianglemirror", enemyGlow);
    }
    else if (!enemyName.compare("circle"))
    {
        createMirror(enemyPos, Mirror::Type::circle, "circlemirror", enemyGlow);
    }
    else if (!enemyName.compare("seeker"))
    {
        std::shared_ptr<Texture> seekerHitboxImage = _assets->get<Texture>("seeker");
        std::shared_ptr<Texture> seekerImage = _assets->get<Texture>("seeker_ani");
        std::shared_ptr<Seeker> seeker = Seeker::alloc(enemyPos, seekerHitboxImage->getSize(), seekerHitboxImage->getSize() / _scale / 10, _scale);
        std::shared_ptr<scene2::SpriteNode> seekerSprite = scene2::SpriteNode::alloc(seekerImage, 3, 6);
        seeker->setSceneNode(seekerSprite);
        seeker->setDebugColor(Color4::GREEN);
        seeker->setGlow(enemyGlow);
        seeker->setPlayedDamagedParticle(false);
        if (spawnerInd > -1) {
            seeker->setSpawnerInd(spawnerInd);
        }
        seekerSprite->setFrame(0);
        seekerSprite->setScale(0.15f);
        seekerSprite->setPriority(1.1);
        addObstacle(seeker, seekerSprite, true);
        _enemies.push_back(seeker);
    }
    else if (!enemyName.compare("glutton"))
    {
        std::shared_ptr<Texture> gluttonHitboxImage = _assets->get<Texture>("glutton");
        std::shared_ptr<Texture> gluttonImage = _assets->get<Texture>("glutton_ani");
        std::shared_ptr<Glutton> glutton = Glutton::alloc(enemyPos + Vec2(0, 2), Vec2(gluttonImage->getSize().width / 7.0f, gluttonHitboxImage->getSize().height / 2.0f), gluttonHitboxImage->getSize() / _scale / 5, _scale);
        std::shared_ptr<scene2::SpriteNode> gluttonSprite = scene2::SpriteNode::alloc(gluttonImage, 4, 7);
        // fix the anchor slightly for glutton only
        gluttonSprite->setAnchor(.5, .4);
        glutton->setSceneNode(gluttonSprite);
        glutton->setDebugColor(Color4::BLUE);
        glutton->setGlow(enemyGlow);
        glutton->setPlayedDamagedParticle(false);
        if (spawnerInd > -1) {
            glutton->setSpawnerInd(spawnerInd);
        }
        gluttonSprite->setScale(0.2f);
        gluttonSprite->setFrame(0);
        gluttonSprite->setPriority(1);
        addObstacle(glutton, gluttonSprite, true);
        _enemies.push_back(glutton);
    }
    else if (!enemyName.compare("spawner"))
    {
        _spawner_ind++;
        _spawnerCount++;

        _spawner_pos.push_back(enemyPos);
        std::shared_ptr<Texture> spawnerHitBoxImage = _assets->get<Texture>("glutton");
        std::shared_ptr<Texture> spawnerImage = _assets->get<Texture>("spawner_ani");
        std::shared_ptr<Spawner> spawner = Spawner::alloc(enemyPos, Vec2(spawnerImage->getSize().width / 5.0f, spawnerImage->getSize().height / 5.0f), spawnerHitBoxImage->getSize() / _scale / 10, _scale);
        std::shared_ptr<scene2::SpriteNode> spawnerSprite = scene2::SpriteNode::alloc(spawnerImage, 5, 5);
        spawner->setSpawned(false);
        spawner->setSceneNode(spawnerSprite);
        spawner->setDebugColor(Color4::BLACK);
        spawner->setGlow(enemyGlow);
        spawner->setIndex(_spawner_ind);
        spawner->setPlayedDamagedParticle(false);
        spawnerSprite->setAnchor(0.5, 0.4);
        spawnerSprite->setScale(0.75f);
        spawnerSprite->setPriority(1.01);
        spawnerSprite->setFrame(0);
        addObstacle(spawner, spawnerSprite, true);
        _enemies.push_back(spawner);
        _spawners.push_back(spawner);
        auto spawnerEnemiesMap = _spawner_enemy_types.at(_spawner_ind);
        for (auto it = spawnerEnemiesMap.begin(); it != spawnerEnemiesMap.end(); ++it)
        {
            int index = it->second.max_count;
            string spawnerEnemyName = it->first;
            while (index != 0)
            {
                spawner->setSpawned(true);
                createSpawnerEnemy(_spawner_ind, spawnerEnemyName);
                _spawner_enemy_types[_spawner_ind][spawnerEnemyName].current_count++;
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

}

void GameScene::createEnemies(int wave)
{
    std::vector<string> enemies;
    std::vector<cugl::Vec2> positions;
    enemies = _spawn_order.at(wave);
    positions = _spawn_pos.at(wave);

    for (int i = 0; i < enemies.size(); i++)
    {
        Vec2 enemyPos;

        enemyPos = positions[i];

        std::string enemyName = enemies[i];

        // lowercase the enemyName
        std::transform(enemyName.begin(), enemyName.end(), enemyName.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });
        createEnemy(enemyName, enemyPos, -1);

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
    // std::shared_ptr<Texture> down = _assets->get<Texture>("close-selected");

    Size bsize = up->getSize();
    _pauseButton = scene2::Button::alloc(scene2::PolygonNode::allocWithTexture(up));
    _pauseButton->setScale(0.55);
    // button->setAnchor(Vec2(1, 1));
    // scene2::PolygonNode::allocWithTexture(down));

    // Create a callback function for the button
    _pauseButton->setName("pause");
    _pauseButton->addListener([=](const std::string &name, bool down)
                              {
            // Only quit when the button is released
            if (!down) {
                _pause = true;
            } });

    // Find the safe area, adapting to the iPhone X
    Rect safe = Application::get()->getSafeBounds();
    safe.origin *= scale;
    safe.size *= scale;

    // Get the right and bottom offsets.
    float bOffset = (size.height) - (safe.origin.y + safe.size.height);
    float rOffset = (size.width) - (safe.origin.x + safe.size.width);

    // Making the floor -jdg274
    Rect floorRect = Rect(0, 0, DEFAULT_WIDTH, 0.5);
    std::shared_ptr<physics2::PolygonObstacle> floor = physics2::PolygonObstacle::allocWithAnchor(floorRect, Vec2::ANCHOR_CENTER);
    floor->setBodyType(b2_staticBody);

    std::shared_ptr<scene2::PolygonNode> floorNode = scene2::PolygonNode::allocWithPoly(floorRect * _scale);
    floorNode->setColor(Color4::CLEAR);
    floor->setName("floor");
    b2Filter filter = b2Filter();
    filter.categoryBits = 0b1000;
    // filter.maskBits = 0b1100;
    floor->setFilterData(filter);
    addObstacle(floor, floorNode, 1);
    
    // Split floor into parts to repeat texture
    Rect safebounds = Application::get()->getSafeBounds();
    float safeWidth = safebounds.size.width;
    Rect screenbounds = Application::get()->getDisplayBounds();

    float worldCoorWidth = safeWidth / DEFAULT_WIDTH;
    float leftOffset = safebounds.getMinX() - screenbounds.getMinX();
    float leftWorldCoors = leftOffset / worldCoorWidth;
    float rightOffset = screenbounds.getMaxX() - safebounds.getMaxX();
    float rightWorldCoors = rightOffset / worldCoorWidth;
    float totalWorldCoors = leftWorldCoors + DEFAULT_WIDTH + rightWorldCoors;

    vector<float> positions;
    
    int split = 3;
    // width of platform in world coors
    float platformCoors = totalWorldCoors / split;
    if (!_biome.compare("shroom")) {
        split = 2;
        platformCoors = totalWorldCoors / split;
        
        // x positions of each platform to make floor fill entire screen
        float secondPos = platformCoors - leftWorldCoors;
        positions.push_back(0);
        positions.push_back(secondPos);
    }
    else if (!_biome.compare("forest")) {
        split = 8;
        platformCoors = totalWorldCoors / split;
        
        float secondPos = platformCoors - leftWorldCoors;
        positions.push_back(0);
        positions.push_back(secondPos);
        for (int i = 1; i < split - 1; i++) {
            positions.push_back(secondPos + (platformCoors * i));
        }
    }
    else {
        float secondPos = platformCoors - leftWorldCoors;
        float thirdPos = secondPos + platformCoors;
        
        positions.push_back(0);
        positions.push_back(secondPos);
        positions.push_back(thirdPos);
    }
    float leftAnchor = 1 - ((platformCoors * 0.5) - leftWorldCoors) / platformCoors;
    
    for (int i = 0; i < split; i++) {
        Rect floorRect = Rect(positions[i], 0, platformCoors, 0.5);
        std::shared_ptr<physics2::PolygonObstacle> floor = physics2::PolygonObstacle::allocWithAnchor(floorRect, Vec2::ANCHOR_CENTER);
        floor->setBodyType(b2_staticBody);

        std::shared_ptr<Texture> floorImage = _assets->get<Texture>("platform");
        if (!_biome.compare("cave"))
        {
            floorImage = _assets->get<Texture>("cave_floor");
        }
        else if (!_biome.compare("shroom"))
        {
            floorImage = _assets->get<Texture>("shroom_floor");
        }
        else if (!_biome.compare("forest"))
        {
            floorImage = _assets->get<Texture>("forest_floor");
        }
        std::shared_ptr<scene2::PolygonNode> floorSprite = scene2::PolygonNode::allocWithTexture(floorImage);
        float desiredWidth =  totalWorldCoors * _scale;
        float floorScale = desiredWidth / floorSprite->getWidth() / split;
        floorSprite->setScale(floorScale);
        floorSprite->setPriority(0.11);
        float xAnchor = 0.5;
        if (i == 0) {
            xAnchor = leftAnchor;
        }
        if (!_biome.compare("shroom")) {
            floorSprite->setAnchor(xAnchor, 0.45);
        } else {
            floorSprite->setAnchor(xAnchor, 0.5);
        }
        addObstacle(floor, floorSprite, 1);
    }

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
    left->setName("leftwall");
    left->setFilterData(filter);

    std::shared_ptr<scene2::PolygonNode> leftNode = scene2::PolygonNode::allocWithPoly(leftRect * _scale);
    leftNode->setColor(Color4::CLEAR);
    addObstacle(left, leftNode, 1);

    // Making the right wall -jdg274
    Rect rightRect = Rect(DEFAULT_WIDTH - 0.5, 0, 0.5, DEFAULT_HEIGHT);
    std::shared_ptr<physics2::PolygonObstacle> right = physics2::PolygonObstacle::allocWithAnchor(rightRect, Vec2::ANCHOR_CENTER);
    right->setBodyType(b2_staticBody);
    right->setName("rightwall");
    right->setFilterData(filter);
    std::shared_ptr<scene2::PolygonNode> rightNode = scene2::PolygonNode::allocWithPoly(rightRect * _scale);
    rightNode->setColor(Color4::CLEAR);
    addObstacle(right, rightNode, 1);

    // Position the button in the bottom right corner
    _pauseButton->setAnchor(Vec2(0, 1));
    _pauseButton->setPosition(size.width - (bsize.width + rOffset) / 2, size.height);// - (bsize.height + bOffset) / 2);

    // Add platforms to the world
    Vec2 pos;
    Rect platRect;
    std::shared_ptr<PlatformModel> platform;
    std::shared_ptr<scene2::PolygonNode> platformSprite;
    // putting this default to see if that fixes platforms being occasionally invisible
    std::shared_ptr<Texture> platformImage = _assets->get<Texture>("platform");
    ;
    for (int i = 0; i < _platforms_attr.size(); i++)
    {
        pos.x = _platforms_attr[i][0];
        pos.y = _platforms_attr[i][1];
        float width = _platforms_attr[i][2];
        float yAnchor = 0.9;
        if (!_biome.compare("shroom"))
        {
            if (width < DEFAULT_WIDTH / 6)
            {
                // use smallest platform
                platformImage = _assets->get<Texture>("shroom_1_platform");
            }
            else if (width < (DEFAULT_WIDTH / 6) * 2)
            {
                platformImage = _assets->get<Texture>("shroom_2_platform");
                yAnchor = 0.94;
            }
            else if (width < (DEFAULT_WIDTH / 6) * 3)
            {
                platformImage = _assets->get<Texture>("shroom_3_platform");
            }
            else if (width < (DEFAULT_WIDTH / 6) * 4)
            {
                yAnchor = 0.97;
                platformImage = _assets->get<Texture>("shroom_4_platform");
            }
            else if (width < (DEFAULT_WIDTH / 6) * 5)
            {
                yAnchor = 0.97;
                platformImage = _assets->get<Texture>("shroom_5_platform");
            }
            else
            {
                // use largest platform
                yAnchor = 0.97;
                platformImage = _assets->get<Texture>("shroom_6_platform");
            }
        }
        else if (!_biome.compare("forest"))
        {
            if (width < DEFAULT_WIDTH / 3)
            {
                platformImage = _assets->get<Texture>("forest_small_platform");
                yAnchor = 0.9;
            }
            else if (width < (DEFAULT_WIDTH / 3) * 2)
            {
                platformImage = _assets->get<Texture>("forest_medium_platform");
                yAnchor = 0.9;
            }
            else
            {
                platformImage = _assets->get<Texture>("forest_large_platform");
                yAnchor = 0.93;
            }
        }
        else
        {
            if (width < DEFAULT_WIDTH / 3)
            {
                platformImage = _assets->get<Texture>("cave_small_platform");
                yAnchor = 0.98;
            }
            else if (width < (DEFAULT_WIDTH / 3) * 2)
            {
                platformImage = _assets->get<Texture>("cave_medium_platform");
                yAnchor = 0.98;
            }
            else
            {
                platformImage = _assets->get<Texture>("cave_large_platform");
                yAnchor = 0.99;
            }
        }
        platformSprite = scene2::PolygonNode::allocWithTexture(platformImage);
        float desiredWidth = width * _scale;
        float scale = desiredWidth / platformSprite->getWidth();
        platformSprite->setScale(scale);
        platformSprite->setAnchor(0.5, yAnchor);
        platform = PlatformModel::alloc(pos, width, PLATFORM_HEIGHT, _scale);
        _platforms.push_back(platform);
        _platformNodes.push_back(platformSprite);
        platform->setName("platform");
        platform->setSceneNode(_platformNodes[i]);
        platform->setDebugColor(Color4::RED);
        platformSprite->setPriority(0.1);
        addObstacle(platform, platformSprite, true);
    }

    float xBackgroundAnchor = (leftWorldCoors / totalWorldCoors);
    //CULog("%f", xBackgroundAnchor);
    if (!_biome.compare("cave")) {
        Vec2 test_pos = Vec2(0, 0);
        std::shared_ptr<Texture> backgroundImage = _assets->get<Texture>("cave_background");
        std::shared_ptr<Glow> testBackground = Glow::alloc(test_pos, backgroundImage->getSize() / _scale, _scale);
        std::shared_ptr<scene2::PolygonNode> bSprite = scene2::PolygonNode::allocWithTexture(backgroundImage);
        bSprite->setAnchor(xBackgroundAnchor, 0);
        testBackground->setSceneNode(bSprite);
        bSprite->setPosition(testBackground->getPosition() * _scale);
        bSprite->setScale(0.7 * _scale / 32);
        bSprite->setPriority(0.01);
        _worldnode2->addChildWithTag(bSprite, 300);
    }
    else if (!_biome.compare("shroom")) {
        Vec2 test_pos = Vec2(0, 0);
        std::shared_ptr<Texture> backgroundImage = _assets->get<Texture>("shroom_background");
        std::shared_ptr<Glow> testBackground = Glow::alloc(test_pos, backgroundImage->getSize() / _scale, _scale);
        std::shared_ptr<scene2::PolygonNode> bSprite = scene2::PolygonNode::allocWithTexture(backgroundImage);
        bSprite->setAnchor(xBackgroundAnchor, 0);
        testBackground->setSceneNode(bSprite);
        bSprite->setPosition(testBackground->getPosition() * _scale);
        bSprite->setScale(0.6 * _scale/32);
        bSprite->setPriority(0.01);
        _worldnode2->addChildWithTag(bSprite, 300);
    }
    else {
        Vec2 test_pos = Vec2(0, 0);
        std::shared_ptr<Texture> backgroundImage = _assets->get<Texture>("forest_background");
        std::shared_ptr<Glow> testBackground = Glow::alloc(test_pos, backgroundImage->getSize() / _scale, _scale);
        std::shared_ptr<scene2::PolygonNode> bSprite = scene2::PolygonNode::allocWithTexture(backgroundImage);
        bSprite->setAnchor(xBackgroundAnchor, 0);
        testBackground->setSceneNode(bSprite);
        bSprite->setPosition(testBackground->getPosition() * _scale);
        bSprite->setScale(0.6 * _scale / 32);
        bSprite->setPriority(0.01);
        _worldnode2->addChildWithTag(bSprite, 300);
    }

    // Add the logo and button to the scene graph
    scene->addChildWithName(_pauseButton, "pauseButton");

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
    std::shared_ptr<scene2::SpriteNode> sprite = scene2::SpriteNode::alloc(image, 5, 8);
    sprite->setFrame(12);
    _prevFrame = 12;
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
    _rangedArm->setLastType(Glow::MeleeState::cool);
    std::shared_ptr<scene2::SpriteNode> rangeArmSprite = scene2::SpriteNode::alloc(rangeImage, 2, 5);
    _rangedArm->setSceneNode(rangeArmSprite);
    _rangedArm->getSceneNode()->setVisible(false);
    rangeArmSprite->setFrame(0);
    rangeArmSprite->setScale(0.22);
    rangeArmSprite->setPriority(5);
    addObstacle(_rangedArm, rangeArmSprite, true);

    // Ranged Arm for the charge shot for player
    std::shared_ptr<Texture> rangeChargeHitboxImage = _assets->get<Texture>(PLAYER_RANGE_TEXTURE);
    std::shared_ptr<Texture> rangeChargeImage = _assets->get<Texture>("player_range_arm_charge");
    _rangedArmCharge = Glow::alloc(rangeArmPos, rangeChargeHitboxImage->getSize() / _scale, _scale);
    _rangedArmCharge->setAttackAngle(0);
    _rangedArmCharge->setGlowTimer(0);
    _rangedArmCharge->setAnimeTimer(0);
    _rangedArmCharge->setLastType(Glow::MeleeState::cool);
    std::shared_ptr<scene2::SpriteNode> rangeChargeSprite = scene2::SpriteNode::alloc(rangeChargeImage, 1, 8);
    _rangedArmCharge->setSceneNode(rangeChargeSprite);
    _rangedArmCharge->getSceneNode()->setVisible(false);
    rangeChargeSprite->setFrame(0);
    rangeChargeSprite->setScale(0.22);
    rangeChargeSprite->setPriority(5);
    addObstacle(_rangedArmCharge, rangeChargeSprite, true);
    
    // Melee Arm for the player
    Vec2 meleeArmPos = PLAYER_POS;
    std::shared_ptr<Texture> meleeHitboxImage = _assets->get<Texture>(PLAYER_MELEE_TEXTURE);
    std::shared_ptr<Texture> meleeImage = _assets->get<Texture>(PLAYER_MELEE_THREE_TEXTURE);
    _meleeArm = Glow::alloc(meleeArmPos, meleeHitboxImage->getSize() / _scale, _scale);
    _meleeArm->setAttackAngle(0);
    _meleeArm->setGlowTimer(0);
    _meleeArm->setLastType(Glow::MeleeState::cool);
    std::shared_ptr<scene2::SpriteNode> meleeArmSprite = scene2::SpriteNode::alloc(meleeImage, 4, 7);
    meleeArmSprite->setFrame(21);
    _meleeArm->setSceneNode(meleeArmSprite);
    _meleeArm->setAnimeTimer(0);
    meleeArmSprite->setScale(0.36);
    meleeArmSprite->setPriority(6);
    addObstacle(_meleeArm, meleeArmSprite, true);

    // Melee Arm for the dash for player
    std::shared_ptr<Texture> meleeDashHitboxImage = _assets->get<Texture>(PLAYER_MELEE_TEXTURE);
    std::shared_ptr<Texture> meleeDashImage = _assets->get<Texture>("player_melee_dash");
    _meleeArmDash = Glow::alloc(meleeArmPos, meleeDashHitboxImage->getSize() / _scale, _scale);
    _meleeArmDash->setAttackAngle(0);
    _meleeArmDash->setGlowTimer(0);
    _meleeArmDash->setLastType(Glow::MeleeState::cool);
    std::shared_ptr<scene2::SpriteNode> meleeDashSprite = scene2::SpriteNode::alloc(meleeDashImage, 1, 7);
    meleeDashSprite->setFrame(0);
    _meleeArmDash->setSceneNode(meleeDashSprite);
    _meleeArmDash->getSceneNode()->setVisible(false);
    _meleeArmDash->setAnimeTimer(0);
    meleeDashSprite->setScale(0.24);
    meleeDashSprite->setPriority(6);
    addObstacle(_meleeArmDash, meleeDashSprite, true);

    // We can only activate a button AFTER it is added to a scene
    _pauseButton->activate();
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
    _worldnode2->addChild(node);
    obj->setNode(node);

    // Dynamic objects need constant updating
    if (obj->getBodyType() == b2_dynamicBody)
    {
        scene2::SceneNode *weak = node.get(); // No need for smart pointer in callback
        obj->setListener([=](physics2::Obstacle *obs)
                         {
                weak->setPosition(obs->getPosition() * _scale);
                weak->setAngle(node->getAngle());
                });
    }
}

/** Saves progress */
void GameScene::save() {
    std::shared_ptr<TextWriter> writer = TextWriter::alloc(Application::get()->getSaveDirectory() + "savedGame.json");
    writer->write("{\"progress\":" + _progress->toString() + ",\"settings\":{\"swap\": " + to_string(_swap) +", \"music\": " + to_string(_music) +", \"sfx\": " + to_string(_sfx) +"}}");
    writer->close();
}

void GameScene::updateTutorialv1(float timestep, int ind) {
    if (_tutorialTimer <= 0 && ind == 1) {
        _tutorial = 0;
        _tutorialInd = 0;
        _tutorialSceneSecond->setVisible(false);
        _tutorialActionDone = false;
        return;
    }
    if (_tutorialTimer <= 0 && ind == 0) {
        _tutorialSceneFirst->setVisible(false);
        _tutorialSceneSecond->setVisible(true);
        _tutorialInd = 1;
        _tutorialActionDone = false;
        _tutorialTimer = TUTORIAL_INIT_TIMER;
        return;
    }
    if(!_tutorialSceneFirst->isVisible() && !_tutorialSceneSecond->isVisible()) {
        _tutorialSceneFirst->setVisible(true);
    }
    if (_tutorialTimer < TUTORIAL_INIT_TIMER || _tutorialActionDone) {
        _tutorialTimer = _tutorialTimer - timestep;
        return;
    }
}

void GameScene::updateTutorialv2(float timestep, int ind) {
    if (_tutorialTimer <= 0 && ind == 2) {
        _tutorial = 0;
        _tutorialInd = 0;
        _tutorialSceneThird->setVisible(false);
        _tutorialActionDone = false;
        return;
    }
    if (_tutorialTimer <= 0 && ind == 1) {
        cout << _tutorialTimer << endl;
        _tutorialSceneSecond->setVisible(false);
        _tutorialSceneThird->setVisible(true);
        _tutorialInd = 2;
        if (_tutorial == 4 || _tutorial == 5) {
            _tutorialActionDone = true;
            _tutorialTimer = TUTORIAL_READING_TIMER;
        } else {
            _tutorialActionDone = false;
            _tutorialTimer = TUTORIAL_INIT_TIMER;
        }
        
        return;
    }
    if (_tutorialTimer <= 0 && ind == 0) {
        _tutorialSceneFirst->setVisible(false);
        _tutorialSceneSecond->setVisible(true);
        _tutorialInd = 1;
        if (_tutorial == 4 || _tutorial == 5) {
            _tutorialActionDone = true;
            _tutorialTimer = TUTORIAL_READING_TIMER;
        } else {
            _tutorialActionDone = false;
            _tutorialTimer = TUTORIAL_INIT_TIMER;
        }
        return;
    }
    if(!_tutorialSceneFirst->isVisible() && !_tutorialSceneSecond->isVisible() && !_tutorialSceneThird->isVisible()) {
        _tutorialSceneFirst->setVisible(true);
    }
    if (_tutorialTimer < TUTORIAL_INIT_TIMER || _tutorialActionDone) {
        _tutorialTimer = _tutorialTimer - timestep;
        return;
    }
}
