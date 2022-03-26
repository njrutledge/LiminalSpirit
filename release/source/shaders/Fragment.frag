R"(////////// SHADER BEGIN /////////
//  Shader.frag
//  Cornell University Game Library (CUGL)
//
//  This is a primitive SpriteBatch fragment shader for both OpenGL and OpenGL ES.
//  It supports textures which can be tinted per vertex. It supports gradients
//  but does not support scissors.  It is for testing purposes only.
//
//  CUGL MIT License:
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
//  Version: 2/10/20


#ifdef CUGLES
// This one line is all the difference
precision mediump float;
#endif

// The output color
out vec4 frag_color;

// Color result from vertex shader
in vec4 outColor;

// Texture result from vertex shader
in vec2 outTexCoord;

// Gradient result from vertex shader
in vec2 outGradCoord;

// GRADIENT UNIFORM
// The gradient matrix
uniform mat3 gdMatrix;
// The gradient inner color
uniform vec4 gdInner;
// The gradient outer color
uniform vec4 gdOuter;
// The gradient extent
uniform vec2 gdExtent;
// The gradient radius
uniform float gdRadius;
// The gradient feather
uniform float gdFeathr;

// What to output
uniform int uType;

// Texture map
uniform sampler2D uTexture;

/**
 * Returns an interpolation value for a box gradient
 *
 * The value returned is the mix parameter for the inner and
 * outer color.
 *
 * Adapted from nanovg by Mikko Mononen (memon@inside.org)
 *
 * pt:      The (transformed) point to test
 * ext:     The gradient extent
 * radius:  The gradient radius
 * feather: The gradient feather amount
 */
float boxgradient(vec2 pt, vec2 ext, float radius, float feather) {
    vec2 ext2 = ext - vec2(radius,radius);
    vec2 dst = abs(pt) - ext2;
    float m = min(max(dst.x,dst.y),0.0) + length(max(dst,0.0)) - radius;
    return clamp((m + feather*0.5) / feather, 0.0, 1.0);
}

/**
 * Performs the main fragment shading.
 */
void main(void) {
    if (uType == 0) {
        // Apply the color
        frag_color = outColor;
    } else if (uType == 1) {
        // Apply a texture
        frag_color = texture(uTexture, outTexCoord);
    } else if (uType == 2) {
        // Apply the color to a texture
        frag_color = texture(uTexture, outTexCoord)*outColor;
    } else if (uType == 3) {
        // Apply a gradient color
        vec2 pt = (gdMatrix * vec3(outGradCoord,1.0)).xy;
        float d = boxgradient(pt,gdExtent,gdRadius,gdFeathr);
        frag_color = mix(gdInner,gdOuter,d);
    } else if (uType == 4) {
        // Apply a gradient color to a texture
        vec2 pt = (gdMatrix * vec3(outGradCoord,1.0)).xy;
        float d = boxgradient(pt,gdExtent,gdRadius,gdFeathr);
        frag_color = mix(gdInner,gdOuter,d)*texture(uTexture, outTexCoord);
    } else {
        // Make it all white
        frag_color = vec4(1,1,1,1);
    }
}

/////////// SHADER END //////////)"
