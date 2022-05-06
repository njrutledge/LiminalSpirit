//
//  LSApp.cpp
//  Liminal Spirit
//
//  This is the root class for your game.  The file main.cpp accesses this class
//  to run the application.  While you could put most of your game logic in
//  this class, we prefer to break the game up into player modes and have a
//  class for each mode.
//
//  Author: Walker White
//  Version: 1/10/17
//
#ifndef LiminalSpirit_hpp
#define LiminalSpirit_hpp
#include <cugl/cugl.h>
#include "GameScene.hpp"
#include "BossScene.hpp"
#include "LoadingScene.hpp"
#include "SoundController.hpp"
#include "HomeScene.hpp"
#include "WorldSelectScene.hpp"
#include "LevelSelectScene.hpp"

/**
 * This class represents the application root for the ship demo.
 */
class LiminalSpirit : public cugl::Application
{
protected:
    /** The global sprite batch for drawing (only want one of these) */
    std::shared_ptr<cugl::SpriteBatch> _batch;
    /** The global asset manager */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    std::shared_ptr<SoundController> _sound_controller;
    
    /** The highest biome the player has access to */
    int _biome;
    
    /** The highest level the player has access to (within the highest biome) */
    int _highest_level;
    
    /** The amount of upgrades the player has unlocked */
    int _unlock_count;
    
    /** Whether or not the player's hands are swapped */
    bool _swap;

    /**
     * The current active scene
     */
    enum State
    {
        /** The loading scene */
        LOADING,
        /** The home menu scene */
        HOME,
        /** The options menu*/
        OPTIONS,
        /** The world menu scene*/
        WORLDS,
        /** The select menu scene */
        SELECT,
        /** The cave level select screen*/
        CAVE_LEVELS,
        /** The fungi level select screen*/
        FUNGI_LEVELS,
        /** The forest level select screen*/
        FOREST_LEVELS,
        /** The pause menu*/
        PAUSE,
        /** The scene to play the game */
        GAME,
        /** The scene to play the boss */
        BOSS
    };

    // Player modes
    /** The primary controller for the game world */
    GameScene _gameplay;
    /** the primary controller for the boss world */
    BossScene _bossgame;
    /** The controller for the loading screen */
    LoadingScene _loading;
    /** The controller for the home screen*/
    HomeScene _home;
    /** The controller for the world select screen */
    WorldSelectScene _worldSelect;
    /** The controller for the world select screen */
    LevelSelectScene _levelSelect;
    /** Whether or not we have finished loading all assets */
    bool _loaded;

    /** The current active scene*/
    State _scene;
    
    /** Saves progress */
    void save();
    
    /** Grants the player abilities once they have access to certain stages
     */
    void checkPlayerUnlocks();

public:
    /**
     * Creates, but does not initialized a new application.
     *
     * This constructor is called by main.cpp.  You will notice that, like
     * most of the classes in CUGL, we do not do any initialization in the
     * constructor.  That is the purpose of the init() method.  Separation
     * of initialization from the constructor allows main.cpp to perform
     * advanced configuration of the application before it starts.
     */
    LiminalSpirit() : cugl::Application(), _loaded(false) {}

    /**
     * Disposes of this application, releasing all resources.
     *
     * This destructor is called by main.cpp when the application quits.
     * It simply calls the dispose() method in Application.  There is nothing
     * special to do here.
     */
    ~LiminalSpirit() {}

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
    virtual void onStartup() override;

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
    virtual void onShutdown() override;

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
    virtual void update(float timestep) override;

    /**
     * Individualized update method for the loading scene.
     *
     * This method keeps the primary {@link #update} from being a mess of switch
     * statements. It also handles the transition logic from the loading scene.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void updateLoadingScene(float timestep);

    /**
     * Individualized update method for the home scene.
     *
     * This method keeps the primary {@link #update} from being a mess of switch
     * statements. It also handles the transition logic from the home scene.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void updateHomeScene(float timestep);

    /**
     * Individualized update method for the level select scene.
     *
     * This method keeps the primary {@link #update} from being a mess of switch
     * statements. It also handles the transition logic from the home scene.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void updateLevelSelectScene(float timestep);

    /**
     * Individualized update method for the world select scene.
     *
     * This method keeps the primary {@link #update} from being a mess of switch
     * statements. It also handles the transition logic from the loading scene.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void updateWorldSelectScene(float timestep);

    /**
     * Individualized update method for the game scene.
     *
     * This method keeps the primary {@link #update} from being a mess of switch
     * statements. It also handles the transition logic from the game scene.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void updateGameScene(float timestep);

    /**
     * Individualized update method for the boss scene.
     *
     * This method keeps the primary {@link #update} from being a mess of switch
     * statements. It also handles the transition logic from the game scene.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void updateBossScene(float timestep);

    /**
     * The method called to draw the application to the screen.
     *
     * This is your core loop and should be replaced with your custom implementation.
     * This method should OpenGL and related drawing calls.
     *
     * When overriding this method, you do not need to call the parent method
     * at all. The default implmentation does nothing.
     */
    virtual void draw() override;
};

#endif /* __LiminalSpiritApp_hpp__ */
