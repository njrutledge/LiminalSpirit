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
    if (_caveButton) _caveButton->deactivate();
    _caveButton = nullptr;
    if (_shroomButton) _shroomButton->deactivate();
    _shroomButton = nullptr;
    if (_forestButton) _forestButton->deactivate();
    _forestButton = nullptr;
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

    // You have to attach the individual loaders for each asset type
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<Font>(FontLoader::alloc()->getHook());

    Rect bounds = Application::get()->getSafeBounds();
    bounds.origin *= boundScale;
    bounds.size *= boundScale;

    _caveButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("world_select_cave"));
    _caveButton->setPositionX(bounds.getMinX() + _caveButton->getPositionX());
    _caveButton->addListener([=](const std::string& name, bool down)
        {
            if (down) {
                _choice = Choice::CAVE_PREP;
            }
            else if (_choice == Choice::CAVE_PREP) {
                _choice = Choice::CAVE;
            }
        });

    _caveButtonBack = assets->get<scene2::SceneNode>("world_select_caveback");
    _caveButtonBack->setPositionX(bounds.getMinX() + _caveButtonBack->getPositionX());

    _shroomButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("world_select_shroom"));
    _shroomButton->addListener([=](const std::string& name, bool down)
        {
            if (down) {
                _choice = Choice::SHROOM_PREP;
            }
            else if (_choice == Choice::SHROOM_PREP) {
                _choice = Choice::SHROOM;
            }
        });

    _shroomButtonBack = assets->get<scene2::SceneNode>("world_select_shroomback");

    _forestButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("world_select_forest"));
    _forestButton->addListener([=](const std::string& name, bool down)
        {
            if (down) {
                _choice = Choice::FOREST_PREP;
            }
            else if (_choice == Choice::FOREST_PREP) {
                _choice = Choice::FOREST;
            }
        });

    _forestButtonBack = assets->get<scene2::SceneNode>("world_select_forestback");

    _timer = 0;


    addChild(scene);
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
 */
void WorldSelectScene::update(float timestep)
{
    _caveButton->setVisible(true);
    _caveButton->activate();
    _shroomButton->setVisible(true);
    _shroomButton->activate();
    _forestButton->setVisible(true);
    _forestButton->activate();

    _timer += timestep;
    int amplitude = 6;
    if (_timer <= .5) {
        _caveButton->setPositionY(_caveButton->getPositionY() + timestep * amplitude);
        _caveButtonBack->setPositionY(_caveButtonBack->getPositionY() + timestep * amplitude);

        _shroomButton->setPositionY(_shroomButton->getPositionY() + timestep * amplitude);
        _shroomButtonBack->setPositionY(_shroomButtonBack->getPositionY() + timestep * amplitude);

        _forestButton->setPositionY(_forestButton->getPositionY() - timestep * amplitude);
        _forestButtonBack->setPositionY(_forestButtonBack->getPositionY() - timestep * amplitude);
    }
    else if (_timer <= 1) {
        _caveButton->setPositionY(_caveButton->getPositionY() - timestep * amplitude);
        _caveButtonBack->setPositionY(_caveButtonBack->getPositionY() - timestep * amplitude);

        _shroomButton->setPositionY(_shroomButton->getPositionY() + timestep * amplitude);
        _shroomButtonBack->setPositionY(_shroomButtonBack->getPositionY() + timestep * amplitude);

        _forestButton->setPositionY(_forestButton->getPositionY() + timestep * amplitude);
        _forestButtonBack->setPositionY(_forestButtonBack->getPositionY() + timestep * amplitude);
    }
    else if (_timer <= 1.5) {
        _caveButton->setPositionY(_caveButton->getPositionY() - timestep * amplitude);
        _caveButtonBack->setPositionY(_caveButtonBack->getPositionY() - timestep * amplitude);

        _shroomButton->setPositionY(_shroomButton->getPositionY() - timestep * amplitude);
        _shroomButtonBack->setPositionY(_shroomButtonBack->getPositionY() - timestep * amplitude);

        _forestButton->setPositionY(_forestButton->getPositionY() + timestep * amplitude);
        _forestButtonBack->setPositionY(_forestButtonBack->getPositionY() + timestep * amplitude);
    }
    else if (_timer <= 2.0) {
        _caveButton->setPositionY(_caveButton->getPositionY() + timestep * amplitude);
        _caveButtonBack->setPositionY(_caveButtonBack->getPositionY() + timestep * amplitude);

        _shroomButton->setPositionY(_shroomButton->getPositionY() - timestep * amplitude);
        _shroomButtonBack->setPositionY(_shroomButtonBack->getPositionY() - timestep * amplitude);

        _forestButton->setPositionY(_forestButton->getPositionY() - timestep * amplitude);
        _forestButtonBack->setPositionY(_forestButtonBack->getPositionY() - timestep * amplitude);
    }

    if (_timer > 2.0) {
        _timer = 0;
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
