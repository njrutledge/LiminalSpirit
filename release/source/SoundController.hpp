//
//  SoundController.hpp
//  LiminalSpirit
//
//  Created by Luis Enriquez on 3/21/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef SoundController_hpp
#define SoundController_hpp

#include <cugl/cugl.h>
#include "BaseEnemyModel.h"

class SoundController {
    
public:
    
    class LevelMusic {
        
    protected:
        
        string _biome;
        
        std::shared_ptr<cugl::Sound> _themeAsset;
        
        std::shared_ptr<cugl::audio::AudioMixer> _mixer;
        
    public:
        
        LevelMusic();
        
        void init(string biome, std::shared_ptr<cugl::AssetManager> &assets);
        
        void play_music();
        
    };
    
    
    class CharacterSFX {

    protected:
        
        string _character;
        
        std::shared_ptr<cugl::audio::AudioPlayer> _player;
        
        std::shared_ptr<cugl::Sound> _attack;
        
        std::shared_ptr<cugl::Sound> _passive;
        
        std::shared_ptr<cugl::Sound> _hurt;
        
    public:
        
        CharacterSFX();
        
        void init();
    };
    
protected:

    /** Reference to Asset Manager to load sounds */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    std::shared_ptr<LevelMusic> _cave1;
    
    std::shared_ptr<LevelMusic> _cave2;
    
public:
    
    SoundController();
    
    void init(std::shared_ptr<cugl::AssetManager> &assets);
    
    void play_level_music();
};

#endif /* SoundController_hpp */
