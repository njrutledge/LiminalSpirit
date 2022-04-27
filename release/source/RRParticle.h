//
//  RRParticle.h
//  Created by Zach of Ragdoll Royale (2022)
//  Adapted with his permission by Alex Lee for use in Liminal Spirit
// 
//

#ifndef __RR_PARTICLE_H__
#define __RR_PARTICLE_H__
#include <cugl/cugl.h>

	class Particle {
	protected:
		// BASIC PROPERTIES

		/* Position of the particle */
		cugl::Vec2 _position;
		/* Velocity of the particle */
		cugl::Vec2 _velocity;
		/* Gravity affecting this particle */
		cugl::Vec2 _gravity;
		/* External acceleratin affecting this particle */
		cugl::Vec2 _accel;
		/* The current lifetime of this particle */
		float _lifetime;
		/* The max lifetime of the particle */
		float _maxlifetime;
		/* Angle of this particle in radians */
		float _angle;
		/* The speed of this particle */
		float _speed;
		/* The size of the particle */
		float _size;

		// FADING AND OPACITY

		/* Whether this particle should fade in or not */
		bool _fadein;
		/* The opacity of this particle (may differ from lifetime / maxlifetime if fadein is enabled */
		float _opacity;
		/* The time for the particle to fade in for */
		float _maxfadetime;
		/* The timer to keep track of the fade */
		float _fadetimer;

		//SIZE ALTERING 
		/* 1 if enlarging, -1 if shrinking and 0 if neither */
		int _shrinkOrEnlarge;
		/* The time for the particle to increase/decrease in size */
		float _maxsizechangetime;
		/* The timer to keep track of size changing */
		float _sizetimer;
		/** The rate of which the particle changes size */
		float _sizechangerate;

		// TEXTURING
		/* Whether random texturing is enabled or not */
		bool _randomTexturing;
		/* The number of random textures to swap between */
		int _numTextures;
		/* The texture ID of this particle */
		int _texID;

	public:
		/**
		* Creates a particle
		*/
		Particle() {}

		/**
		* Destroys a particle
		*/
		~Particle() {}

		/**
		* Creates a new particle
		*
		* @return  A newly allocated particle
		*/
		static std::shared_ptr<Particle> alloc(cugl::Vec2 position, float size, cugl::Vec2 gravity, float speed, float lifetime, float angle) {
			std::shared_ptr<Particle> result = std::make_shared<Particle>();
			return (result->init(position, size, gravity, speed, lifetime, angle) ? result : nullptr);
		}

		/**
		* Creates a new particle with a random amount of textures to swap between
		*
		* @return  A newly allocated particle
		*/
		static std::shared_ptr<Particle> alloc(cugl::Vec2 position, float size, cugl::Vec2 gravity, float speed, float lifetime, float angle, int numTextures) {
			std::shared_ptr<Particle> result = std::make_shared<Particle>();
			return (result->initRandomTexture(position, size, gravity, speed, lifetime, angle, numTextures) ? result : nullptr);
		}

		/**
		* Creates a new particle with fade-in
		*
		* @return  A newly allocated particle with fade-in
		*/
		static std::shared_ptr<Particle> alloc(cugl::Vec2 position, float size, cugl::Vec2 gravity, float speed, float lifetime, float angle, float maxFadeTime) {
			std::shared_ptr<Particle> result = std::make_shared<Particle>();
			return (result->initFading(position, size, gravity, speed, lifetime, angle, maxFadeTime) ? result : nullptr);
		}

		/**
		* Creates a new particle with size changing
		*
		* @return  A newly allocated particle with size changing
		*/
		static std::shared_ptr<Particle> alloc(cugl::Vec2 position, float size, cugl::Vec2 gravity, float speed, float lifetime, float angle, float maxChangeTime, float changeRate) {
			std::shared_ptr<Particle> result = std::make_shared<Particle>();
			return (result->initSizeChanging(position, size, gravity, speed, lifetime, angle, maxChangeTime, changeRate) ? result : nullptr);
		}

		/*
		* Creates a new particle with random texturing
		* 
		* @return true if succesful
		*/
		bool initRandomTexture(cugl::Vec2 position, float size, cugl::Vec2 gravity, float speed, float lifetime, float angle, int numTextures);


		/**
		* Creates a new particle
		*
		* @return  true if successful
		*/
		bool init(cugl::Vec2 position, float size, cugl::Vec2 gravity, float speed, float lifetime, float angle);

		/**
		* Creates a particle with fade-in
		* 
		* @return true if successful
		*/
		bool initFading(cugl::Vec2 position, float size, cugl::Vec2 gravity, float speed, float lifetime, float angle, float maxFadeTime);

		/**
		* Creates a particle with size increase/decreasing
		*
		* @return true if successful
		*/
		bool initSizeChanging(cugl::Vec2 position, float size, cugl::Vec2 gravity, float speed, float lifetime, float angle, float maxChangeTime, float changeRate);

		/*Returns the position of this particle.
			*
			* The object returned is read only. Therefore,
			* changes to this vector are reflected in the particle animation.
			*
			* @return the position of this particle
		*/
		const cugl::Vec2 getPosition() { return _position; }

		/**
		 * Sets the particle position.
		*
		* @param x  the x-coordinate of the particle position
		* @param y  the y-coordinate of the particle position
		*/
		void setPosition(float x, float y) { _position.set(x, y); }


		/*
		* Sets the particle velocity.
		*/
		void setVelocity(float x, float y) { _velocity.set(x, y); }

		/*
		* Gets the particle opacity
		*/
		float getOpacity() { return _opacity; }

		/*
		* Gets the texture ID of this particle
		*/
		int getTexture() { return _texID; }

		/*
		* Gets the size of the particle (scales invididual particle textures)
		*/
		float getSize() { return _size; }
		
		/**
		* Sets the size (represented as a scaling value for the texture) of the particle
		*/
		void setSize(float scale) { _size = scale; }

		/**
		* Returns the angle of this particle
		*
		* The particle velocity is (PARTICLE_SPEED,angle) in polar-coordinates.
		*
		 * @return the angle of this particle
		 */
		const float getAngle() { return _angle; }
		/**
		* Sets the angle of this particle
		 *
		* When the angle is set, the particle will change its velocity
		 * to (PARTICLE_SPEED,angle) in polar-coordinates.
		 *
		* @param angle  the angle of this particle
		*/
		void setAngle(float a);

		/*
		* Gets the velocity of this particle
		*/
		cugl::Vec2 getVelocity() { return _velocity; }

		/*
		* Sets the acceleration of this particle
		*/
		void setAccel(float x, float y) { _accel.set(x, y); }

		/**
		* Returns the lifetime of this particle
		*
		*
		 * @return the lifetime of this particle
		 */
		const float getLifetime() { return _lifetime; }

		/**
		* Sets the particle lifetime.
		*
		* @param l the lifetime of the particle
		*/
		void setLifetime(float l) { _lifetime = l; }

		/*
		* Updates the particle
		*/
		void update(float dt);

	};


#endif