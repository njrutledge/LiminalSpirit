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
#include "PlayerModel.h"
#include "Platform.hpp"

#include "AttackController.hpp"
#include "AIController.hpp"
#include "InputController.hpp"
#include "TiltController.hpp"
#include "CollisionController.hpp"
#include "Platform.hpp"
#include "Glow.hpp"
#include "Particle.hpp"
#include "SoundController.hpp"
/**
 * Class for a simple Hello World style application
 *
 * The application simply moves the CUGL logo across the screen.  It also
 * provides a button to quit the application.
 */
class GameScene : public cugl::Scene2
{
protected:
    /** The loaders to (synchronously) load in assets */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** The JSON value with all of the constants */
    std::shared_ptr<cugl::JsonValue> _constants;
    /** A scene graph, used to display our 2D scenes */
    // std::shared_ptr<cugl::Scene2> _scene;
    /** A 3152 style SpriteBatch to render the scene */
    std::shared_ptr<cugl::SpriteBatch> _batch; // check this
    /** A reference to the logo, so that we can move it around */
    std::shared_ptr<cugl::scene2::SceneNode> _logo;

    /** The physics world */
    std::shared_ptr<cugl::physics2::ObstacleWorld> _world;
    /** Reference to the physics root of the scene graph */
    std::shared_ptr<cugl::scene2::ScrollPane> _worldnode;
    /** Reference to the debug root of the scene graph */
    std::shared_ptr<cugl::scene2::ScrollPane> _debugnode;

    /** Graphics related*/
    std::shared_ptr<Glow> _playerGlow;

    /** Ranged arm texture*/
    std::shared_ptr<Glow> _rangedArm;
    /** Melee arm texture */
    std::shared_ptr<Glow> _meleeArm;
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
    
    /** The text with the current health */
    std::shared_ptr<cugl::TextLayout> _text;
    /** Text font */
    std::shared_ptr<cugl::Font> _font;
    /** The temp text with the ending win message */
    std::shared_ptr<cugl::TextLayout> _endText;
    
    std::shared_ptr<SoundController> _sound;

    float _platform_attr;
    /** Particle Pool */
    std::vector<std::shared_ptr<Particle>> _particlePool;

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
    /** Number of waves for this level */
    int _numWaves;
    /** Next wave number for spawning, starts at 0 */
    int _nextWaveNum;
    /** A game timer used for spawn times */
    float _timer;

    /** Whether or not debug mode is active */
    bool _debug;
    
    /** How long current melee dash has lasted */
    float _dashTime;
    /** XVel for this current melee dash */
    int _dashXVel;
    /** YVel for the current melee dash */
    int _dashYVel;
    
    /** jump frame for paused jump*/
    int _jumpFrame;

    /** true if going back to world select */
    bool _back;
    
    /** Boolean check for walking sound effect timing*/
    bool _step;

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
    bool init(const std::shared_ptr<cugl::AssetManager> &assets, const std::shared_ptr<SoundController> sound, string json);

    bool goingBack() { return _back; }

#pragma mark -
#pragma mark Gameplay Handling
    /**
     * The method called to update the game mode.
     *
     * This method contains any gameplay code that is not an OpenGL call.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void update(float timestep);

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
     * Creates all enemies and adds to _enemies
     */
    void createEnemies(int wave);

    /** 
    * helper to create mirror enemies, adding them to _enemmies
    */
    void createMirror(cugl::Vec2 enemyPos, Mirror::Type type, std::string asset, std::shared_ptr<Glow> enemyGlow);

    std::shared_ptr<BaseEnemyModel> getNearestNonMirror(cugl::Vec2 pos);

    /** Creates the intial particle pool */
    void createParticles();

};

#endif /* __Game_Scene_hpp__ */
