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

public:

	//override to set the linkedEnemy
	bool init(const cugl::Vec2& pos, const cugl::Size& size, float scale, EnemyProperties props, Mirror::Type type, std::shared_ptr<BaseEnemyModel> enemy);

	//set the linked enemy
	void setLinkedEnemy(std::shared_ptr<BaseEnemyModel> enemy) { _linkedEnemy = enemy; }

	//get the linked enemy
	std::shared_ptr<BaseEnemyModel> getLinkedEnemy() { return _linkedEnemy; }

	Type getType() { return _type; }
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
