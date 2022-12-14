//
//  WorldSelectScene.cpp
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
#include "WorldSelectScene.hpp"
#include <cugl/cugl.h>
#include "InputController.hpp"

using namespace cugl;

/** This is the size of the active portion of the screen */
//TODO: MAKE THIS IN ONE SPOT ONLY
#define SCENE_WIDTH 1024

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void WorldSelectScene::dispose()
{
    if (_caveButton) {
        //_caveButton->setPositionX( _caveButton->getPositionX() - _safeBounds.getMinX());//reset the position, so it doesn't move
        //_caveButtonBack->setPositionX( _caveButtonBack->getPositionX()- _safeBounds.getMinX());
        _caveButton->deactivate();
    }

    _caveButton = nullptr;
    if (_shroomButton) _shroomButton->deactivate();
    _shroomButton = nullptr;
    if (_forestButton) _forestButton->deactivate();
    _forestButton = nullptr;
    if (_backButton) {
        //_backButton->setPositionX(_backButton->getPositionX() - _safeBounds.getMinX());//reset the position, so it doesn't move
        _backButton->deactivate();
    }
    _backButton = nullptr;
    _batch = nullptr;
    _assets = nullptr;
    removeAllChildren();

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
bool WorldSelectScene::init(const std::shared_ptr<cugl::AssetManager> &assets)
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
    auto scene = _assets->get<scene2::SceneNode>("world_select");
    scene->setContentSize(dimen);
    scene->doLayout();
    
    Rect bounds = Application::get()->getSafeBounds();
    bounds.origin *= boundScale;
    bounds.size *= boundScale;

    float scale = bounds.size.width / 32.0f;

      auto backdrop = _assets->get<scene2::SceneNode>("world_select_backdrop");
      backdrop->setScale(0.66 * scale / 32.0f);

    // You have to attach the individual loaders for each asset type
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<Font>(FontLoader::alloc()->getHook());

    _safeBounds = Application::get()->getSafeBounds();
    _safeBounds.origin *= boundScale;
    _safeBounds.size *= boundScale;

    _backButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("world_select_back"));
    _backButton->clearListeners();
    _backButton->addListener([=](const std::string& name, bool down)
        {
            if (!down) {
                _choice = Choice::BACK;
            }
        });



    _caveButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("world_select_cave"));
    _caveButton->clearListeners();
    _caveButton->addListener([=](const std::string& name, bool down)
        {
            if (!down) {
                _choice = Choice::CAVE;
            }
        });
    _caveButton->setPositionX(_safeBounds.getMinX() + _caveButton->getPositionX());


    _caveButtonBack = assets->get<scene2::SceneNode>("world_select_caveback");
    _caveButtonBack->setPositionX(_safeBounds.getMinX() + _caveButtonBack->getPositionX());

    _shroomButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("world_select_shroom"));
    _shroomButton->clearListeners();
    _shroomButton->addListener([=](const std::string& name, bool down)
        {
         if (!down) {
                _choice = Choice::SHROOM;
            }
        });
    _shroomButton->setColor(Color4(24,25,26));
    
    _shroomButtonBack = assets->get<scene2::SceneNode>("world_select_shroomback");
    _shroomButtonBack->setColor(Color4(24,25,26));

    _forestButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("world_select_forest"));
    _forestButton->clearListeners();
    _forestButton->addListener([=](const std::string& name, bool down)
        {
         if (!down) {
                _choice = Choice::FOREST;
            }
        });
    _forestButton->setColor(Color4(24,25,26));

    _forestButtonBack = assets->get<scene2::SceneNode>("world_select_forestback");
    _forestButtonBack->setColor(Color4(24,25,26));

    _timerA = 0.0f;
    _timerB = 0.0f;
    _timerC = 0.0f;

    addChild(scene);
    return true;
}

#pragma mark -
#pragma mark Screen Handling

float WorldSelectScene::easing(float time, float time_position, float amplitude) {
    
    float position = time_position / time;
    
    
    if (position <= 0.25) {
        return (amplitude) * (1 - ((time_position / (time / 4))));
    } else if (position <= 0.5) {
        return (amplitude) * (((time_position - time / 4) / (time / 4))) * -1;
    } else if (position <= 0.75) {
        return (amplitude) * (1 - ((time_position -  time / 2) / (time / 4))) * -1;
    } else if (position <= 1.0f) {
        return (amplitude) * (((time_position - (3 * time / 4)) / (time / 4)));
    } else {
        return 0.0f;
    }
    
}
/**
 * The method called to update the screen mode.
 *
 * There is not much we need to do here unless a button is pressed
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void WorldSelectScene::update(float timestep, int biome)
{
    _caveButton->setVisible(true);
    _shroomButton->setVisible(true);
    _forestButton->setVisible(true);
    _backButton->setVisible(true);
    _backButton->activate();
    
    
    switch (biome){
        case 3:
            _forestButton->activate();
            _forestButton->setColor(Color4::WHITE);
            _forestButtonBack->setColor(Color4::WHITE);
        case 2:
            _shroomButton->activate();
            _shroomButton->setColor(Color4::WHITE);
            _shroomButtonBack->setColor(Color4::WHITE);
        default:
            _caveButton->activate();
    }
    
    float threshA = 4.0f;
    float threshB = 3.0f;
    float threshC = 5.0f;
    
    
    
    _caveButton->setPositionY(_caveButton->getPositionY() + easing(threshA, _timerA, 0.25));
    _caveButtonBack->setPositionY(_caveButtonBack->getPositionY() + easing(threshA, _timerA, 0.25));

    _shroomButton->setPositionY(_shroomButton->getPositionY() + easing(threshB, _timerB, -0.25));
    _shroomButtonBack->setPositionY(_shroomButtonBack->getPositionY() + easing(threshB, _timerB, -0.25));

    _forestButton->setPositionY(_forestButton->getPositionY() + easing(threshC, _timerC, 0.25));
    _forestButtonBack->setPositionY(_forestButtonBack->getPositionY() + easing(threshC, _timerC, 0.25));
    
    
    _timerA += timestep;
    _timerB += timestep;
    _timerC += timestep;


    if (_timerA > threshA) {
        _timerA = 0.0f;
    }
    
    if (_timerB > threshB) {
        _timerB = 0.0f;
    }
    
    if (_timerC > threshC) {
        _timerC = 0.0f;
    }

}

/**
 * @brief Overrides the Scene2 render to render the scene.
 *
 * @param batch the batch to draw
 */
void WorldSelectScene::render(const std::shared_ptr<cugl::SpriteBatch> &batch)
{
    Scene2::render(batch);
}
