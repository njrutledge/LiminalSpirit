//
//  BossScene.hpp
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
#ifndef BossScene_hpp
#define BossScene_hpp
#include "GameScene.hpp"
/**
 * Class for a simple Hello World style application
 *
 * The application simply moves the CUGL logo across the screen.  It also
 * provides a button to quit the application.
 */
class BossScene : public GameScene
{
protected:

public:
    /**
     * Creates a new game mode with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    BossScene() : GameScene() {}

    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~BossScene() { dispose(); }

    void dispose();

    bool init(const std::shared_ptr<cugl::AssetManager>& assets, const std::shared_ptr<SoundController> sound);

    void update(float timestep);
    /**
    *  The method called to draw the gameplay scene
    */
    void render(const std::shared_ptr<cugl::SpriteBatch>& batch);

   
};

#endif /* __Boss_Scene_hpp__ */
