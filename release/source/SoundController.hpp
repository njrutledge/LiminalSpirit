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
        slashEmpty,
        slashHit,
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
    
    enum GameState {
        LOAD,
        MENU,
        PAUSE,
        LEVEL
    };
    
    class LevelMusic {
        
    protected:
        
        string _biome;
        
        std::shared_ptr<cugl::Sound> _themeAsset;
        
        std::shared_ptr<cugl::Sound> _glutton;
        std::shared_ptr<cugl::audio::AudioNode> _gNode;
        
        std::shared_ptr<cugl::Sound> _phantom;
        std::shared_ptr<cugl::audio::AudioNode> _pNode;
        
        std::shared_ptr<cugl::audio::AudioMixer> _mixer;
        
    public:
        
        LevelMusic();
        
        void init(string biome, std::shared_ptr<cugl::AssetManager> &assets);
        
        void play_music(std::vector<bool> e, SoundController::GameState s);
        
        std::shared_ptr<cugl::audio::AudioMixer> getMixer() {return _mixer;}
        
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
    
    GameState _state;

    /** Reference to Asset Manager to load sounds */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    std::shared_ptr<LevelMusic> _cave1;
    
    std::shared_ptr<LevelMusic> _cave2;
    
    std::shared_ptr<LevelMusic> _mushroom1;
    
    std::shared_ptr<LevelMusic> _mushroom2;
    
    std::shared_ptr<cugl::Sound> _menu;
    
    int _track;
    
    // Player Sounds
    
    std::vector<std::shared_ptr<cugl::audio::AudioScheduler>> _player_sfx_slots;
    
    std::shared_ptr<cugl::Sound> _playerShoot;
    
    std::shared_ptr<cugl::Sound> _playerStep;
    
    std::shared_ptr<cugl::Sound> _playerSlashEmpty;
    
    std::shared_ptr<cugl::Sound> _playerSlashHit;
    
public:
    
    SoundController();
    
    void init(std::shared_ptr<cugl::AssetManager> &assets);
    
    void play_menu_music();
    
    void play_level_music(string biome, std::vector<bool> enemies);
    
    void play_player_sound(playerSType sound);
    
    void reset_level_tracks();
    
    /**
            Kill
     */
    void dispose();
};

#endif /* SoundController_hpp */
