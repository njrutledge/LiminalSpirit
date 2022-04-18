//
//  Mirror.hpp
//  Liminal Spirit Game
//
//  This class is the mirrroring crystal enemy
// 
//

#ifndef __MIRROR_HPP__
#define __MIRROR_HPP__

#include <cugl/cugl.h>
#include "BaseEnemyModel.h"


//distance from linked Enemy
#define MIRROR_DISTANCE 2.5f
//amplification power
#define MIRROR_AMPLIFY 2.0f
//attack texture definitions
#define MIRROR_REFLECT_TEXTURE "mirror_reflectattack"
#define MIRROR_REFLECT_ROWS 1
#define MIRROR_REFLECT_COLS 3
//shard textures
#define MIRROR_SHARD_TEXTURE_1 "mirror_shard1"
#define MIRROR_SHARD_TEXTURE_2 "mirror_shard2"
#define MIRROR_SHARD_TEXTURE_3 "mirror_shard3"
#define MIRROR_SHARD_TEXTURE_4 "mirror_shard4"
#define MIRROR_SHARD_TEXTURE_5 "mirror_shard5"
#define MIRROR_SHARD_TEXTURE_6 "mirror_shard6"


#pragma mark - 
#pragma mark Lost

extern struct EnemyProperties MIRROR_PROPS;

class Mirror : public BaseEnemyModel {

public:

	enum class Type {
		square,
		triangle,
		circle
	};

protected:
	//Type of mirror
	Type _type;

	//Enemy linked to the mirror
	std::shared_ptr<BaseEnemyModel> _linkedEnemy;

	/** Time passed since last attack frame(?) */
	float _attackTime;
	/** true if showing attack animation */
	bool _showAttack;

	std::shared_ptr<cugl::scene2::SpriteNode> _attackSprite;

	//three mirror shards
	std::shared_ptr<cugl::scene2::PolygonNode> _shard1;
	std::shared_ptr<cugl::scene2::PolygonNode> _shard2;
	std::shared_ptr<cugl::scene2::PolygonNode> _shard3;

	//three shard timers
	float _shard1Time;
	float _shard2Time;
	float _shard3Time;

	cugl::Vec2 _shard1Positions[4];
	int _shard1Index;
	cugl::Vec2 _shard2Positions[4];
	int _shard2Index;
	cugl::Vec2 _shard3Positions[4];
	int _shard3Index;

	void updateShard(float* shardTime, int* shardIndex, std::shared_ptr<cugl::scene2::PolygonNode> shard, cugl::Vec2 shardPositions[4], float time);
public:

	//override to set the linkedEnemy
	bool init(const cugl::Vec2& pos, const cugl::Size& size, float scale, EnemyProperties props, Mirror::Type type, std::shared_ptr<BaseEnemyModel> enemy);

	//set the linked enemy
	void setLinkedEnemy(std::shared_ptr<BaseEnemyModel> enemy) { _linkedEnemy = enemy; }

	//get the linked enemy
	std::shared_ptr<BaseEnemyModel> getLinkedEnemy() { return _linkedEnemy; }

	Type getType() { return _type; }

	// updates the attack animations
	void updateAnimations(float dt);
	// sets the attack timer
	void setAttackAnimationTimer(float value) { _attackTime = value; }
	// set if the attack should be shown, and reset the attack frame and timer
	void showAttack(bool value) { _showAttack = value; _attackTime = 0; _attackSprite->setFrame(0); }

	//sets the attack sprite
	void setAttackSprite(std::shared_ptr<cugl::scene2::SpriteNode> sprite) {
		_attackSprite = sprite;
	}
	//set the shard sprites
	void setThreeShards(std::shared_ptr<cugl::scene2::PolygonNode> sprite1, std::shared_ptr<cugl::scene2::PolygonNode> sprite2, std::shared_ptr<cugl::scene2::PolygonNode> sprite3) {
		_shard1 = sprite1;
		_shard2 = sprite2;
		_shard3 = sprite3;
	}

#pragma mark - 
#pragma mark Static Constructors
	/** Allocates a new mirror */
	static std::shared_ptr<Mirror> alloc(const cugl::Vec2& pos, const cugl::Size& size, float scale, Mirror::Type type) {
		std::shared_ptr<Mirror> result = std::make_shared<Mirror>();
		return (result->init(pos, size, scale, MIRROR_PROPS, type, nullptr) ? result : nullptr);
	}

	static std::shared_ptr<Mirror> alloc(const cugl::Vec2& pos, const cugl::Size& size, float scale, Mirror::Type type, std::shared_ptr<BaseEnemyModel> enemy) {
		std::shared_ptr<Mirror> result = std::make_shared<Mirror>();
		return (result->init(pos, size, scale, MIRROR_PROPS, type, enemy) ? result : nullptr);
	}

#pragma mark -
#pragma mark Physics Methods
	/** Updates the object's physics state and checks life of linked enemy */
	void update(float dt) override;

};



#endif
