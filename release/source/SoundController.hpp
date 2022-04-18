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

class SoundController {
    
public:
    
    enum playerSType {
        slash,
        slashFinal,
        slashDash,
        shoot,
        shootCharge,
        hurt,
        death,
        step,
        jump,
        chargeP,
        chargeM
    };
    
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
    
    
    class EnemySFX {

    protected:
        
        string _enemy;
        
        std::shared_ptr<cugl::audio::AudioPlayer> _player;
        
        std::shared_ptr<cugl::Sound> _attack;
        
        std::shared_ptr<cugl::Sound> _death;
        
        std::shared_ptr<cugl::Sound> _hurt;
        
    public:
        
        EnemySFX();
        
        void init();
    };
    
protected:

    /** Reference to Asset Manager to load sounds */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    std::shared_ptr<LevelMusic> _cave1;
    
    std::shared_ptr<LevelMusic> _cave2;
    
    // Player Sounds
    
    std::shared_ptr<cugl::Sound> _playerShoot;
    
    std::shared_ptr<cugl::Sound> _playerStep;
    
    std::shared_ptr<cugl::Sound> _playerSlash;
public:
    
    SoundController();
    
    void init(std::shared_ptr<cugl::AssetManager> &assets);
    
    void play_level_music();
    
    void play_player_sound(playerSType sound);
};

#endif /* SoundController_hpp */
