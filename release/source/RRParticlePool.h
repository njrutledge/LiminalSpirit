//
//  RRParticlePool.h
//  Created by Zach of Ragdoll Royale (2022)
//  Adapted with his permission by Alex Lee for use in Liminal Spirit
// 
//

#ifndef __NL_PARTICLE_POOL_H__
#define __NL_PARTICLE_POOL_H__
#include "RRParticle.h"
#include <cmath>

class ParticlePool {
protected:
	// The maximum number of objects in this pool
	int _capacity;
	// The emission rate this second
	int _currentEmissionRate;
	// Timer to keep track of emission rate
	float _timer;
	// Whether or not the particle system is a temporary burst
	bool _burst;
	// If the particle system is complete or not. This only happens if it is a burst.
	bool _complete;
	// The number of dead particles in the system. This is only used in a burst system to check if we should mark the system complete or not.
	int _deadparticles;

	// PARTICLE VALUES

	// The gravity effecting each particle
	cugl::Vec2 _gravity;
	// The range of emission rate values. This determines how many particles are emmitted at any given point.
	cugl::Vec2 _emissionRateRange;
	// The range of particle lifetimes
	cugl::Vec2 _lifetimeRange;
	// The range of particle angles
	cugl::Vec2 _angleRange;
	// The range of particle speeds
	cugl::Vec2 _speedRange;
	// The range of particle fade in times
	cugl::Vec2 _fadeinRange;
	// The size range of the of the particles in the pool
	cugl::Vec2 _sizeRange;
	// The rates of change of the particles size in the pool
	cugl::Vec2 _sizeChangeRateRange;
	// The max time of the size change
	float _maxChangeTime;
	// The number of particles to emit in the burst if it is a temporary burst
	int _numparticlesinburst;
	// Whether particle fade in is enabled or not
	bool _fadein;
	// The number of textures to choose from for the particle
	int _numTex;
	// The vector of particles in this pool
	std::vector<std::shared_ptr<Particle>> _particles;

	// EMITTER TYPES

	// POINT EMITTER

	// The point of emission if this is a point emitter
	cugl::Vec2 _emissionPoint;

	/*
	* Creates a new particle
	*/
	void newParticle();

	/*
	* Creates a new particle and adds it to the pool if there is free space, otherwise remove the oldest particle and add a new one to the pool.
	*/
	void free();

	/* Initializes the global constants that are shared between all ParticlePool modes
	*/
	void init(std::shared_ptr<cugl::JsonValue> constants);


public:
	/**
	* Creates a particle pool
	*/
	ParticlePool() {}

	/**
	* Destroys a particle pool
	*/
	~ParticlePool() {}

	/**
	* Creates a new particle pool using a point emitter
	*
	*
	* @return  A newly allocated particle pool
	*/
	static std::shared_ptr<ParticlePool> allocPoint(std::shared_ptr<cugl::JsonValue> constants, cugl::Vec2 point) {
		std::shared_ptr<ParticlePool> result = std::make_shared<ParticlePool>();
		return (result->initPoint(constants, point) ? result : nullptr);
	}

	/**
	* Creates a new particle pool using a point emitter
	*
	*
	* @return  A newly allocated particle pool
	*/
	static std::shared_ptr<ParticlePool> allocPoint(std::shared_ptr<cugl::JsonValue> constants, cugl::Vec2 point, int numTex) {
		std::shared_ptr<ParticlePool> result = std::make_shared<ParticlePool>();
		return (result->initRandomTexture(constants, point, numTex) ? result : nullptr);
	}

	/**
	* Creates a new particle pool using a point emitter with an angle offset
	*
	*
	* @return  A newly allocated particle pool
	*/
	static std::shared_ptr<ParticlePool> allocPointWithOffset(std::shared_ptr<cugl::JsonValue> constants, cugl::Vec2 point, float angleoffset) {
		std::shared_ptr<ParticlePool> result = std::make_shared<ParticlePool>();
		return (result->initPointWithOffset(constants, point, angleoffset) ? result : nullptr);
	}

	/**
	* Creates a new particle pool using a point emitter with an angle offset
	*
	*
	* @return  A newly allocated particle pool
	*/
	static std::shared_ptr<ParticlePool> allocPointWithOffset(std::shared_ptr<cugl::JsonValue> constants, cugl::Vec2 point, float angleoffset, int numTex) {
		std::shared_ptr<ParticlePool> result = std::make_shared<ParticlePool>();
		return (result->initRandomTexture(constants, point, angleoffset, numTex) ? result : nullptr);
	}

	/**
	* Creates a new particle pool using a point emitter
	*
	*
	* @return  True if initialized correctly, false otherwise
	*/
	bool initPoint(std::shared_ptr<cugl::JsonValue> constants, cugl::Vec2 point);

	/**
	* Creates a new particle pool using a point emitter
	*
	*
	* @return  True if initialized correctly, false otherwise
	*/
	bool initPointWithOffset(std::shared_ptr<cugl::JsonValue> constants, cugl::Vec2 point, float angleoffset);

	/**
	* Creates a new particle pool using a point emitter
	*
	*	
	* @return  True if initialized correctly, false otherwise
	*/
	bool initRandomTexture(std::shared_ptr<cugl::JsonValue> constants, cugl::Vec2 point, float angleoffset, int numTex);

	/**
	* Creates a new particle pool using a point emitter
	*
	*
	* @return  True if initialized correctly, false otherwise
	*/
	bool initRandomTexture(std::shared_ptr<cugl::JsonValue> constants, cugl::Vec2 point, int numTex);

	/*
	* Gets the maximum lifetime of a particle
	*/
	float getMaxLifetime() { return _lifetimeRange.y; }

	/*
	* Gets whether the particle system is complete or not
	*/
	bool isComplete() { return _complete; }

	/* 
	* Updates the pool, updating existing particles, then adding new particles based off of emmision rate and dt.
	*/
	void update(float dt);

	/*
	* Gets the list of particles in the ParticlePool
	*/
	std::vector<std::shared_ptr<Particle>> getParticles() { return _particles; }

	/*
	* Adds the angle range of this pool. This is needed to send the particles at the right angles on a collision
	*/
	void addAngleRange(float angleRange) { _angleRange = _angleRange.add(angleRange, angleRange); }

};


#endif