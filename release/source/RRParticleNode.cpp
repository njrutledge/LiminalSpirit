//
//  RRParticleNode.cpp
//  Created by Zach of Ragdoll Royale (2022)
//  Adapted with his permission by Alex Lee for use in Liminal Spirit
// 
//

#include "RRParticleNode.h"

using namespace cugl;

bool ParticleNode::init(const Vec2& pos, const std::shared_ptr<Texture> texture, std::shared_ptr<ParticlePool> particles) {
	if (scene2::SceneNode::init()) {
		std::string name("particles");
		setName(name);
		setPosition(pos);
		_texture = texture;
		_particlePool = particles;
		_hasMultipleTextures = false;
		_hasTwoLinkedTextures = false;
		_linkOffset = 0;
		return true;
	}
    // added to fix none-void function does not return value in control path
    return false;
}

bool ParticleNode::init(const Vec2& pos, std::vector<std::shared_ptr<Texture>> textures, std::shared_ptr<ParticlePool> particles, bool linked, Vec2& offset) {
	if (scene2::SceneNode::init()) {
		std::string name("particles");
		setName(name);
		setPosition(pos);
		_textures = textures;
		_particlePool = particles;
		if (!linked) {
			_hasMultipleTextures = true;
			_hasTwoLinkedTextures = false;
			_linkOffset = 0;
		}
		else {
			_hasMultipleTextures = false;
			_hasTwoLinkedTextures = true;
			_linkOffset = offset;
		}
		return true;
	}
	// added to fix none-void function does not return value in control path
	return false;
}

void ParticleNode::update(float dt) {
	if (isVisible()) {
		_particlePool->update(dt);
	}
}

void ParticleNode::draw(const std::shared_ptr<SpriteBatch>& batch, const Affine2& transform, Color4 tint) {
	for (std::shared_ptr<Particle> p : _particlePool->getParticles()) {
		Color4 tintLerp = tint.getLerp(Color4(255, 255, 255, p->getOpacity() * 255), 0.5f);
		if (p->getOpacity() * 255.f < tintLerp.a) {
			tintLerp.a = p->getOpacity() * 255.f;
		}
		if (_hasMultipleTextures) {
			std::shared_ptr<Texture> texture = _textures.at(p->getTexture());
			batch->draw(texture, tintLerp, Vec2(texture->getWidth() / 2, texture->getHeight() / 2), transform.getScale()*p->getSize(), p->getAngle(), transform.getTranslation() + p->getPosition());
		}
		else if (_hasTwoLinkedTextures) {
			std::shared_ptr<Texture> texture1 = _textures.at(0);
			std::shared_ptr<Texture> texture2 = _textures.at(1);
			batch->draw(texture1, tintLerp, Vec2(texture1->getWidth() / 2, texture1->getHeight() / 2), transform.getScale() * p->getSize(), p->getAngle(), transform.getTranslation() + p->getPosition());
			batch->draw(texture2, tintLerp, Vec2(texture1->getWidth() / 2, texture1->getHeight() / 2), transform.getScale() * p->getSize(), p->getAngle(), transform.getTranslation() + p->getPosition() + _linkOffset);
		}
		else {
			batch->draw(_texture, tintLerp, Vec2(_texture->getWidth() / 2, _texture->getHeight() / 2), transform.getScale()*p->getSize(), p->getAngle(), transform.getTranslation() + p->getPosition());
		}
	}
}
