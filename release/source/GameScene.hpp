//
//  GameScene.hpp
//  Cornell University Game Library (CUGL)
//
//  This class manages the gameplay. It represents a Scene
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
#ifndef GameScene_hpp
#define GameScene_hpp
#include <cugl/cugl.h>
#include "BaseEnemyModel.h"
#include "Lost.hpp"
#include "Seeker.hpp"
#include "Phantom.hpp"
#include "Mirror.hpp"
#include "Glutton.hpp"
#include "Spawner.hpp"
#include "PlayerModel.h"
#include "Platform.hpp"

#include "AttackController.hpp"
#include "AIController.hpp"
#include "InputController.hpp"
#include "TiltController.hpp"
#include "CollisionController.hpp"
#include "Platform.hpp"
#include "Glow.hpp"
#include "SoundController.hpp"


#include "RRParticle.h"
#include "RRParticleNode.h"
#include "RRParticlePool.h"
/**
 * Class for a simple Hello World style application
 *
 * The application simply moves the CUGL logo across the screen.  It also
 * provides a button to quit the application.
 */
class GameScene : public cugl::Scene2
{
protected:
    struct spawnerEnemy {
        int max_count;
        int current_count;
        float timer;
    };
    
    /** The loaders to (synchronously) load in assets */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** The JSON value with all of the constants */
    std::shared_ptr<cugl::JsonValue> _constants;
    /** The JSON value with all particle effects*/
    std::shared_ptr<cugl::JsonValue> _particleInfo;
    /** A scene graph, used to display our 2D scenes */
    // std::shared_ptr<cugl::Scene2> _scene;
    /** A 3152 style SpriteBatch to render the scene */
    std::shared_ptr<cugl::SpriteBatch> _batch; // check this
    /** A reference to the logo, so that we can move it around */
    std::shared_ptr<cugl::scene2::SceneNode> _logo;
    /** The first damage upgrade picture */
    std::shared_ptr<cugl::scene2::SceneNode> _dmg2;
    /** The second damage upgrade picture */
    std::shared_ptr<cugl::scene2::SceneNode> _dmg3;

    /** The physics world */
    std::shared_ptr<cugl::physics2::ObstacleWorld> _world;
    /** Reference to the physics root of the scene graph */
    std::shared_ptr<cugl::scene2::ScrollPane> _worldnode;
    /** ordered world node to take over _worldnode */
    std::shared_ptr < cugl::scene2::OrderedNode> _worldnode2;
    /** Reference to the debug root of the scene graph */
    std::shared_ptr<cugl::scene2::ScrollPane> _debugnode;

    /** Graphics related*/
    std::shared_ptr<Glow> _playerGlow;

    /** Ranged arm texture*/
    std::shared_ptr<Glow> _rangedArm;
    /** Melee arm texture */
    std::shared_ptr<Glow> _meleeArm;
    /** Melee arm dash texture */
    std::shared_ptr<Glow> _meleeArmDash;
    /** Melee arm jump frame increment */
    int _frameIncrement;
    /** A shader */
    std::shared_ptr<cugl::Shader> _shader;
    /** A vertex buffer */
    std::shared_ptr<cugl::VertexBuffer> _vertbuff;
    /** A mesh for drawing */
    cugl::Mesh<cugl::SpriteVertex2> _mesh;
    /** Test texture */
    std::shared_ptr<cugl::Texture> _textureGraphics;
    /** The type*/
    int _type;
    
    /** The text with the current wave */
    std::shared_ptr<cugl::TextLayout> _text;
    /** The text with the wave timer */
    std::shared_ptr<cugl::TextLayout> _timer_text;
    /** Text font */
    std::shared_ptr<cugl::Font> _font;

    std::shared_ptr<scene2::Label> _leftText;

    std::shared_ptr<scene2::Label> _rightText;

    /** Textures for numbers 0-9 */
    std::vector<std::shared_ptr<Texture>> _numberTextures;

    /** The temp text with the ending win message */
    std::shared_ptr<cugl::TextLayout> _endText;
    
    std::shared_ptr<SoundController> _sound;

    float _platform_attr;

    /** The scale between the physics world and the screen (MUST BE UNIFORM) */
    float _scale;

    InputController _input;

    std::shared_ptr<AttackController> _attacks;
    /** reference to the player Attack Texture*/
    std::shared_ptr<cugl::Texture> _pMeleeTexture;

    /** Swipe Controller */
    SwipeController _swipes;

    /** AI Controller */
    AIController _ai;

    /** Tilt Controller */
    TiltController _tilt;

    /** Collision Controller */
    CollisionController _collider;

    /** Enemies set */
    std::vector<std::shared_ptr<BaseEnemyModel>> _enemies;
    /** Spawner  set */
    std::vector<std::shared_ptr<BaseEnemyModel>> _spawners;

    /** Player character */
    std::shared_ptr<PlayerModel> _player;
    /** Platform character */
    std::vector<float*> _platforms_attr;
    std::vector<std::shared_ptr<PlatformModel>> _platforms;
    std::vector <std::shared_ptr<scene2::PolygonNode>> _platformNodes;
    
    /**spawn info */
    std::vector<vector<string>> _spawn_order;
    std::vector<vector<Vec2>> _spawn_pos;
    std::vector<float> _spawn_times;
    int _spawner_ind;
    /**spawners' enemy types and their max count and current count*/
    std::vector<unordered_map<string, spawnerEnemy>> _spawner_enemy_types;
    std::vector<cugl::Vec2> _spawner_pos;
    int _spawnerCount;
    /** Number of waves for this level */
    int _numWaves;
    /** Next wave number for spawning, starts at 0 */
    int _nextWaveNum;
    /** Whether or not spawn particles have been created already */
    bool _spawnParticlesDone;
    /** Mirror Shard Texture vector*/
    std::vector<std::shared_ptr<Texture>> _mirrorShardList;
    /** Death Particle Texture Vector */
    std::vector<std::shared_ptr<Texture>> _deathParticleList;
    /** Melee Particle Texture Vector */
    std::vector<std::shared_ptr<Texture>> _meleeParticleList;
    /** Range Particle Texture Vector */
    std::vector<std::shared_ptr<Texture>> _rangeParticleList;
    /** Spawn Portal Texture Vector */
    std::vector<std::shared_ptr<Texture>> _spawnPortalList;

    /** A game timer used for spawn times */
    float _timer;
    /** living spawners */
    std::vector<int> _living_spawners;
    
    /** Whether or not debug mode is active */
    bool _debug;
    
    /** How long current melee dash has lasted */
    float _dashTime;
    /** XVel for this current melee dash */
    int _dashXVel;
    /** YVel for the current melee dash */
    int _dashYVel;
    /** bool whether dash is cancelled */
    bool _cancelDash;
    
    /** frame used for when animation interrupted when stunned */
    int _prevFrame;
    
    /** Used for dash animation */
    SwipeController::SwipeAttack _dashDir;

    /** true if going back to world select */
    bool _back;
    
    /** true if going to level select */
    bool _levelselect;
    
    /** Boolean check for walking sound effect timing*/
    bool _step;

    /** healthbar */
    std::shared_ptr<scene2::ProgressBar> _healthbar;
    
    /** range charge indicator */
    std::shared_ptr<scene2::ProgressBar> _range_charge;
    
    /** melee charge indicator */
    std::shared_ptr<scene2::ProgressBar> _melee_charge;
    
    std::shared_ptr<scene2::ProgressBar> _wavebar;

    bool _winInit;

    float _winFadeTimer;

    bool _next;

    string _biome;

    int _stageNum;

    bool _pause;

    bool _options;
    
    bool _lose;
    
    bool _chargeSoundCueM;
    
    bool _chargeSoundCueR;

    int _tutorial;
    
    int _initTutorial;
    
    float _tutorialTimer;
    
    int _tutorialInd;
    
    bool _tutorialActionDone;
    std::shared_ptr<scene2::Button> _pauseButton;

    std::shared_ptr<scene2::SceneNode> _pauseScene;

    std::shared_ptr<scene2::Button> _returnButton;

    std::shared_ptr<scene2::Button> _homeButton;

    std::shared_ptr<scene2::Button> _optionButton;

    std::shared_ptr<scene2::SceneNode> _optionScene;

    std::shared_ptr<scene2::Button> _optionReturnButton;

    std::shared_ptr<scene2::Button> _swapHandsButton;

    bool _swap;

    std::shared_ptr<scene2::Button> _musicButton;

    int _music;

    std::shared_ptr<scene2::Button> _sfxButton;

    int _sfx;
    
    std::shared_ptr<scene2::SceneNode> _loseScene;
    
    std::shared_ptr<scene2::Button> _loseHomeButton;

    std::shared_ptr<scene2::Button> _loseLevelButton;
    
    std::shared_ptr<scene2::Button> _loseRestartButton;

    std::shared_ptr<scene2::SceneNode> _tutorialSceneFirst;
    
    std::shared_ptr<scene2::SceneNode> _tutorialSceneSecond;
    
    std::shared_ptr<scene2::SceneNode> _tutorialSceneThird;
    std::shared_ptr<JsonValue> _progress;

    void save();

    
    /**
     * Internal helper to build the scene graph.
     *
     * Scene graphs are not required.  You could manage all scenes just like
     * you do in 3152.  However, they greatly simplify scene management, and
     * have become standard in most game engines.
     */
    void buildScene(std::shared_ptr<cugl::scene2::SceneNode> scene);

public:
    /**
     * Creates a new game mode with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    GameScene() : cugl::Scene2() {}

    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~GameScene() { dispose(); }

    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose();

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
    bool init(const std::shared_ptr<cugl::AssetManager> &assets, const std::shared_ptr<SoundController> sound, string biome, int stageNum, int tutorial);

    bool goingBack() { return _back; }
    
    bool goingLevelSelect() {return _levelselect; }

    bool next() { return _next; }

#pragma mark -
#pragma mark Gameplay Handling
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
    void update(float timestep, int unlockCount);

    /**
     * helper method to update sound, input, particles, and tilt/
     */
    void updateSoundInputParticlesAndTilt(float timestep);
    /**
     * helper method to update all animations
     */
    void updateAnimations(float timestep, int unlockCount, SwipeController::SwipeAttack left, SwipeController::SwipeAttack right);
    /**
    * helper method to update melee arm animations
    */
    void updateMeleeArm(float timestep);
    /**
     * helper method to update all enemies 
     */
    void updateEnemies(float timestep);
    /** helper method to update left swipe */
    SwipeController::SwipeAttack updateLeftSwipe(int unlockCount);
    /** helper method to update right swipe */
    SwipeController::SwipeAttack updateRightSwipe(int unlockCount);
    /** 
     * helper method to update attacks
     */
    void updateAttacks(float timestep, int unlockCount, SwipeController::SwipeAttack left, SwipeController::SwipeAttack right);
    /** 
     * helper method to remove deleted attacks
     */
    void updateRemoveDeletedAttacks();
    /**
     * helper method to remove deleted enemies
     */
    void updateRemoveDeletedEnemies();
    /**
     * helper method to update text
     */
    void updateText();
    /** 
     * helper method to update spawning times 
     */
    void updateSpawnTimes();
    /** 
     * helper method to remove deleted player 
     */
    void updateRemoveDeletedPlayer();
    /**
     * helper method to update player HUD
     */
    void updateHUD(int unlockCount);
    /**
     * helper method to update camera 
     */
    void updateCamera();
    /**
     * helper method to spawn enemies 
     */
    void updateSpawnEnemies(float timestep);
    /** Creates the spawning particles of enemies 3 sec before they spawn*/
    void createSpawnParticles();
    /**
     * helper method to win game
     */
    bool updateWin();

    void updateTilt();

    /**
     * Resets the status of the game so that we can play again.
     */
    void reset();

    /**
     * @brief Overrides the Scene2 render to render the scene.
     *
     * @param batch the batch to draw
     */
    void render(const std::shared_ptr<cugl::SpriteBatch> &batch);

    /**
     * @brief Adds an object to the ObstacleWorld.
     *
     * @param obj
     * @param node
     * @param useObjPosition
     */
    virtual void addObstacle(const std::shared_ptr<cugl::physics2::Obstacle> &obj,
                             const std::shared_ptr<cugl::scene2::SceneNode> &node,
                             bool useObjPosition);

    /**
     * Returns true if debug mode is active.
     *
     * If true, all objects will display their physics bodies.
     *
     * @return true if debug mode is active.
     */
    bool isDebug() const { return _debug; }


    /** Helper for creating an uninteractable obstacle with Texture textureName at startPos with scale scale */
    void createAndAddDeathAnimationObstacle(string textureName, Vec2 startPos, float scale, int frames, int tag);

    /**
     * Sets whether debug mode is active.
     *
     * If true, all objects will display their physics bodies.
     *
     * @param value whether debug mode is active.
     */
    void setDebug(bool value)
    {
        _debug = value;
        _debugnode->setVisible(value);
    }

    /**
    * Creates a single enemy
    */
    void createEnemy(string enemyName, Vec2 enemyPos, int isSpawnerEnemy);

    /**
     * Creates all enemies and adds to _enemies.
     */
    void createEnemies(int wave);
    
    /**
    * helper to create enemy from spawner, adding it to _enemmies
    */
    void createSpawnerEnemy(int spawnerInd, string enemyName);
    
    /** 
    * helper to create mirror enemies, adding them to _enemmies
    */
    void createMirror(cugl::Vec2 enemyPos, Mirror::Type type, std::string asset, std::shared_ptr<Glow> enemyGlow);

    std::shared_ptr<BaseEnemyModel> getNearestNonMirror(cugl::Vec2 pos);

    /** Helpers to create particles in GameScene */
    void createParticles(std::shared_ptr<Texture> texture, Vec2 pos, string poolName, Color4 tint, Vec2 pointOffset, float scale);
    void createParticles(std::vector<std::shared_ptr<Texture>> textures, Vec2 pos, string poolName, Color4 tint, Vec2 pointOffset, float scale, bool hasMultipleLinkedTextures, Vec2 linkOffset, int numTex);


    /** Helper to convert numbers into Textures **/
    std::vector<std::shared_ptr<Texture>> getTexturesFromNumber(int num);

    string getBiome() { return _biome; }

    int getStageNum() { return _stageNum; }
    
    void updateTutorialv1(float timestep, int ind);

    void updateTutorialv2(float timestep, int ind);

};

#endif /* __Game_Scene_hpp__ */
