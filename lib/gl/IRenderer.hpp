// RasteriCEr
// https://github.com/ToNi3141/RasteriCEr
// Copyright (c) 2021 ToNi3141

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef IRENDERER_HPP
#define IRENDERER_HPP

#include "Veci.hpp"
#include <stdint.h>
#include <array>
#include <utility>
#include <memory>
#include "Vec.hpp"

class IRenderer
{
public:
    enum TestFunc
    {
        ALWAYS,
        NEVER,
        LESS,
        EQUAL,
        LEQUAL,
        GREATER,
        NOTEQUAL,
        GEQUAL
    };
    enum TexEnvTarget
    {
        TEXTURE_ENV
    };
    enum TexEnvParamName
    {
        TEXTURE_ENV_MODE,
        GL_TEXTURE_ENV_COLOR
    };
    enum TexEnvParam
    {
        DISABLE,
        REPLACE,
        MODULATE,
        DECAL,
        BLEND,
        ADD
    };
    enum BlendFunc
    {
        ZERO,
        ONE,
        DST_COLOR,
        SRC_COLOR,
        ONE_MINUS_DST_COLOR,
        ONE_MINUS_SRC_COLOR,
        SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA,
        DST_ALPHA,
        ONE_MINUS_DST_ALPHA,
        SRC_ALPHA_SATURATE
    };
    enum LogicOp
    {
        CLEAR,
        SET,
        COPY,
        COPY_INVERTED,
        NOOP,
        INVERTED,
        AND,
        NAND,
        OR,
        NOR,
        XOR,
        EQUIV,
        AND_REVERSE,
        AND_INVERTED,
        OR_REVERSE,
        OR_INVERTED
    };
    enum TextureWrapMode
    {
        REPEAT,
        CLAMP_TO_EDGE
    };

    /// @brief Will render a triangle which is constructed with the given parameters
    /// TODO: Document ranges, the vectors should be in screen coordinates, textures should be in the range of 0..1.0
    /// color 0..255
    /// @return true if the triangle was rendered, otherwise the display list was full and the triangle can't be added
    virtual bool drawTriangle(const Vec4& v0,
                              const Vec4& v1,
                              const Vec4& v2,
                              const Vec2& st0,
                              const Vec2& st1,
                              const Vec2& st2,
                              const Vec4& c0,
                              const Vec4& c1,
                              const Vec4& c2) = 0;

    /// @brief Will send a picture from working buffers, caches or whatever to the buffer which is
    ///    read from the display
    virtual void commit() = 0;

    /// @brief Creates a new texture 
    /// @return pair with the first value to indicate if the operation succeeded (true) and the second value with the id
    virtual std::pair<bool, uint16_t> createTexture() = 0;

    /// @brief This will update the texture data of the texture with the given id
    /// @param texId The texture id which texture has to be updated
    /// @param pixels The texture as RGBA4444
    /// @param texWidth The width of the texture
    /// @param texHeight The height of the texture
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool updateTexture(const uint16_t texId, std::shared_ptr<const uint16_t> pixels, const uint16_t texWidth, const uint16_t texHeight) = 0;
    
    /// @brief Activates a texture which then is used for rendering
    /// @param texId The id of the texture to use
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool useTexture(const uint16_t texId) = 0; 

    /// @brief Deletes a texture 
    /// @param texId The id of the texture to delete
    /// @return true if succeeded
    virtual bool deleteTexture(const uint16_t texId) = 0;

    /// @brief Will clear a buffer
    /// @param frameBuffer Will clear the frame buffer
    /// @param zBuffer Will clear the z buffer
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool clear(bool frameBuffer, bool zBuffer) = 0;

    /// @brief Will set a color which is used as clear color for the frame buffer
    /// when clear() is called
    /// @param color the color in ABGR
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setClearColor(const Vec4i& color) = 0;

    /// @brief Will set a depth which is used as initial value for the depth buffer when clear() is called
    /// @param depth The depth which is used by calling clear()
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setClearDepth(uint16_t depth) = 0;

    /// @brief Enables the depth buffer tests
    /// @param enable true to enable the depth buffer tests. If the depth test enabled, a fragment is discarded when the depth test
    /// failed or is written to the depth buffer.
    /// if enable is false, the depth test always passes and the current depth value is not written into the depth buffer. Unlike
    /// setDepthMask(), this function does not prevent that clear() writes into the depth buffer.
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool enableDepthTest(const bool enable) = 0;

    /// @brief Enables writing to the depth buffer. See also glDepthMask().
    /// @param flag true means writing to the depth buffer is enabled
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setDepthMask(const bool flag) = 0;

    /// @brief Enables writing to the color buffer. See also glColorMask().
    /// @param r true means writing ret to the color buffer is enabled
    /// @param g true means writing green to the color buffer is enabled
    /// @param b true means writing blue to the color buffer is enabled
    /// @param a true means writing alpha to the color buffer is enabled
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setColorMask(const bool r, const bool g, const bool b, const bool a) = 0;

    /// @brief Configures a depth function. See also glDepthFunc().
    /// @param func The depth function
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setDepthFunc(const TestFunc func) = 0;

    /// @brief Configures the alpha test. See also glAlphaFunc().
    /// @param func The alpha function
    /// @param ref The reference value agaist func is testing (4 bit color)
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setAlphaFunc(const TestFunc func, const uint8_t ref) = 0;

    /// @brief Configures the texture environment. See glTexEnv()
    /// @param target The target of the texture env
    /// @param pname parameter name of the env parameter
    /// @param param Function of the tex env
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setTexEnv(const TexEnvTarget target, const TexEnvParamName pname, const TexEnvParam param) = 0;

    /// @brief Set a static color for the tex environment
    /// @param color the color in ABGR
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setTexEnvColor(const Vec4i& color) = 0;

    /// @brief Configures the blend func. See glBlendFunc()
    /// @param sfactor Source factor function
    /// @param dfactor Destination factor function
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setBlendFunc(const BlendFunc sfactor, const BlendFunc dfactor) = 0;

    /// @brief Configures the logic opcodes. See glLogicOp()
    /// @param opcode The used opcode
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setLogicOp(const LogicOp opcode) = 0;

    /// @brief Configures the wrapping mode of the texture parameter S
    /// @param mode The new wrap mode
    /// @return true if succeeded
    virtual bool setTextureWrapModeS(const TextureWrapMode mode) = 0;

    /// @brief Configures the wrapping mode of the texture parameter T
    /// @param mode The new wrap mode
    /// @return true if succeeded
    virtual bool setTextureWrapModeT(const TextureWrapMode mode) = 0;

};

#endif // IRENDERER_HPP
