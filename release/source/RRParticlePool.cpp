//
//  RRParticlePool.cpp
//  Created by Zach of Ragdoll Royale (2022)
//  Adapted with his permission by Alex Lee for use in Liminal Spirit
// 
//
#include "RRParticlePool.h"

using namespace cugl;

float randomLerp(Vec2 range) {
	return range.x + ((float)rand() / (float)RAND_MAX) * (range.y - range.x);
}

int randomIntLerp(Vec2 range) {
	return range.x + ((float)rand() / (float)RAND_MAX) * (range.y - range.x);
}

float randomRoll() {
	return ((float)rand() / (float)RAND_MAX);
}

Vec2 floatArrayToVec(std::vector<float> arr) {
	return Vec2(arr[0], arr[1]);
}

void ParticlePool::init(std::shared_ptr<JsonValue> constants) {
	// if this is a burst particle, initialize it as such
	if (constants->get("burstParticles") != nullptr) {
		_burst = true;
		_numparticlesinburst = constants->getInt("burstParticles");
	}
	else {
		_burst = false;
		_numparticlesinburst = 0;
	}
	_capacity = constants->getInt("maxParticleCount");
	_gravity = floatArrayToVec(constants->get("gravity")->asFloatArray());
	_emissionRateRange = floatArrayToVec(constants->get("emissionRateRange")->asFloatArray());
	_lifetimeRange = floatArrayToVec(constants->get("lifetimeRange")->asFloatArray());
	_angleRange = floatArrayToVec(constants->get("angleRange")->asFloatArray());
	_speedRange = floatArrayToVec(constants->get("speedRange")->asFloatArray());
	_sizeRange = floatArrayToVec(constants->get("startSizeRange")->asFloatArray());
	_sizeChangeRateRange = floatArrayToVec(constants->get("sizeChangeRateRange")->asFloatArray());
	_maxChangeTime = constants->getFloat("maxSizeChangeTime");
	if (constants->get("fadeinRange") != nullptr) {
		_fadeinRange = floatArrayToVec(constants->get("fadeinRange")->asFloatArray());
		_fadein = constants->getBool("fadein");
	}
	else {
		_fadein = false;
	}

	_complete = false;
	_currentEmissionRate = 0;
	_deadparticles = 0;
	_timer = 0;
	_numTex = 0;
}

bool ParticlePool::initPoint(std::shared_ptr<JsonValue> constants, Vec2 point) {
	init(constants);
	_emissionPoint = point;
	for (int i = 0; i < _numparticlesinburst; i++) {
		free();
	}
	return true;
}

bool ParticlePool::initPointWithOffset(std::shared_ptr<JsonValue> constants, Vec2 point, float angleoffset) {
	init(constants);
	_angleRange = _angleRange.add(angleoffset, angleoffset);
	_emissionPoint = point;
	for (int i = 0; i < _numparticlesinburst; i++) {
		free();
	}
	return true;
}

bool ParticlePool::initRandomTexture(std::shared_ptr<JsonValue> constants, Vec2 point, float angleoffset, int numTex) {
	init(constants);
	_angleRange = _angleRange.add(angleoffset, angleoffset);
	_emissionPoint = point;
	_numTex = numTex;
	for (int i = 0; i < _numparticlesinburst; i++) {
		free();
	}
	return true;
}

bool ParticlePool::initRandomTexture(std::shared_ptr<JsonValue> constants, Vec2 point, int numTex) {
	init(constants);
	_emissionPoint = point;
	_numTex = numTex;
	for (int i = 0; i < _numparticlesinburst; i++) {
		free();
	}
	return true;
}

void ParticlePool::newParticle() {
	float speed = randomLerp(_speedRange);
	float lifetime = randomLerp(_lifetimeRange);
	float angle = randomLerp(_angleRange);
	float size = randomLerp(_sizeRange);
	float changeRate = randomLerp(_sizeChangeRateRange);
	if (_numTex > 0) {
		std::shared_ptr<Particle> p = Particle::alloc(_emissionPoint, size, _gravity, speed, lifetime, angle, _numTex);
		_particles.push_back(p);
	}
	else {
		std::shared_ptr<Particle> p = Particle::alloc(_emissionPoint, size, _gravity, speed, lifetime, angle, _maxChangeTime, changeRate);
		_particles.push_back(p);
	}
}

void ParticlePool::free() {
	if (_particles.size() < _capacity) {
		newParticle();
	}
	else {
		_particles.erase(_particles.begin());
		newParticle();
	}
}

void ParticlePool::update(float dt) {
	std::vector<std::vector<std::shared_ptr<Particle>>::iterator> _toRemove;
	for (auto it = _particles.begin(); it != _particles.end(); it += 0) {
		(*it)->update(dt);
		if (_burst && (*it)->getLifetime() == 0) {
			_deadparticles++;
			it = _particles.erase(it);
		}
		else {
			it++;
		}
	}
	// Handling new emissions
	if (!_burst) {
		_timer += dt;
		if (_timer > 1) {
			_currentEmissionRate = randomIntLerp(_emissionRateRange);
			_timer -= 1;
		}
		if (randomRoll() < _currentEmissionRate * dt) {
			free();
		}
	}
	else if (_deadparticles == _numparticlesinburst) {
		_complete = true;
	}
}