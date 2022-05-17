//
//  CreditScene.hpp
//  LiminalSpirit
//
//  Created by Chenyu Wei on 5/16/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef CreditScene_hpp
#define CreditScene_hpp

#include <stdio.h>
#include <cugl/cugl.h>
#include "InputController.hpp"

class CreditScene : public cugl::Scene2
{
public:
    enum class Choice {
        none,
        HOME,
        HOME_PREP
    };
protected:
  /** The loaders to (synchronously) load in assets */
  std::shared_ptr<cugl::AssetManager> _assets;
  /** A scene graph, used to display our 2D scenes */
  std::shared_ptr<cugl::Scene2> _scene;
  /** A 3152 style SpriteBatch to render the scene */
  std::shared_ptr<cugl::SpriteBatch> _batch; // check this

  std::shared_ptr<cugl::scene2::Button> _homeButton;
    
  std::shared_ptr<cugl::scene2::SceneNode> _creditText;

  Choice _homeChoice;

public:

  CreditScene() : cugl::Scene2() {}

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   *
   * This method is different from dispose() in that it ALSO shuts off any
   * static resources, like the input controller.
   */
  ~CreditScene() { dispose(); }

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

    void setDefaultChoice()
    {
      _homeChoice = Choice::none;
    }

    Choice getChoice() { return _homeChoice; }

  /**
   * @brief Overrides the Scene2 render to render the scene.
   *
   * @param batch the batch to draw
   */
  void render(const std::shared_ptr<cugl::SpriteBatch> &batch);
};

#endif /* CreditScene_hpp */
