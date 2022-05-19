//
//  HomeScene.hpp
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
#ifndef HomeScene_hpp
#define HomeScene_hpp
#include <cugl/cugl.h>
#include "InputController.hpp"
#include "SoundController.hpp"

/**
 * Class representing the Home screen. It links to world select and options menu.
 */
class HomeScene : public cugl::Scene2
{
public:
	enum class Choice {
		MENU,
		PLAY,
		OPTIONS,
        CREDIT,
	};
protected:

  std::shared_ptr<SoundController> _sound;

  /** The loaders to (synchronously) load in assets */
  std::shared_ptr<cugl::AssetManager> _assets;
  /** A scene graph, used to display our 2D scenes */
  std::shared_ptr<cugl::Scene2> _scene;
  /** A 3152 style SpriteBatch to render the scene */
  std::shared_ptr<cugl::SpriteBatch> _batch; // check this

  std::shared_ptr<cugl::scene2::Button> _playButton;
  std::shared_ptr<cugl::scene2::Button> _optionsButton;
  std::shared_ptr<cugl::scene2::Button> _creditButton;
    
  std::shared_ptr<cugl::scene2::Label> _leftText;
  std::shared_ptr<cugl::scene2::Label> _rightText;

  std::shared_ptr<cugl::scene2::Button> _optionReturnButton;

  std::shared_ptr<cugl::scene2::Button> _swapHandsButton;

  std::vector<std::shared_ptr<cugl::scene2::Button>> _musicButtons;

  std::vector<std::shared_ptr<cugl::scene2::Button>> _sfxButtons;



  std::shared_ptr<cugl::scene2::SceneNode> _optionScene;
  /** the player choice of this menu */
  Choice _choice = Choice::MENU;

  /** Whether or not the player's hands are swapped */
  bool _swap;

  int _music;

  int _sfx;

  std::shared_ptr<cugl::JsonValue> _progress;

  void save();

public:
  /**
   * Creates a new game mode with the default values.
   *
   * This constructor does not allocate any objects or start the game.
   * This allows us to use the object without a heap pointer.
   */
  HomeScene() : cugl::Scene2() {}

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   *
   * This method is different from dispose() in that it ALSO shuts off any
   * static resources, like the input controller.
   */
  ~HomeScene() { dispose(); }

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  void dispose();

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
  bool init(const std::shared_ptr<cugl::AssetManager> &assets);

  Choice getChoice() { return _choice; };

  void setDefaultChoice();

  void addOptionsButtons(float scale);

  void addMusicButtons(float buttonScale);

  void addSFXButtons(float buttonScale);

  void setSoundController(std::shared_ptr<SoundController> sound) {
	  _sound = sound;
	  _sound->set_music_volume(_music/10.0f);
	  _sound->set_sfx_volume(_sfx/10.0f);
  };

#pragma mark -
#pragma mark Screen Handling
  /**
   * The method called to update the screen mode.
   *
   * There is not much we need to do here unless a button is pressed
   *
   * @param timestep  The amount of time (in seconds) since the last frame
   */
  void update(float timestep);

  /**
   * @brief Overrides the Scene2 render to render the scene.
   *
   * @param batch the batch to draw
   */
  void render(const std::shared_ptr<cugl::SpriteBatch> &batch);
};

#endif /* __Home_Scene_hpp__ */
