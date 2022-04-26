//
//  SoundController.cpp
//  LiminalSpirit
//
//  Created by Luis Enriquez on 3/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#define FADE 0.003
#define MAX_LAYER_VOLUME 0.3

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
    
    _glutton = assets->get<cugl::Sound>(biome + "Glutton");
    _glutton->setVolume(0.0);
    _gNode = _glutton->createNode();
    
    _phantom = assets->get<cugl::Sound>(biome + "Phantom");
    _phantom->setVolume(0.0);
    _pNode = _phantom->createNode();
    
    _mixer = cugl::audio::AudioMixer::alloc(8);
    
    _mixer->attach(7, _themeAsset->createNode());
    _mixer->attach(0, _gNode);
    _mixer->attach(1, _pNode);
};

/**
 * Plays the music for the specified level class.
 */
void SoundController::LevelMusic::play_music(std::vector<bool> e, GameState s) {
    
    if (s != LEVEL) {
        cugl::AudioEngine::get()->getMusicQueue()->advance(0, 0.2);
        cugl::AudioEngine::get()->getMusicQueue()->enqueue(_mixer, true, 0.3);
    }
    
    for (int i = 0; i < 7; i++) {
        switch (i) {
            case 0:
                if (e[0]) {
                    _gNode->setGain(clampf(_gNode->getGain() + FADE, 0.0f, MAX_LAYER_VOLUME));
                } else {
                    _gNode->setGain(clampf(_gNode->getGain() - FADE, 0.0f, MAX_LAYER_VOLUME));
                }
                break;
            case 1:
                if (e[1]) {
                    _pNode->setGain(clampf(_pNode->getGain() + FADE, 0.0f, MAX_LAYER_VOLUME * 1.3f));
                } else {
                    _pNode->setGain(clampf(_pNode->getGain() - FADE, 0.0f, MAX_LAYER_VOLUME * 1.3f));
                }
                break;
            default:
                break;
        }
    }
    
    
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
    
    _mushroom2 = make_shared<LevelMusic>();
    _mushroom2->init("mushroom2", _assets);
    
    _playerStep = assets->get<cugl::Sound>("playerStep");
    _playerShoot = assets->get<cugl::Sound>("playerShoot");
    
    _playerSlashEmpty = assets->get<cugl::Sound>("playerSlashEmpty");
    
    _playerSlashHit = assets->get<cugl::Sound>("playerSlashHit");
    
    //int playerslots = 12;
    //_playerMixer = cugl::audio::AudioMixer::alloc(playerslots);
    
    //for (int i = 0; i < playerslots; i++) {
    //    std::shared_ptr<cugl::audio::AudioScheduler> s = cugl::audio::AudioScheduler::alloc(2, 48000);//
    //    _playerMixer->attach(i, s);
    //    _player_sfx_slots.emplace_back(s);
    //}
    //cugl::AudioEngine::get()->play("playerMixer", _playerMixer);
};

void SoundController::play_menu_music() {
    
    if (_state != MENU) {
        _state = MENU;
        cugl::AudioEngine::get()->getMusicQueue()->advance(0, 0.2);
        cugl::AudioEngine::get()->getMusicQueue()->enqueue(_menu, true, 0.4);
    }
    
}

void SoundController::play_level_music(string biome, std::vector<bool> enemies) {
    
    if (biome == "cave") {
        if (_state != LEVEL) {
            _track = rand()%2;
            reset_level_tracks();
        }
        switch (_track) {
            case 0:
                _cave1->play_music(enemies, _state);
                _state = LEVEL;
                break;
            case 1:
                _cave2->play_music(enemies, _state);
                _state = LEVEL;
                break;
        }
    } else if (biome == "shroom") {
        if (_state != LEVEL) {
            _track = rand()%2;
            reset_level_tracks();
        }
        switch (_track) {
            case 0:
                _mushroom1->play_music(enemies, _state);
                _state = LEVEL;
                break;
            case 1:
                _mushroom2->play_music(enemies, _state);
                _state = LEVEL;
                break;
        }
        
        
    } else {
        
    }
    
    
    
};

void SoundController::play_player_sound(playerSType sound) {
    switch (sound) {
        case slashEmpty:
            cugl::AudioEngine::get()->play("playerSlashEmpty", _playerSlashEmpty, false, 1.0, true);
            //_player_sfx_slots[0]->trim();
           // _player_sfx_slots[0]->play(_playerSlashEmpty->createNode());
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
            cugl::AudioEngine::get()->play("playerStep", _playerStep, false, 1.0, true);
            break;
        case jump:
        case chargeP:
        case chargeM:
            break;
    }
};

void SoundController::reset_level_tracks() {
    _cave1->getMixer()->reset();
    _cave2->getMixer()->reset();
    _mushroom1->getMixer()->reset();
    _mushroom2->getMixer()->reset();
    
};
