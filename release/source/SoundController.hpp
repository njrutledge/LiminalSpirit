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
        slashDashHit,
        shoot,
        shootHit,
        shootCharge,
        explosion,
        hurt,
        death,
        step,
        jump,
        jumpAttack,
        charge
    };
    
    enum enemy {
        lost,
        phantom,
        mirror,
        seeker,
        glutton,
        spawner
    };
    
    enum etype {
        attack,
        ehurt
    };
    
    enum GameState {
        LOAD,
        MENU,
        PAUSE,
        LEVEL_CAVE,
        LEVEL_SHROOM,
        LEVEL_FOREST,
        TRANSITION
    };
    
    class LevelMusic {
        
    protected:
        
        string _biome;
        
        std::shared_ptr<cugl::Sound> _themeAsset;
        
        std::shared_ptr<cugl::Sound> _glutton;
        std::shared_ptr<cugl::audio::AudioNode> _gNode;
        
        std::shared_ptr<cugl::Sound> _phantom;
        std::shared_ptr<cugl::audio::AudioNode> _pNode;
        
        std::shared_ptr<cugl::Sound> _mirror;
        std::shared_ptr<cugl::audio::AudioNode> _mNode;
        
        std::shared_ptr<cugl::Sound> _spawner;
        std::shared_ptr<cugl::audio::AudioNode> _spNode;
        
        std::shared_ptr<cugl::Sound> _seeker;
        std::shared_ptr<cugl::audio::AudioNode> _sNode;
        
        std::shared_ptr<cugl::audio::AudioMixer> _mixer;
        
    public:
        
        LevelMusic();
        
        void init(string biome, std::shared_ptr<cugl::AssetManager> &assets);
        
        void play_music(std::vector<bool> e, SoundController::GameState s);
        
        std::shared_ptr<cugl::audio::AudioMixer> getMixer() {return _mixer;}
        
        void reset_mix();
        
    };
    
    
    class EnemySFX {

    protected:
        
        string _enemy;
        
        std::shared_ptr<cugl::Sound> _attack;
        
        std::shared_ptr<cugl::Sound> _hurt;
        
    public:
        
        EnemySFX();
        
        void init(string enemy, std::shared_ptr<cugl::AssetManager> &assets);
        
        void play_sound(etype t, float vol);
    };
    
protected:
    
    GameState _state;

    /** Reference to Asset Manager to load sounds */
    std::shared_ptr<cugl::AssetManager> _assets;
    
    std::shared_ptr<LevelMusic> _cave1;
    
    std::shared_ptr<LevelMusic> _cave2;
    
    std::shared_ptr<LevelMusic> _mushroom1;
    
    std::shared_ptr<LevelMusic> _mushroom2;
    
    std::shared_ptr<LevelMusic> _forest1;
    
    std::shared_ptr<cugl::Sound> _menu;
    
    int _track;
    
    //from 0 to 1 for music
    float _volume;
    
    //from 0 to 1 for sfx
    float _vSFX;
    
    // Player Sounds
    
    std::vector<std::shared_ptr<cugl::audio::AudioScheduler>> _player_sfx_slots;
    
    std::shared_ptr<cugl::Sound> _playerShoot;
    
    std::shared_ptr<cugl::Sound> _playerStep;
    
    std::shared_ptr<cugl::Sound> _playerSlashEmpty;
    
    std::shared_ptr<cugl::Sound> _playerSlashHit;
    
    std::shared_ptr<cugl::Sound> _playerShootHit;
    
    std::shared_ptr<cugl::Sound> _playerHurt;
    
    std::shared_ptr<cugl::Sound> _playerExp;
    
    std::shared_ptr<cugl::Sound> _playerExpPckg;
    
    std::shared_ptr<cugl::Sound> _playerDash;
    
    std::shared_ptr<cugl::Sound> _playerDashHit;
    
    std::shared_ptr<cugl::Sound> _playerJump;
    
    std::shared_ptr<cugl::Sound> _playerJumpAttack;
    
    std::shared_ptr<cugl::Sound> _playerCharge;
    
    std::shared_ptr<cugl::Sound> _enemyDeath;
    
    std::shared_ptr<cugl::Sound> _mirrorDeath;
    
    std::shared_ptr<EnemySFX> _lost;
    
    std::shared_ptr<EnemySFX> _phantom;
    
    std::shared_ptr<EnemySFX> _mirror;
    
    std::shared_ptr<EnemySFX> _glutton;
    
    std::shared_ptr<EnemySFX> _seeker;
    
    std::shared_ptr<EnemySFX> _spawner;
    
    
public:
    
    SoundController();
    
    void init(std::shared_ptr<cugl::AssetManager> &assets);
    
    void play_menu_music();
    
    void play_level_music(string biome, std::vector<bool> enemies);
    
    void play_player_sound(playerSType sound);
    
    void reset_level_tracks();
    
    void level_transition();
    
    void play_death_sound(bool mirror);
    
    void play_enemy_sound(enemy e, etype t);
    
    /**
     * Sets music volume
     *
     * @param vol   volume to set. Must be in range 0 - 1
     */
    void set_music_volume(float vol) {_volume = vol;};
    
    /**
     * Sets sfx volume
     *
     * @param vol   volume to set. Must be in range 0 - 1
     */
    void set_sfx_volume(float vol) {_vSFX = vol;}
    
    /**
            Kill
     */
    void dispose();
};

#endif /* SoundController_hpp */
