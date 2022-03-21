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
    
    /** Reference to Asset Manager to load sounds */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** The JSON value with all of the constants */
    std::shared_ptr<cugl::JsonValue> _constants;
    
    SoundController();
    
    void init();
};

#endif /* SoundController_hpp */
