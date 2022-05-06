//
//  LevelSelectScene.cpp
//  Cornell University Game Library (CUGL)
//
//  This class manages the home screen, which can switch to the world select or
//  options. It represents a Scene
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
#include "LevelSelectScene.hpp"
#include <cugl/cugl.h>

using namespace cugl;

/** This is the size of the active portion of the screen */
//TODO: MAKE THIS IN ONE SPOT ONLY
#define SCENE_WIDTH 1024

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void LevelSelectScene::dispose()
{
    if (_button1) _button1->deactivate();
    _button1 = nullptr;
    if (_button2) _button2->deactivate();
    _button1 = nullptr;
    if (_button3) _button3->deactivate();
    _button3 = nullptr;
    if (_buttonHome) _buttonHome->deactivate();
    _buttonHome = nullptr;
    if (_buttonForward) _buttonForward->deactivate();
    _buttonForward = nullptr;
    if (_buttonBackward) _buttonBackward->deactivate();
    _buttonBackward = nullptr;

    _batch = nullptr;
    _assets = nullptr;
    removeAllChildren();
    _stageChoice = none;
    _switchChoice = none;
}

/**
 * Initializes the controller contents, and starts the menu
 *
 * The constructor does not allocate any objects or memory.  This allows
 * us to have a non-pointer reference to this controller, reducing our
 * memory allocation.  Instead, allocation happens in this method.
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool LevelSelectScene::init(const std::shared_ptr<cugl::AssetManager> &assets, string biome)
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

    // set assets
    _assets = assets;
    _biome = biome;
    auto scene = _assets->get<scene2::SceneNode>("level_select_" + biome);
    scene->setContentSize(dimen);
    scene->doLayout();

    Rect bounds = Application::get()->getSafeBounds();
    bounds.origin *= boundScale;
    bounds.size *= boundScale;

    // You have to attach the individual loaders for each asset type
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<Font>(FontLoader::alloc()->getHook());
    _button1 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("level_select_" + biome + "_button1"));
    _button1->setPositionX(bounds.getMinX() + _button1->getPositionX());

    _button1->addListener([=](const std::string& name, bool down) {
        if (down) {
            _stageChoice = Choice::button1_prep;
        }
        else if (_stageChoice == Choice::button1_prep) {
            _stage = _stage + 1;
            _stageChoice = Choice::selected;
        }
        });

    _button2 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("level_select_" + biome + "_button2"));
    _button2->setPositionX(bounds.getMinX() + _button2->getPositionX());

    _button2->addListener([=](const std::string& name, bool down){
        if (down) {
            _stageChoice = Choice::button2_prep;
        }
        else if (_stageChoice == Choice::button2_prep) {
            _stage = _stage + 2;
            _stageChoice = Choice::selected;
        }
        });

    _button3 = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("level_select_" + biome + "_button3"));
    _button3->setPositionX(bounds.getMinX() + _button3->getPositionX());
    _button3->addListener([=](const std::string& name, bool down){
        if (down) {
            _stageChoice = Choice::button3_prep;
        }
        else if (_stageChoice == Choice::button3_prep) {
            _stage = _stage + 3;
            _stageChoice = Choice::selected;
        }
        });

    _buttonHome = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("level_select_" + biome + "_home"));
    _buttonHome->setPositionX(bounds.getMinX() + _buttonHome->getPositionX());

    _buttonHome->addListener([=](const std::string& name, bool down) {
        if (down) {
            _stageChoice = Choice::home_prep;
        }
        else if (_stageChoice == Choice::home_prep) {
            _stageChoice = Choice::home;
        }
        });

    _buttonForward = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("level_select_" + biome + "_forward"));
    _buttonForward->addListener([=](const std::string& name, bool down) {
        if (down) {
            _switchChoice = Choice::forward_prep;
        }
        else if (_switchChoice == Choice::forward_prep) {
            _switchChoice = Choice::forward;
            _stage = _stage + 3;
        }
        });

    _buttonBackward = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("level_select_" + biome + "_backward"));
    _buttonBackward->addListener([=](const std::string& name, bool down) {
        if (down) {
            _switchChoice = Choice::backward_prep;
        }
        else if (_switchChoice == Choice::backward_prep) {
            _switchChoice = Choice::backward;
            _stage = _stage - 3;
        }
        });
    

    _buttonBackward->setPositionX(bounds.getMinX() + _buttonBackward->getPositionX());
    addChild(scene);

    _stage = 0;
    if (_biome == "cave") {
        _maxStages = CAVE_MAXLEVELS;
    }
    else if (_biome == "shroom") {
        _maxStages = SHROOM_MAXLEVELS;
    }
    else {
        _maxStages = FOREST_MAXLEVELS;
    }



    return true;
}

#pragma mark -
#pragma mark Screen Handling
/**
 * The method called to update the screen mode.
 *
 * There is not much we need to do here unless a button is pressed
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 * @param highestBiome    The highest biome the player has achieved
 * @param highestLevel    The highest level the player has achieved in the highest biome
 */
void LevelSelectScene::update(float timestep, string highestBiome, int highestLevel)
{
    bool iterateLevel = highestBiome == _biome;
    if (_stage + 1 <= _maxStages) {
        _button1->setVisible(true);
        if (iterateLevel && _stage + 1 > highestLevel){
            _button1->setColor(Color4(24,25,26));
            _button1->deactivate();
        } else {
            _button1->setColor(Color4::WHITE);
            _button1->activate();
        }
    } else {
        _button1->setVisible(false);
        _button1->deactivate();
    }

    if (_stage + 2 <= _maxStages) {
        _button2->setVisible(true);
        if (iterateLevel && _stage + 2 > highestLevel){
            _button2->setColor(Color4(24,25,26));
            _button2->deactivate();
        } else {
            _button2->setColor(Color4::WHITE);
            _button2->activate();
        }
    } else {
        _button2->setVisible(false);
        _button2->deactivate();
    }

    if (_stage + 3 <= _maxStages) {
        _button3->setVisible(true);
        if (iterateLevel && _stage + 3 > highestLevel){
            _button3->setColor(Color4(24,25,26));
            _button3->deactivate();
        } else {
            _button3->setColor(Color4::WHITE);
            _button3->activate();
        }
    }
    else {
        _button3->setVisible(false);
        _button3->deactivate();
    }
    

    _buttonHome->setVisible(true);
    _buttonHome->activate();
    
    if (_stage > 0) {
        _buttonBackward->setVisible(true);
        _buttonBackward->activate();
    }
    else {
        _buttonBackward->setVisible(false);
        _buttonBackward->deactivate();
    }

    if (_stage + 3 < _maxStages) {
        _buttonForward->setVisible(true);
        _buttonForward->activate();
    }
    else {
        _buttonForward->setVisible(false);
        _buttonForward->deactivate();
    }
}

/**
 * @brief Overrides the Scene2 render to render the scene.
 *
 * @param batch the batch to draw
 */
void LevelSelectScene::render(const std::shared_ptr<cugl::SpriteBatch> &batch)
{
    Scene2::render(batch);
}
