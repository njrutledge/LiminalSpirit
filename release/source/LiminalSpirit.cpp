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

// CAVE UNLOCKS
#define RANGED_UNLOCK 5
// SHROOM UNLOCKS
#define CHARGED_RANGED_UNLOCK 1
#define ATTACK_UPGRADE_1 4
// FOREST UNLOCKS
#define CHARGED_MELEE_UNLOCK 1
#define ATTACK_UPGRADE_2 5

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
    _assets->attach<WidgetValue>(WidgetLoader::alloc()->getHook());
    _assets->attach<scene2::SceneNode>(Scene2Loader::alloc()->getHook());
    _assets->attach<Sound>(SoundLoader::alloc()->getHook());

    // Create a "loading" screen
    //_loaded = false;
    _loading.init(_assets);

    // TODO check this
    _assets->attach<JsonValue>(JsonLoader::alloc()->getHook());

    // Queue up the other assets
    _assets->loadDirectoryAsync("json/assets.json", nullptr);
    //_assets->loadDirectory("json/assets.json");
    
    AudioEngine::start(32);
    
    if (!filetool::file_exists(Application::get()->getSaveDirectory() + "savedGame.json")) {
        std::shared_ptr<TextWriter> writer = TextWriter::alloc(Application::get()->getSaveDirectory() + "savedGame.json");
        writer->write("{\"progress\":{\"biome\": 1, \"highest_level\": 1, \"unlock_count\": 0}, \"settings\":{\"swap\": false}}");
        writer->close();
    }
    
    std::shared_ptr<JsonReader> reader = JsonReader::alloc(Application::get()->getSaveDirectory() + "savedGame.json");
    std::shared_ptr<JsonValue> save = reader->readJson();
    std::shared_ptr<JsonValue> progress = save->get("progress");
    std::shared_ptr<JsonValue> settings = save->get("settings");
    reader->close();
    
    // Note: COMMENT THESE OUT TO DISABLE PROGRESSION!!!!!!!!
//    _biome = progress->get("biome")->asInt();
//    _highest_level = progress->get("highest_level")->asInt();
//    _unlock_count = progress->get("unlock_count")->asInt();
    // Note: COMMENT THESE OUT TO ENABLE PROGRESSION!!!!!!!!
    _biome = 3;
    _highest_level = 9;
    _unlock_count = 5;
    int swap = settings->get("swap")->asInt();
    _swap = settings->get("swap")->asInt();
    this->save();
    
    CULog("Biome: %d, Level: %d, Unlocks: %d, Swap: %d", _biome, _highest_level, _unlock_count, _swap);
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
    _levelSelect.dispose();
    _credit.dispose();
    _assets = nullptr;
    _batch = nullptr;
    _sound_controller = nullptr;

    // Shutdown input
#ifdef CU_MOBILE
    // Deativate input
    Input::deactivate<Touchscreen>();
// #if defined CU_TOUCH_SCREEN
#else
    Input::deactivate<Mouse>();
#endif
    AudioEngine::stop();
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
    case SELECT:
        updateLevelSelectScene(timestep);
        break;
    case GAME:
        updateGameScene(timestep);
        break;
    case CREDIT:
        updateCreditScene();
        break;
    case BOSS:
        updateBossScene(timestep);
        break;
    }
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
        _sound_controller = make_shared<SoundController>();
        _sound_controller->init(_assets);
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
    _sound_controller->play_menu_music();
    switch (_home.getChoice()) {
    case HomeScene::Choice::PLAY:
        _scene = State::WORLDS;
        break;
    case HomeScene::Choice::CREDIT:
        _credit.init(_assets);
        _credit.setDefaultChoice();
        _worldSelect.setDefaultChoice();
        _levelSelect.setDefaultChoice();
        _scene = State::CREDIT;
        break;
    }
}

void LiminalSpirit::updateCreditScene()
{
    _sound_controller->play_menu_music();
    switch (_credit.getChoice()) {
    case CreditScene::Choice::HOME:
        _scene = State::WORLDS;
        _credit.dispose();
        _worldSelect.setDefaultChoice();
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
    _worldSelect.update(timestep, _biome);
    _sound_controller->play_menu_music();
    _credit.setDefaultChoice();
    switch (_worldSelect.getChoice()) {
    case WorldSelectScene::Choice::CAVE:
        _levelSelect.init(_assets, "cave");
        _scene = State::SELECT;
        //_gameplay.init(_assets, _sound_controller, "surround");
        //_scene = State::GAME;
        break;
    case WorldSelectScene::Choice::SHROOM:
        _levelSelect.init(_assets, "shroom");
        _scene = State::SELECT;
        //_gameplay.init(_assets, _sound_controller, "battlefield");
        //_scene = State::GAME;
        break;
    case WorldSelectScene::Choice::FOREST:
        _levelSelect.init(_assets, "forest");
        _scene = State::SELECT;
        //_gameplay.init(_assets, _sound_controller, "stack");
        //_scene = State::GAME;
//        _bossgame.init(_assets, _sound_controller);
//        _scene = State::BOSS;
        break;
    case WorldSelectScene::Choice::BACK:
        _scene = State::HOME;
        _home.setDefaultChoice();
        _worldSelect.setDefaultChoice();
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
    _gameplay.update(timestep, _unlock_count);
    if (_gameplay.goingBack()) {
        _scene = State::WORLDS;
        _gameplay.dispose();
        _worldSelect.setDefaultChoice();
        _levelSelect.setDefaultChoice();
    }
    else if (_gameplay.goingLevelSelect()) {
        string biome = _gameplay.getBiome();
        _worldSelect.setDefaultChoice();
        _levelSelect.init(_assets, biome);
        _scene = State::SELECT;
        _gameplay.dispose();
    }
    else if (_gameplay.next()) {
        _gameplay.dispose();
        string biome = _gameplay.getBiome();
        int nextStage = _gameplay.getStageNum() + 1;
        if ( nextStage > _levelSelect.getMaxStages(biome)) {
            if (biome == "cave") {
                biome = "shroom";
                if(_biome < 2){
                    _biome = 2;
                }
            }
            else if (biome == "shroom") {
                biome = "forest";
                if(_biome < 3){
                    _biome = 3;
                }
            }
            else {
                //no more biomes, you won!
                _scene = State::CREDIT;
                _credit.init(_assets);
                _credit.setDefaultChoice();
//                _scene = State::WORLDS;
                _worldSelect.setDefaultChoice();
                _levelSelect.setDefaultChoice();
                return;
            }
            nextStage = 1;
            _highest_level = 1;
            //more levels to go!
        }
        bool checkLevels = (biome == "cave" && _biome == 1) || (biome == "shroom" && _biome == 2) || (biome == "forest" && _biome == 3);
        if(checkLevels && nextStage > _highest_level){
            _highest_level = nextStage;
        }
        checkPlayerUnlocks();
        save();
        
        if (checkLevels && biome == "cave" && _highest_level == 1 && nextStage == 1) {
            _gameplay.init(_assets, _sound_controller, biome, nextStage, 1);
        }
        else if (checkLevels && biome == "cave" && _highest_level == 2 && nextStage == 2) {
            _gameplay.init(_assets, _sound_controller, biome, nextStage, 2);
        }
        else if(checkLevels && biome == "cave" && _highest_level == RANGED_UNLOCK && nextStage == RANGED_UNLOCK) {
            _gameplay.init(_assets, _sound_controller, biome, nextStage, 3);
        }
        else if(checkLevels && biome == "shroom" && _highest_level == CHARGED_RANGED_UNLOCK && nextStage == CHARGED_RANGED_UNLOCK) {
            _gameplay.init(_assets, _sound_controller, biome, nextStage, 4);
        }
        else if(checkLevels && biome == "forest" && _highest_level == CHARGED_MELEE_UNLOCK && nextStage == CHARGED_MELEE_UNLOCK) {
            _gameplay.init(_assets, _sound_controller, biome, nextStage, 5);
        }
        else {
            _gameplay.init(_assets, _sound_controller, biome, nextStage, 0);
        }
        
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
void LiminalSpirit::updateLevelSelectScene(float timestep)
{
    _sound_controller->play_menu_music();
    string biome;
    switch(_biome){
        case 1:
            biome = "cave";
            break;
        case 2:
            biome = "shroom";
            break;
        default:
            biome = "forest";
            break;
    }
    _levelSelect.update(timestep, biome, _highest_level);
    switch (_levelSelect.getChoice()) {
        case LevelSelectScene::Choice::selected: {
            string biome = _levelSelect.getBiome();
            int nextStage = _levelSelect.getStage();
            if (_biome == 1 && biome == "cave" && _highest_level == 1 && nextStage == 1) {
                _gameplay.init(_assets, _sound_controller, biome, nextStage, 1);
            }
            else if (_biome == 1 && biome == "cave" && _highest_level == 2 && nextStage == 2) {
                _gameplay.init(_assets, _sound_controller, biome, nextStage, 2);
            }
            else if(_biome == 1 && biome == "cave" && _highest_level == RANGED_UNLOCK && nextStage == RANGED_UNLOCK) {
                _gameplay.init(_assets, _sound_controller, biome, nextStage, 3);
            }
            else if(_biome == 2 && biome == "shroom" && _highest_level == CHARGED_RANGED_UNLOCK && nextStage == CHARGED_RANGED_UNLOCK) {
                _gameplay.init(_assets, _sound_controller, biome, nextStage, 4);
            }
            else if(_biome == 3 && biome == "forest" && _highest_level == CHARGED_MELEE_UNLOCK && nextStage == CHARGED_MELEE_UNLOCK) {
                _gameplay.init(_assets, _sound_controller, biome, nextStage, 5);
            }
            else {
                _gameplay.init(_assets, _sound_controller, biome, nextStage, 0);
            }

            _levelSelect.dispose();
            _scene = State::GAME;
            break;
        }
        case LevelSelectScene::Choice::home: {
            _worldSelect.setDefaultChoice();
            _levelSelect.dispose();
            _scene = State::WORLDS;
            break;
        }
    }
}

/**
 * Individualized update method for the boss scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the game scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void LiminalSpirit::updateBossScene(float timestep)
{
    _bossgame.update(timestep);
    if (_bossgame.goingBack()) {
        _scene = State::WORLDS;
        _bossgame.dispose();
        _worldSelect.setDefaultChoice();
    }
}

/** Grants the player abilities once they have access to certain stages
 */
void LiminalSpirit::checkPlayerUnlocks(){
    switch(_biome){
        case 1:
            if(_highest_level >= RANGED_UNLOCK){
                _unlock_count = 1;
            }
            break;
        case 2:
            if (_highest_level >= ATTACK_UPGRADE_1){
                _unlock_count = 3;
            } else if (_highest_level >= CHARGED_RANGED_UNLOCK){
                _unlock_count = 2;
            }
            break;
        case 3:
            if (_highest_level >= ATTACK_UPGRADE_2){
                _unlock_count = 5;
            } else if (_highest_level >= CHARGED_MELEE_UNLOCK){
                _unlock_count = 4;
            }
            break;
    }
}

/** Saves progress */
void LiminalSpirit::save(){
    std::shared_ptr<TextWriter> writer = TextWriter::alloc(Application::get()->getSaveDirectory() + "savedGame.json");
    writer->write("{\"progress\":{\"biome\": " + to_string(_biome) + ", \"highest_level\": " + to_string(_highest_level) + ", \"unlock_count\": " + to_string(_unlock_count) + "}, \"settings\":{\"swap\": " + to_string(_swap) +"}}");
    writer->close();
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
    case SELECT:
        _levelSelect.render(_batch);
        break;
    case GAME:
        _gameplay.render(_batch);
        break;
    case CREDIT:
        _credit.render(_batch);
        break;
    case BOSS:
        _bossgame.render(_batch);
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
