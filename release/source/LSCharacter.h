//
//  LSCharacter.h
//  Liminal Spirit Game
//
//  This class tracks the state of the character, a spirit.
//

#ifndef __GL_CHARACTER_H__
#define __GL_CHARACTER_H__
#include <cugl/cugl.h>

/** The number of frames until we can fire again */
#define RELOAD_RATE 3

/**
 * Model class representing the main character.
 */
class Character
{
private:
  /** Position of the character */
  cugl::Vec2 _pos;
  /** Velocity of the character */
  cugl::Vec2 _vel;

  /** Health of the character */
  int _health;
  /** Countdown for melee refire rate */
  int _meleerefire;
  /** Countdown for range refire rate */
  int _rangerefire;

  // JSON DEFINED ATTRIBUTES
  /** Mass/weight of the character. Used in collisions. */
  float _mass;
  /** The number of frames until we can use melee again */
  int _meleefirerate;
  /** The number of frames until we can use melee again */
  int _rangefirerate;
  /** Amount to adjust horizontal movement from input */
  float _horthrust;
  /** The maximum horizontal velocity */
  float _maxhorvel;
  /** The height the character can jump*/
  float _jumpheight;

  // /** The number of columns in the sprite sheet */
  // int _framecols;
  // /** The number of frames in the sprite sheet */
  // int _framesize;
  // /** The sprite sheet frame for being at rest */
  // int _frameflat;
  // /** The shadow offset in pixels */
  // float _shadows;

  // Asset references. These should be set by GameScene
  /** Reference to the character sprite sheet */
  std::shared_ptr<cugl::SpriteSheet> _sprite;
  /** Height of character */
  float _height;
  /** Width of character */
  float _width;

public:
#pragma mark Constructors
  /**
   * Creates a character with the given position and data.
   *
   * The JsonValue should be a reference of all of the constants
   * that necessary to set the "hidden physical properties".
   *
   * @param pos   The character position
   * @param data  The data defining the physics constants
   */
  Character(const cugl::Vec2 &pos, std::shared_ptr<cugl::JsonValue> data);

  /**
   * Disposes the character, releasing all resources.
   */
  ~Character() {}

#pragma mark Properties
  /**
   * Returns the position of this character.
   *
   * This is location of the center pixel of the character on the screen.
   *
   * @return the position of this character
   */
  const cugl::Vec2 &getPosition() const { return _pos; }

  /**
   * Sets the position of this character.
   *
   * This is location of the center pixel of the character on the screen.
   * Setting this value does NOT respect wrap around. It is possible
   * to use this method to place the character off screen (so be careful).
   *
   * @param value the position of this character
   */
  void setPosition(cugl::Vec2 value) { _pos = value; }

  /**
   * Sets the position of this character, supporting wrap-around.
   *
   * This is the preferred way to "bump" a character in a collision.
   *
   * @param value     The position of this character
   * @param size      The size of the window (for wrap around)
   */
  void setPosition(cugl::Vec2 value, cugl::Vec2 size);

  /**
   * Returns the velocity of this character.
   *
   * This value is necessary to control momementum in character movement.
   *
   * @return the velocity of this character
   */
  const cugl::Vec2 &getVelocity() const { return _vel; }

  /**
   * Sets the velocity of this character.
   *
   * This value is necessary to control momementum in character movement.
   *
   * @param value the velocity of this character
   */
  void setVelocity(cugl::Vec2 value) { _vel = value; }

  /**
   * Returns the current character health.
   *
   * When the health of the character is 0, it is "dead"
   *
   * @return the current character health.
   */
  int getHealth() const { return _health; }

  /**
   * Sets the current character health.
   *
   * When the health of the character is 0, it is "dead"
   *
   * @param value The current character health.
   */
  void setHealth(int value);

  /**
   * Returns the mass of the character.
   *
   * This value is necessary to resolve collisions. It is set by the
   * initial JSON file.
   *
   * @return the character mass
   */
  float getMass() const { return _mass; }

  /**
   * @brief Get the height of the character sprite, for collisions
   *
   * @return float
   */
  float getHeight() const { return _height; }
  /**
   * @brief Get the width of the character sprite, for collisions
   *
   * @return float
   */
  float getWidth() const { return _width; }

  /**************************************
   * MELEE
   * ************************************/
  /**
   * @brief Whether the character can use their melee attack
   *
   * @return true
   * @return false
   */
  bool canUseMelee() const { return (_meleerefire > _meleefirerate); }

  /**
   * @brief Resets melee counter so character cannot fire immediately
   * Reload rate is set by "melee fire rate" in JSON file
   */
  void reloadMelee() { _meleerefire = 0; }

  /**************************************
   * RANGE
   * ************************************/
  /**
   * @brief Whether the character can use their range attack
   */
  bool canUseRange() const { return (_rangerefire > _rangefirerate); }

  /**
   * @brief Resets range counter so character cannot fire immediately
   * Reload rate is set by "range fire rate" in JSON file
   */
  void reloadRange() { _rangerefire = 0; }

#pragma mark Graphics
  /**
   * Returns the sprite sheet for the character
   *
   * The size and layout of the sprite sheet should already be specified
   * in the initializing JSON. Otherwise, the contents of the sprite sheet
   * will be ignored.     *
   * @return the sprite sheet for the character
   */
  const std::shared_ptr<cugl::SpriteSheet> &getSprite() const
  {
    return _sprite;
  }

  /**
   * Sets the texture for this character.
   *
   * The texture should be formated as a sprite sheet, and the size and
   * layout of the sprite sheet should already be specified in the
   * initializing JSON. If so, this method will construct a sprite sheet
   * from this texture. Otherwise, the texture will be ignored.
   *
   * @param texture   The texture for the sprite sheet
   */
  void setTexture(const std::shared_ptr<cugl::Texture> &texture);

  /**
   * Draws this character to the sprite batch within the given bounds.
   *
   * This drawing code supports "wrap around". If the character is partly off of
   * one edge, then it will also be drawn across the edge on the opposite
   * side.
   *
   * @param batch     The sprite batch to draw to
   * @param size      The size of the window (for wrap around)
   */
  void draw(const std::shared_ptr<cugl::SpriteBatch> &batch, cugl::Size size);

#pragma mark Movement
  /**
   * @brief Moves the character by the specified amount.
   * The character can move in horizontal and vertical directions
   * Horizontal is the amount to move horizontally. Negative represents left,
   * positive represents right.
   * Vertical is the amount to move vertically. Negative represents down,
   * positive represents up.
   *
   * No collision detection - resolved afterwards
   */
  void move(float horizontal, float vertical, cugl::Size size);
};

#endif /* __LS_CHARACTER_H__ */
