//
//  SoundController.cpp
//  LiminalSpirit
//
//  Created by Luis Enriquez on 3/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//


#include "SoundController.hpp"

SoundController::LevelMusic::LevelMusic() {};

/**
 * Initializes the level music class with the name and assets. Also creates the mixer and attaches the audio assets to the mixer.
 * @param biome     The name of the biome along with a number representing which track it is
 * @param assets   The Asset manager to get the appropriate sound files
 */
void SoundController::LevelMusic::init(string biome, std::shared_ptr<cugl::AssetManager> &assets) {
    _biome = biome;
    _themeAsset = assets->get<cugl::Sound>(biome);
    
    _mixer = cugl::audio::AudioMixer::alloc(8);
    
    _mixer->attach(1, _themeAsset->createNode());
};

/**
 * Plays the music for the specified level class.
 */
void SoundController::LevelMusic::play_music() {
    
    cugl::AudioEngine::get()->getMusicQueue()->advance(0, 0.5);
    cugl::AudioEngine::get()->getMusicQueue()->enqueue(_mixer, true);
    
};

SoundController::SoundController(){};

void SoundController::init(std::shared_ptr<cugl::AssetManager> &assets) {
    
    _state = LOAD;
    
    _assets = assets;
    
    _menu = assets->get<cugl::Sound>("menu");
    
    _cave1 = make_shared<LevelMusic>();
    _cave1->init("cave1", _assets);
    
    _cave2 = make_shared<LevelMusic>();
    _cave2->init("cave2", _assets);
    
    _mushroom1 = make_shared<LevelMusic>();
    _mushroom1->init("mushroom1", _assets);
    
    _playerStep = assets->get<cugl::Sound>("playerStep");
    _playerShoot = assets->get<cugl::Sound>("playerShoot");
    _playerSlashEmpty = assets->get<cugl::Sound>("playerSlashEmpty");
    _playerSlashHit = assets->get<cugl::Sound>("playerSlashHit");
};

void SoundController::play_menu_music() {
    
    if (_state != MENU) {
        _state = MENU;
        cugl::AudioEngine::get()->getMusicQueue()->advance(0, 0.5);
        cugl::AudioEngine::get()->getMusicQueue()->enqueue(_menu, true);
    }

}

void SoundController::play_level_music(string biome) {
    
    int r = rand()%3;
    
    if (_state != LEVEL) {
        _state = LEVEL;
        if (r == 0) {
            _cave1->play_music();
        } else if (r == 1) {
            _cave2->play_music();
        }else {
            _mushroom1->play_music();
        }
        
    }
    
};

void SoundController::play_player_sound(playerSType sound) {
    switch (sound) {
        case slashEmpty:
            cugl::AudioEngine::get()->play("playerSlashEmpty", _playerSlashEmpty, false, 1.0, true);
            break;
        case slashHit:
            cugl::AudioEngine::get()->play("playerSlashEmpty", _playerSlashHit, false, 1.0, true);
            break;
        case slashDash:
        case shoot:
            cugl::AudioEngine::get()->play("playerShoot", _playerShoot, false, 1.0, true);
            break;
        case shootCharge:
        case hurt:
        case death:
        case step:
            cugl::AudioEngine::get()->play("playerStep", _playerStep, false, 0.5, true);
            break;
        case jump:
        case chargeP:
        case chargeM:
            break;
    }
};
