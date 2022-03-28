//
//  HomeScene.cpp
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
#include "HomeScene.hpp"
#include <cugl/cugl.h>
#include "controllers/InputController.hpp"

using namespace cugl;

/** This is the size of the active portion of the screen */
#define SCENE_WIDTH 1024

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
bool HomeScene::init(const std::shared_ptr<cugl::AssetManager> &assets)
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
  auto scene = _assets->get<scene2::SceneNode>("menu");
  scene->setContentSize(dimen);
  scene->doLayout();

  // You have to attach the individual loaders for each asset type
  _assets->attach<Texture>(TextureLoader::alloc()->getHook());
  _assets->attach<Font>(FontLoader::alloc()->getHook());

  // set buttons here
  addChild(scene);
  return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void HomeScene::dispose()
{
  _batch = nullptr;
  _assets = nullptr;
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
void HomeScene::update(float timestep)
{
  // Update input controller
  _input.update();
}

/**
 * @brief Overrides the Scene2 render to render the scene.
 *
 * @param batch the batch to draw
 */
void HomeScene::render(const std::shared_ptr<cugl::SpriteBatch> &batch)
{
  Scene2::render(batch);
}