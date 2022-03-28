//
//  LiminalSpiritApp.cpp
//  LiminalSpirit
//
//  This is the root class for your game.  The file main.cpp accesses this class
//  to run the application.  While you could put most of your game logic in
//  this class, we prefer to break the game up into player modes and have a
//  class for each mode.
//
//  Created: 3/13/22
//
#include "LiminalSpiritApp.hpp"

using namespace cugl;

#pragma mark -
#pragma mark Gameplay Control

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
    _assets = AssetManager::alloc();
    _batch = SpriteBatch::alloc();
    _scene = State::LOADING;

    // Start-up basic input
#ifdef CU_MOBILE
    Input::activate<Touchscreen>();
#else
    Input::activate<Mouse>();
#endif

    _assets->attach<Font>(FontLoader::alloc()->getHook());
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<scene2::SceneNode>(Scene2Loader::alloc()->getHook());

    // Create a "loading" screen
    //_loaded = false;
    _loading.init(_assets);

    // TODO check this
    _assets->attach<JsonValue>(JsonLoader::alloc()->getHook());

    // Queue up the other assets
    _assets->loadDirectoryAsync("json/assets.json", nullptr);
    //_assets->loadDirectory("json/assets.json");
    Application::onStartup(); // YOU MUST END with call to parent
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
    _loading.dispose();
    _gameplay.dispose();
    _home.dispose();
    _worldSelect.dispose();
    _assets = nullptr;
    _batch = nullptr;

    // Shutdown input
#ifdef CU_MOBILE
    // Deativate input
    Input::deactivate<Touchscreen>();
// #if defined CU_TOUCH_SCREEN
#else
    Input::deactivate<Mouse>();
#endif

    Application::onShutdown(); // YOU MUST END with call to parent
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
    switch (_scene)
    {
    case LOADING:
        updateLoadingScene(timestep);
        break;
    case HOME:
        updateHomeScene(timestep);
        break;
    case WORLDS:
        updateWorldSelectScene(timestep);
        break;
    case GAME:
        updateGameScene(timestep);
        break;
    }
    /*if (!_loaded && _loading.isActive())
    {
        _loading.update(0.01f);
    }
    else if (!_loaded)
    {
        _loading.dispose(); // Disables the input listeners in this mode
        _gameplay.init(_assets);
        _home.init(_assets);
        _loaded = true;
    }
    else
    {
        _gameplay.update(timestep);
    }*/
}

/**
 * Inidividualized update method for the loading scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the loading scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void LiminalSpirit::updateLoadingScene(float timestep)
{
    if (_loading.isActive())
    {
        _loading.update(timestep);
    }
    else
    {
        _loading.dispose(); // Permanently disables the input listeners in this mode
        // TODO add other screens
        _home.init(_assets);
        _gameplay.init(_assets);
        _worldSelect.init(_assets);
        _scene = State::HOME;
    }
}

/**
 * Individualized update method for the home scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the home scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void LiminalSpirit::updateHomeScene(float timestep)
{
    _home.update(timestep);
    switch (_home.getChoice()) {
    case HomeScene::Choice::PLAY:
        _scene = State::WORLDS;
        break;
    }
}

/**
 * Individualized update method for the world select scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the home scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void LiminalSpirit::updateWorldSelectScene(float timestep)
{
    _worldSelect.update(timestep);
    switch (_worldSelect.getChoice()) {
    case WorldSelectScene::Choice::CAVE:
        _scene = State::GAME;
        break;
    case WorldSelectScene::Choice::SHROOM:
        _scene = State::GAME;
        break;
    case WorldSelectScene::Choice::FOREST:
        _scene = State::GAME;
        break;
    }
}

/**
 * Individualized update method for the game scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the game scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void LiminalSpirit::updateGameScene(float timestep)
{
    _gameplay.update(timestep);
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
    switch (_scene)
    {
    case LOADING:
        _loading.render(_batch);
        break;
    case HOME:
        _home.render(_batch);
        break;
    case WORLDS:
        _worldSelect.render(_batch);
        break;
    case GAME:
        _gameplay.render(_batch);
        break;
    }
    /*if (!_loaded)
    {
        _loading.render(_batch);
    }
    else
    {
        _gameplay.render(_batch);
    }*/
}
