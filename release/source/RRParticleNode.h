//
//  RRParticleNode.h
//  Created by Zach of Ragdoll Royale (2022)
//  Adapted with his permission by Alex Lee for use in Liminal Spirit
// 
//
#ifndef __RR_PARTICLE_NODE_H__
#define __RR_PARTICLE_NODE_H__

#include <cugl/cugl.h>
#include "RRParticlePool.h"

using namespace cugl;

class ParticleNode : public scene2::SceneNode {
private:
    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(ParticleNode);
protected:

    // The particle pool containing the particles in this node
    std::shared_ptr<ParticlePool> _particlePool;

    // The texture for each particle
    std::shared_ptr<Texture> _texture;

    // The list of textures for the particles if there are multiple textures
    std::vector<std::shared_ptr<Texture>> _textures;

    // If multiple textures are enabled
    bool _hasMultipleTextures;

    // If made up of two linked textures
    bool _hasLinkedTextures;

    // The offset between two linked textures
    Vec2 _linkOffset;

public:
    /*
    * Creates a new particle node at the origin.
    */
    ParticleNode(void) : SceneNode() { }

    /**
     * Destroys this particle node, releasing all resources.
     */
    virtual ~ParticleNode(void) {}
    /**
     * Creates a new particle node with the given position
     *
     * @return  A newly allocated particle node, at the given position, with the given particle pool
     */
    static std::shared_ptr<ParticleNode> alloc(const Vec2& pos, const std::shared_ptr<Texture> texture, std::shared_ptr<ParticlePool> particles) {
        std::shared_ptr<ParticleNode> result = std::make_shared<ParticleNode>();
        return (result->init(pos, texture, particles) ? result : nullptr);
    }

    /**
    * Creates a new particle node with the given position
    *
    * @return  A newly allocated particle node, at the given position, with the given particle pool
    */
    static std::shared_ptr<ParticleNode> alloc(const Vec2& pos, std::vector<std::shared_ptr<Texture>> textures, std::shared_ptr<ParticlePool> particles, bool linked, Vec2& offset) {
        std::shared_ptr<ParticleNode> result = std::make_shared<ParticleNode>();
        return (result->init(pos, textures, particles, linked, offset) ? result : nullptr);
    }   


    /**
     * Initializes a new particle node with the given position and particlePool
     *
     * @return  A newly allocated particle node, at the given position, with the given particle pool
     */
    bool init(const Vec2& pos, const std::shared_ptr<Texture> texture, std::shared_ptr<ParticlePool> particles);

    /**
    * Initializes a new particle node with the given position and particlePool
    *
    * @return  A newly allocated particle node, at the given position, with the given particle pool
     */
    bool init(const Vec2& pos, std::vector<std::shared_ptr<Texture>> textures, std::shared_ptr<ParticlePool> particles, bool linked, Vec2& offset);

    /*
    * Updates the particles in this node
    */
    void update(float dt);

    /*
    * Gets the particle pool attached to this system.
    */
    std::shared_ptr<ParticlePool> getPool() { return _particlePool; }

protected:
    /**
    * Draws this Node via the given SpriteBatch.
    *
    * This method only worries about drawing the current node.  It does not
    * attempt to render the children.
    *
    * This is the method that you should override to implement your custom
    * drawing code.  You are welcome to use any OpenGL commands that you wish.
    * You can even skip use of the SpriteBatch.  However, if you do so, you
    * must flush the SpriteBatch by calling end() at the start of the method.
    * in addition, you should remember to call begin() at the start of the
    * method.
    *
    * This method provides the correct transformation matrix and tint color.
    * You do not need to worry about whether the node uses relative color.
     * This method is called by render() and these values are guaranteed to be
    * correct.  In addition, this method does not need to check for visibility,
    * as it is guaranteed to only be called when the node is visible.
    *
    * @param batch     The SpriteBatch to draw with.
    * @param transform The global transformation matrix.
    * @param tint      The tint to blend with the Node color.
    */
    virtual void draw(const std::shared_ptr<SpriteBatch>& batch, const Affine2& transform, Color4 tint) override;
};
#endif