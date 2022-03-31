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
    
    std::shared_ptr<cugl::AudioQueue> q = cugl::AudioEngine::get()->getMusicQueue();
    
    if (q->getState() == cugl::AudioEngine::State::INACTIVE) {
       q->play(_mixer, true);
    }
    
};

SoundController::SoundController(){};

void SoundController::init(std::shared_ptr<cugl::AssetManager> &assets) {
    _assets = assets;
    
    _cave1 = make_shared<LevelMusic>();
    _cave1->init("cave2", _assets);
    
    _cave2 = make_shared<LevelMusic>();
    _cave2->init("cave2", _assets);
    
};

void SoundController::play_level_music() {
    
    _cave2->play_music();
    
}
