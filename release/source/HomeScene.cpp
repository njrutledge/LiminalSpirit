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
#include "InputController.hpp"

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
bool HomeScene::init(const std::shared_ptr<cugl::AssetManager>& assets)
{
    _choice = Choice::MENU;
    std::shared_ptr<JsonReader> reader = JsonReader::alloc(Application::get()->getSaveDirectory() + "savedGame.json");
    std::shared_ptr<JsonValue> save = reader->readJson();
    _progress = save->get("progress");
    std::shared_ptr<JsonValue> settings = save->get("settings");
    reader->close();
    _swap = settings->get("swap")->asInt();
    _sfx = settings->get("sfx")->asInt();
    _music = settings->get("music")->asInt();
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
  auto scene = _assets->get<scene2::SceneNode>("main_menu");
  scene->setContentSize(dimen);
  scene->doLayout();

  // You have to attach the individual loaders for each asset type
  _assets->attach<Texture>(TextureLoader::alloc()->getHook());
  _assets->attach<Font>(FontLoader::alloc()->getHook());

  _playButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("main_menu_start"));
  _playButton->clearListeners();
  _playButton->addListener([=](const std::string& name, bool down)
      {
          if (!down) {
              _choice = Choice::PLAY;
              removeChildByName("options");
          }
      });
  _optionsButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("main_menu_options"));
  _optionsButton->clearListeners();
  _optionsButton->addListener([=](const std::string& name, bool down)
      {
          if (!down) {
              _choice = Choice::OPTIONS;
          }
      });
  _creditButton = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("main_menu_credit"));
  _creditButton->clearListeners();
  _creditButton->addListener([=](const std::string& name, bool down)
      {
          if (!down) {
              _choice = Choice::CREDIT;
              removeChildByName("options");
        }
      });
  Rect bounds = Application::get()->getSafeBounds();
  bounds.origin *= boundScale;
  bounds.size *= boundScale;

  float scale = bounds.size.width / 32.0f;


 

  addChild(scene);



  // set buttons here

  _optionScene = _assets->get<scene2::SceneNode>("optionScene");
  _optionScene->setContentSize(dimen);
  _optionScene->doLayout();
  addOptionsButtons(scale);

  addChildWithName(_optionScene, "options");

  return true;
}

void HomeScene::addOptionsButtons(float scale) {
    float buttonScale = scale / 32.0f;

    _optionReturnButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("optionScene_return"));
    _optionReturnButton->clearListeners();
    _optionReturnButton->addListener([=](const std::string& name, bool down)
        {
            // Only quit when the button is released
            if (!down) {
                _choice = Choice::MENU;
            } });
    _optionReturnButton->setScale(.4 * buttonScale);

    _leftText = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("optionScene_text_left"));
    _rightText = std::dynamic_pointer_cast<scene2::Label>(_assets->get<scene2::SceneNode>("optionScene_text_right"));

    _leftText->setScale(.66 * buttonScale);
    _rightText->setScale(.66 * buttonScale);
    _swapHandsButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("optionScene_swap"));
    _swapHandsButton->clearListeners();
    _swapHandsButton->addListener([=](const std::string& name, bool down)
        {
            // Only quit when the button is released
            if (!down) {
                _swap = !_swap;
                this->save();
            } });
    _swapHandsButton->setScale(.4 * buttonScale);
    addMusicButtons(buttonScale);
    addSFXButtons(buttonScale);
}

void HomeScene::addMusicButtons(float buttonScale) {
    _musicButtons.clear();
    for (int i = 1; i <= 10; i++) {
        std::shared_ptr<scene2::Button> button = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("optionScene_musicButton" + std::to_string(i)));
        button->setScale(.4*buttonScale);

        // Create a callback function for the button
        button->setName("music" + i);
        button->clearListeners();
        button->addListener([=](const std::string& name, bool down)
            {
                // Only quit when the button is released
                if (!down) {
                    _music = i;
                } });
        _musicButtons.push_back(button);

    }
}

void HomeScene::addSFXButtons(float buttonScale) {
    _sfxButtons.clear();
    for (int i = 1; i <= 10; i++) {
        std::shared_ptr<scene2::Button> button = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("optionScene_sfxButton" + std::to_string(i)));
        button->setScale(.4 * buttonScale);

        // Create a callback function for the button
        button->setName("sfx" + i);
        button->clearListeners();
        button->addListener([=](const std::string& name, bool down)
            {
                // Only quit when the button is released
                if (!down) {
                    _sfx = i;
                } });
        _sfxButtons.push_back(button);

    }
}

void HomeScene::setDefaultChoice() {
    _choice = Choice::MENU;
    //other stuff to reset HomeScene...didn't want to put it else where sue me - Nick
    Size dimen = Application::get()->getDisplaySize();
    float boundScale = SCENE_WIDTH / dimen.width;
    dimen *= boundScale;

    Rect bounds = Application::get()->getSafeBounds();
    bounds.origin *= boundScale;
    bounds.size *= boundScale;

    _optionScene = _assets->get<scene2::SceneNode>("optionScene");
    _optionScene->setContentSize(dimen);
    _optionScene->doLayout();
    addOptionsButtons(bounds.size.width / 32.0f);

    addChildWithName(_optionScene, "options");
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void HomeScene::dispose()
{
  if(_playButton) _playButton->deactivate();
  _playButton = nullptr;
  if(_optionsButton) _optionsButton->deactivate();
  _optionsButton = nullptr;
  if(_creditButton) _creditButton->deactivate();
  _creditButton = nullptr;
  if (_swapHandsButton) _swapHandsButton->deactivate();
  _swapHandsButton = nullptr;
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
    if (_choice != Choice::OPTIONS) {
        _playButton->setVisible(true);
        _playButton->activate();
        _creditButton->setVisible(true);
        _creditButton->activate();
        _optionsButton->setVisible(true);
        _optionsButton->activate();

        _optionScene->setVisible(false);
        _optionReturnButton->deactivate();
        _swapHandsButton->deactivate();
        for (auto it = _musicButtons.begin(); it != _musicButtons.end(); ++it) {
            (*it)->deactivate();
        }
        for (auto it = _sfxButtons.begin(); it != _sfxButtons.end(); ++it) {
            (*it)->deactivate();
        }
    }
    else {
        _playButton->setVisible(false);
        _playButton->deactivate();
        _optionsButton->setVisible(false);
        _optionsButton->deactivate();
        _creditButton->setVisible(false);
        _creditButton->deactivate();
        _optionScene->setVisible(true);
        _optionReturnButton->setVisible(true);
        _optionReturnButton->activate();
        _swapHandsButton->setVisible(true);
        _swapHandsButton->activate();
        int counter = 1;
        for (auto it = _musicButtons.begin(); it != _musicButtons.end(); ++it) {
            (*it)->activate();
            (*it)->setVisible(true);
            if (counter <= _music) {
                (*it)->setColor(Color4(255, 255, 255));
            }
            else {
                (*it)->setColor(Color4(150, 150, 150));
            }
            counter++;
        }
        counter = 1;
        for (auto it = _sfxButtons.begin(); it != _sfxButtons.end(); ++it) {
            (*it)->activate();
            (*it)->setVisible(true);
            if (counter <= _sfx) {
                (*it)->setColor(Color4(255, 255, 255));
            }
            else {
                (*it)->setColor(Color4(150, 150, 150));
            }
            counter++;
        }
        if (!_swap) {
            _leftText->setText("range");
            _rightText->setText("melee");
        }
        else {
            _leftText->setText("melee");
            _rightText->setText("range");
        }
    }
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

void HomeScene::save() {
    std::shared_ptr<TextWriter> writer = TextWriter::alloc(Application::get()->getSaveDirectory() + "savedGame.json");
    string test = _progress->toString();
    writer->write("{\"progress\":" + _progress->toString() + "settings\":{\"swap\": " + to_string(_swap) +", \"music\": " + to_string(_music) +", \"sfx\": " + to_string(_sfx) +"}}");
    writer->close();
}
