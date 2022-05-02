//
//  LevelSelectScene.hpp
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
#ifndef Level_Select_Scene_hpp
#define Level_Select_Scene_hpp
#include <cugl/cugl.h>
#include "InputController.hpp"


#define CAVE_MAXLEVELS 2
#define SHROOM_MAXLEVELS 9
#define FOREST_MAXLEVELS 2
/**
 * Class representing the Home screen. It links to world select and options menu.
 */
class LevelSelectScene : public cugl::Scene2
{
public:
	enum Choice {
		none,
		button1_prep,
		button2_prep,
		button3_prep,
		forward_prep,
		backward_prep,
		forward,
		backward,
		selected,
		home_prep,
		home
	};
protected:
  /** The loaders to (synchronously) load in assets */
  std::shared_ptr<cugl::AssetManager> _assets;
  /** A scene graph, used to display our 2D scenes */
  std::shared_ptr<cugl::Scene2> _scene;
  /** A 3152 style SpriteBatch to render the scene */
  std::shared_ptr<cugl::SpriteBatch> _batch; // check this

  /** buttons for the world selection */
  std::shared_ptr<cugl::scene2::Button> _button1;
  std::shared_ptr<cugl::scene2::Button> _button2;
  std::shared_ptr<cugl::scene2::Button> _button3;

  std::shared_ptr<cugl::scene2::Button> _buttonHome;
  std::shared_ptr<cugl::scene2::Button> _buttonForward;
  std::shared_ptr<cugl::scene2::Button> _buttonBackward;

  Choice _stageChoice;

  Choice _switchChoice;

  int _stage;

  int _maxStages;

  int _unlockedStages;

  string _biome;
  

public:
  /**
   * Creates a new game mode with the default values.
   *
   * This constructor does not allocate any objects or start the game.
   * This allows us to use the object without a heap pointer.
   */
  LevelSelectScene() : cugl::Scene2() {}

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   *
   * This method is different from dispose() in that it ALSO shuts off any
   * static resources, like the input controller.
   */
  ~LevelSelectScene() { dispose(); }

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
  bool init(const std::shared_ptr<cugl::AssetManager> &assets, string biome);


  void setDefaultChoice() { _stageChoice = Choice::none; _switchChoice = Choice::none; _stage = 0; }

  Choice getChoice() { return _stageChoice; }

  string getBiome() { return _biome; }

  int getStage() { return _stage; }
  //Choice getChoice() { return _choice; }

  //void setDefaultChoice() { _choice = Choice::MENU; }

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

  int getMaxStages(string biome) {
	  if (biome == "cave") {
		  return CAVE_MAXLEVELS;
	  }
	  else if (biome == "shroom") {
		  return SHROOM_MAXLEVELS;
	  }
	  else if (biome == "forest") {
		  return FOREST_MAXLEVELS;
	  }
	  else {
		  return -1;
	  }
  }
};

#endif /* __Level_Select_Scene_hpp__ */
