//
//  RRParticle.cpp
//  Created by Zach of Ragdoll Royale (2022)
//  Adapted with his permission by Alex Lee for use in Liminal Spirit
// 
//

#include "RRParticle.h"

using namespace cugl;

bool Particle::init(cugl::Vec2 position, float size, cugl::Vec2 gravity, float speed, float lifetime, float angle) {
	_position = position;
	_size = size;
	_speed = speed;
	_gravity = gravity;
	_lifetime = lifetime;
	_maxlifetime = lifetime;
	_shrinkOrEnlarge = 0;
	_opacity = 1;
	_accel = Vec2();
	setAngle(angle);
	return true;
}

bool Particle::initFading(cugl::Vec2 position, float size, cugl::Vec2 gravity, float speed, float lifetime, float angle, float maxFadeTime) {
	init(position, size, gravity, speed, lifetime, angle);
	_fadein = true;
	_maxfadetime = maxFadeTime;
	_opacity = 0;
	_fadetimer = 0;
	return true;
}

bool Particle::initRandomTexture(cugl::Vec2 position, float size, cugl::Vec2 gravity, float speed, float lifetime, float angle, int numTextures) {
	init(position, size,  gravity, speed, lifetime, angle);
	_randomTexturing = true;
	_numTextures = numTextures;
	_texID = (float)rand() / (float)RAND_MAX * numTextures;
	return true;
}

bool Particle::initSizeChanging(cugl::Vec2 position, float size, cugl::Vec2 gravity, float speed, float lifetime, float angle, float maxChangeTime, float changeRate) {
	init(position, size, gravity, speed, lifetime, angle);
	_maxsizechangetime = maxChangeTime;
	_sizechangerate = changeRate;
	_sizetimer = 0;
	if (changeRate > 0) {
		_shrinkOrEnlarge = 1;
	}
	else if (changeRate < 0) {
		_shrinkOrEnlarge = -1;
	}
	else {
		_shrinkOrEnlarge = 0;
		CULog("Why would you do this...");
	}

	return true;
}

void Particle::setAngle(float a) {
	_angle = a;
	_velocity.set(_speed * cos(_angle), _speed * sin(_angle));
}

void Particle::update(float dt) {
	_velocity += _gravity * dt + _accel * dt;
	_angle = atan2(_velocity.y, _velocity.x);
	_position += _velocity * dt;
	_lifetime = max(0.f, _lifetime - dt);
	_opacity = _lifetime / _maxlifetime;
	if (_fadein && _fadetimer < _maxfadetime) {
		_opacity = _fadetimer / _maxfadetime;
		_fadetimer += dt;
		// If we have lost enough life such that our fadein is greater than the current life of the particle, cancel the fade in
		if (_opacity > _lifetime / _maxlifetime) {
			_fadein = false;
			_opacity = _lifetime / _maxlifetime;
		}
	}
	CULog("ParticleUpdate: size is %f", _size);
	if (_shrinkOrEnlarge != 0 && _sizetimer < _maxsizechangetime) {
		_sizetimer += dt;
		float prevSize = _size;
		_size += _sizechangerate;
		// if size becomes zero or negative end the shrink
		if (_size <= 0) {
			_shrinkOrEnlarge = 0;
			_size = prevSize; // set to previous last size (maybe setting to arbitrary small value is better?)
		}
	}
}