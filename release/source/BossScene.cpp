//
//  BossScene.cpp
//  Cornell University Game Library (CUGL)
//
//  Root scene to manage the gameplay
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
// Include the class header, which includes all of the CUGL classes
#include "BossScene.hpp"

#include <cugl/base/CUBase.h>
#include <box2d/b2_contact.h>

#include "BaseEnemyModel.h"
#include "Lost.hpp"
#include "Phantom.hpp"
#include "Mirror.hpp"

#include "PlayerModel.h"
#include "Platform.hpp"

#include "AttackController.hpp"
#include "AIController.hpp"
#include "CollisionController.hpp"

#include "Glow.hpp"


#include "RRParticle.h"
#include "RRParticleNode.h"
#include "RRParticlePool.h"

// Add support for simple random number generation
#include <cstdlib>
#include <ctime>

// This keeps us from having to write cugl:: all the time
using namespace cugl;

/**
 * Initializes the controller contents, and starts the game
 *
 * The constructor does not allocate any objects or memory.  This allows
 * us to have a non-pointer reference to this controller, reducing our
 * memory allocation.  Instead, allocation happens in this method.
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool BossScene::init(const std::shared_ptr<cugl::AssetManager>& assets, const std::shared_ptr<SoundController> sound)
{
    _seedBarrageTimer = 0;
    return GameScene::init(assets, sound, "BOSS");
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void BossScene::dispose()
{
    GameScene::dispose();
}

void BossScene::update(float timestep)
{
    updateSoundInputParticlesAndTilt(timestep);

    updateAnimations(timestep);

    updateEnemies(timestep);


    _seedBarrageTimer += timestep;
    if (_seedBarrageTimer > 2.0f) {
        seedBarrage();
        _seedBarrageTimer = 0;
    }


    updateSwipesAndAttacks(timestep);

    

    updateRemoveDeletedAttacks();

    updateRemoveDeletedEnemies();

    //updateText();

    updateSpawnTimes();

    updateRemoveDeletedPlayer();

    updateHealthbar();

    updateCamera();

    updateSpawnEnemies(timestep);

    updateMeleeArm(timestep);

    //updateWin();
}

void BossScene::seedBarrage() {
    int seeds = rand() % 5 + 10;
    float radius = 0.5f;
    float damage = 1.0f;
    Vec2 play_p = _player->getPosition();
    Vec2 en_p = Vec2(16.0, 4.0);
    for (int ii = 0; ii < seeds; ii++) {
        float age = 3.0f;//rand() % 20 / 5;
        float random = rand() % 50;
        Vec2 vel = Vec2(0.5, 0).scale(.2 + random / 75);
        random = rand() % 100;
        vel = vel.rotate((play_p - en_p).getAngle()).rotate((M_PI / 6.0f * (random) / 100) - M_PI / 12);
        _attacks->createAttack(en_p, radius, age, damage, AttackController::Type::e_range, vel, 0, "seed", 1);
    }
}

/**
 * The method called to draw the gameplay scene
 */
void BossScene::render(const std::shared_ptr<cugl::SpriteBatch>& batch)
{
    GameScene::render(batch);
}
