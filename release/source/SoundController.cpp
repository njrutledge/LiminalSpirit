//
//  SoundController.cpp
//  LiminalSpirit
//
//  Created by Luis Enriquez on 3/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

//Fade is determined in Seconds. A fade of 1.5 means the music fades in or out in 1.5 seconds.
#define FADE 1.5
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
    
    _mirror = assets->get<cugl::Sound>(biome + "Mirror");
    _mirror->setVolume(0.0);
    _mNode = _mirror->createNode();
    
    _mixer = cugl::audio::AudioMixer::alloc(8);
    
    _mixer->attach(7, _themeAsset->createNode());
    _mixer->attach(0, _gNode);
    _mixer->attach(1, _pNode);
    _mixer->attach(2, _mNode);
};

/**
 * Plays the music for the specified level class.
 */
void SoundController::LevelMusic::play_music(std::vector<bool> e, GameState s) {
    
    string b = _biome;
    b.pop_back();
    
    
    if ((s != LEVEL_CAVE && b == "cave")  ||  (s != LEVEL_SHROOM && b == "mushroom") || (s != LEVEL_FOREST && b == "forest")) {
        cugl::AudioEngine::get()->getMusicQueue()->clear(0.1);
        cugl::AudioEngine::get()->getMusicQueue()->enqueue(_mixer, true, 0.4);
        
        
    }
    
    for (int i = 0; i < 7; i++) {
        switch (i) {
            case 0:
                if (e[0]) {
                    _gNode->setGain(clampf(_gNode->getGain() + (MAX_LAYER_VOLUME / (FADE * 60.0f)), 0.0f, MAX_LAYER_VOLUME * 1.15f));
                } else {
                    _gNode->setGain(clampf(_gNode->getGain() - (MAX_LAYER_VOLUME / (FADE * 60.0f)), 0.0f, MAX_LAYER_VOLUME * 1.15f));
                }
                break;
            case 1:
                if (e[1]) {
                    _pNode->setGain(clampf(_pNode->getGain() + (MAX_LAYER_VOLUME / (FADE * 60.0f)), 0.0f, MAX_LAYER_VOLUME * 1.1f));
                } else {
                    _pNode->setGain(clampf(_pNode->getGain() - (MAX_LAYER_VOLUME / (FADE * 60.0f)), 0.0f, MAX_LAYER_VOLUME * 1.1f));
                }
                break;
            case 2:
                if (e[2]) {
                    _mNode->setGain(clampf(_mNode->getGain() + (MAX_LAYER_VOLUME / (FADE * 60.0f)), 0.0f, MAX_LAYER_VOLUME * 0.4f));
                } else {
                    _mNode->setGain(clampf(_mNode->getGain() - (MAX_LAYER_VOLUME / (FADE * 60.0f)), 0.0f, MAX_LAYER_VOLUME * 0.4f));
                }
            default:
                break;
        }
    }
    
    
};

void SoundController::LevelMusic::reset_mix() {
    _mixer->reset();
    _gNode->setGain(0.0f);
    _pNode->setGain(0.0f);
    _mNode->setGain(0.0f);
}


SoundController::SoundController(){};


void SoundController::init(std::shared_ptr<cugl::AssetManager> &assets) {
    
    _volume = 1.0f;
    
    _vSFX = 1.0f;
    
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
    
    _forest1 = make_shared<LevelMusic>();
    _forest1->init("forest1", _assets);
    
    _playerStep = assets->get<cugl::Sound>("playerStep");
    
    _playerShoot = assets->get<cugl::Sound>("playerShoot");
    
    _playerShootHit = assets->get<cugl::Sound>("playerShootHit");
    
    _playerSlashEmpty = assets->get<cugl::Sound>("playerSlashEmpty");
    
    _playerSlashHit = assets->get<cugl::Sound>("playerSlashHit");
    
    _playerHurt = assets->get<cugl::Sound>("playerHurt");
    
    _playerExp = assets->get<cugl::Sound>("playerExp");
    
    _playerExpPckg = assets->get<cugl::Sound>("playerExpPck");
    
    _playerDash = assets->get<cugl::Sound>("playerDash");
    
    _playerDashHit = assets->get<cugl::Sound>("playerDashHit");
    
    _playerJump = assets->get<cugl::Sound>("playerJump");
    
    _playerJumpAttack = assets->get<cugl::Sound>("playerJumpAttack");
    
    _playerCharge = assets->get<cugl::Sound>("playerCharge");
    
};

void SoundController::play_menu_music() {
    
    cugl::AudioEngine::get()->getMusicQueue()->setVolume(_volume);
    
    if (_state != MENU) {
        cugl::AudioEngine::get()->getMusicQueue()->clear(0.1);
        cugl::AudioEngine::get()->getMusicQueue()->enqueue(_menu, true, 0.4);
        
        
    }
    _state = MENU;
    
}

void SoundController::play_level_music(string biome, std::vector<bool> enemies) {
    
    cugl::AudioEngine::get()->getMusicQueue()->setVolume(_volume);
    
    if (biome == "cave") {
        if (_state != LEVEL_CAVE) {
            _track = rand()%2;
            reset_level_tracks();
        }
        switch (_track) {
            case 0:
                _cave1->play_music(enemies, _state);
                _state = LEVEL_CAVE;
                break;
            case 1:
                _cave2->play_music(enemies, _state);
                _state = LEVEL_CAVE;
                break;
        }
    } else if (biome == "shroom") {
        if (_state != LEVEL_SHROOM) {
            _track = rand()%2;
            reset_level_tracks();
        }
        switch (_track) {
            case 0:
                _mushroom1->play_music(enemies, _state);
                _state = LEVEL_SHROOM;
                break;
            case 1:
                _mushroom2->play_music(enemies, _state);
                _state = LEVEL_SHROOM;
                break;
        }
        
        
    } else {
        if (_state != LEVEL_FOREST) {
            reset_level_tracks();
        }
            _forest1->play_music(enemies, _state);
            _state = LEVEL_FOREST;
    }
    
    
    
    
};

void SoundController::play_player_sound(playerSType sound) {
    switch (sound) {
        case slashEmpty:
            cugl::AudioEngine::get()->play("playerSlashEmpty", _playerSlashEmpty, false, _vSFX, true);
            break;
        case slashHit:
            cugl::AudioEngine::get()->play("playerSlashEmpty", _playerSlashHit, false, _vSFX, true);
            break;
        case slashDash:
            cugl::AudioEngine::get()->play("playerSDash", _playerDash, false, _vSFX, true);
            break;
        case slashDashHit:
            cugl::AudioEngine::get()->play("playerDashHit", _playerDashHit, false, _vSFX, true);
            break;
        case shoot:
            cugl::AudioEngine::get()->play("playerShoot", _playerShoot, false, _vSFX, true);
            break;
        case shootHit:
            cugl::AudioEngine::get()->play("playerShoot", _playerShootHit, false, _vSFX, true);
            break;
        case shootCharge:
            cugl::AudioEngine::get()->play("playerChargeShoot", _playerExpPckg, false, _vSFX, true);
            break;
        case explosion:
            cugl::AudioEngine::get()->play("playerExp", _playerExp, false, _vSFX, true);
            break;
        case hurt:
            cugl::AudioEngine::get()->play("playerShoot", _playerHurt, false, _vSFX, true);
            break;
        case death:
        case step:
            cugl::AudioEngine::get()->play("playerStep", _playerStep, false, _vSFX, true);
            break;
        case jump:
            cugl::AudioEngine::get()->play("playerJump", _playerJump, false, _vSFX, true);
            break;
        case jumpAttack:
            cugl::AudioEngine::get()->play("playerJumpAttack", _playerJumpAttack, false, _vSFX, true);
            break;
        case charge:
            cugl::AudioEngine::get()->play("playerCharge", _playerCharge, false, _vSFX, true);
            break;
    }
};

void SoundController::reset_level_tracks() {
    _cave1->reset_mix();
    _cave2->reset_mix();
    _mushroom1->reset_mix();
    _mushroom2->reset_mix();
    _forest1->reset_mix();
};

void SoundController::level_transition() {
    
    if (_state != TRANSITION) {
        cugl::AudioEngine::get()->getMusicQueue()->clear(1.0);
    }
    _state = TRANSITION;
    
}

void SoundController::dispose() {
    CULog("kill");
};

