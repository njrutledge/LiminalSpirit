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
#include "controllers/InputController.hpp"

using namespace cugl;

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void WorldSelectScene::dispose()
{
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
}

/**
 * @brief Overrides the Scene2 render to render the scene.
 *
 * @param batch the batch to draw
 */
void WorldSelectScene::render(const std::shared_ptr<cugl::SpriteBatch> &batch)
{
}
