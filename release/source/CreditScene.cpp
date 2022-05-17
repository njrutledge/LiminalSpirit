//
//  CreditScene.cpp
//  LiminalSpirit
//
//  Created by Chenyu Wei on 5/16/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#include "CreditScene.hpp"
using namespace cugl;
#define SCENE_WIDTH 1024
/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void CreditScene::dispose()
{
    if (_homeButton) _homeButton->deactivate();
    _homeButton = nullptr;
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
bool CreditScene::init(const std::shared_ptr<cugl::AssetManager> &assets)
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
    auto scene = _assets->get<scene2::SceneNode>("creditScene");
    scene->setContentSize(dimen);
    scene->doLayout();
    addChild(scene);
    // You have to attach the individual loaders for each asset type
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<Font>(FontLoader::alloc()->getHook());
    std::shared_ptr<Texture> up = _assets->get<Texture>("close-normal");
    Rect bounds = Application::get()->getSafeBounds();
    bounds.origin *= boundScale;
    bounds.size *= boundScale;


    _homeButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("creditScene_home"));
    _homeButton->setPositionY(bounds.getMaxY() - _homeButton->getHeight()/2);
    _homeButton->setPositionX(bounds.getMinX() + _homeButton->getPositionX());
    _homeButton->clearListeners();
    _homeButton->addListener([=](const std::string &name, bool down)
                             {
        if (down) {
            _homeChoice = Choice::HOME_PREP;
        }
        else if (_homeChoice == Choice::HOME_PREP) {
            _homeChoice = Choice::HOME;
        }
    });
    _homeButton->activate();
    _creditText = _assets->get<scene2::SceneNode>("creditText");
    dimen.width = dimen.width/2;
    _creditText->setContentSize(dimen);
    _creditText->setPositionX(bounds.getMidX());
    _creditText->doLayout();

    
    addChildWithName(_creditText, "credit");
    return true;
}

/**
 * @brief Overrides the Scene2 render to render the scene.
 *
 * @param batch the batch to draw
 */
void CreditScene::render(const std::shared_ptr<cugl::SpriteBatch> &batch)
{
    Scene2::render(batch);
}
