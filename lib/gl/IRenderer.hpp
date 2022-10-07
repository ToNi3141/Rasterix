// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2022 ToNi3141

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
    enum class TMU 
    {
        TMU0
    };

    class TexEnvConf
    {
    public:
        enum class Combine
        {
            REPLACE,
            MODULATE,
            ADD,
            ADD_SIGNED,
            INTERPOLATE,
            SUBTRACT,
            DOT3_RGB,
            DOT3_RGBA
        };
        enum class Operand
        {
            SRC_ALPHA,
            ONE_MINUS_SRC_ALPHA,
            SRC_COLOR,
            ONE_MINUS_SRC_COLOR
        };
        enum class SrcReg
        {
            TEXTURE,
            CONSTANT,
            PRIMARY_COLOR,
            PREVIOUS
        };

        TexEnvConf() = default;

        void setCombineRgb(const Combine val) { m_regVal.fields.combineRgb = val; }
        void setCombineAlpha(const Combine val) { m_regVal.fields.combineAlpha = val; }
        void setSrcRegRgb0(const SrcReg val) { m_regVal.fields.srcRegRgb0 = val; }
        void setSrcRegRgb1(const SrcReg val) { m_regVal.fields.srcRegRgb1 = val; }
        void setSrcRegRgb2(const SrcReg val) { m_regVal.fields.srcRegRgb2 = val; }
        void setSrcRegAlpha0(const SrcReg val) { m_regVal.fields.srcRegAlpha0 = val; }
        void setSrcRegAlpha1(const SrcReg val) { m_regVal.fields.srcRegAlpha1 = val; }
        void setSrcRegAlpha2(const SrcReg val) { m_regVal.fields.srcRegAlpha2 = val; }
        void setOperandRgb0(const Operand val) { m_regVal.fields.operandRgb0 = val; }
        void setOperandRgb1(const Operand val) { m_regVal.fields.operandRgb1 = val; }
        void setOperandRgb2(const Operand val) { m_regVal.fields.operandRgb2 = val; }
        void setOperandAlpha0(const Operand val) { m_regVal.fields.operandAlpha0 = val; }
        void setOperandAlpha1(const Operand val) { m_regVal.fields.operandAlpha1 = val; }
        void setOperandAlpha2(const Operand val) { m_regVal.fields.operandAlpha2 = val; }
        void setShiftRgb(const uint8_t val) { m_regVal.fields.shiftRgb = val; }
        void setShiftAlpha(const uint8_t val) { m_regVal.fields.shiftAlpha = val; }

        uint32_t serialize() const { return m_regVal.data; }

    private:
        union RegVal
        {
            struct __attribute__ ((__packed__)) RegContent
            {
                RegContent() :
                    combineRgb(Combine::MODULATE),
                    combineAlpha(Combine::MODULATE),
                    srcRegRgb0(SrcReg::TEXTURE),
                    srcRegRgb1(SrcReg::PREVIOUS),
                    srcRegRgb2(SrcReg::CONSTANT),
                    srcRegAlpha0(SrcReg::TEXTURE),
                    srcRegAlpha1(SrcReg::PREVIOUS),
                    srcRegAlpha2(SrcReg::CONSTANT),
                    operandRgb0(Operand::SRC_COLOR),
                    operandRgb1(Operand::SRC_COLOR),
                    operandRgb2(Operand::SRC_COLOR),
                    operandAlpha0(Operand::SRC_ALPHA),
                    operandAlpha1(Operand::SRC_ALPHA),
                    operandAlpha2(Operand::SRC_ALPHA),
                    shiftRgb(0),
                    shiftAlpha(0)
                { }

                Combine combineRgb : 3;
                Combine combineAlpha : 3;
                SrcReg srcRegRgb0 : 2;
                SrcReg srcRegRgb1 : 2;
                SrcReg srcRegRgb2 : 2;
                SrcReg srcRegAlpha0 : 2;
                SrcReg srcRegAlpha1 : 2;
                SrcReg srcRegAlpha2 : 2;
                Operand operandRgb0 : 2;
                Operand operandRgb1 : 2;
                Operand operandRgb2 : 2;
                Operand operandAlpha0 : 1;
                Operand operandAlpha1 : 1;
                Operand operandAlpha2 : 1;
                uint8_t shiftRgb : 2;
                uint8_t shiftAlpha : 2;
            } fields {};
            uint32_t data;
        } m_regVal;
    };

    class FragmentPipelineConf
    {
    public:
        enum class TestFunc
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

        enum class BlendFunc
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

        enum class LogicOp
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

        FragmentPipelineConf() = default;

        void setDepthFunc(const TestFunc val) { m_regVal.fields.depthFunc = val; }
        void setAlphaFunc(const TestFunc val) { m_regVal.fields.alphaFunc = val; }
        void setRefAlphaValue(const uint8_t val) { m_regVal.fields.referenceAlphaValue = val; }
        void setDepthMask(const bool val) { m_regVal.fields.depthMask = val; }
        void setColorMaskA(const bool val) { m_regVal.fields.colorMaskA  = val; }
        void setColorMaskR(const bool val) { m_regVal.fields.colorMaskR  = val; }
        void setColorMaskG(const bool val) { m_regVal.fields.colorMaskG  = val; }
        void setColorMaskB(const bool val) { m_regVal.fields.colorMaskB  = val; }
        void setBlendFuncSFactor(const BlendFunc val) { m_regVal.fields.blendFuncSFactor = val; }
        void setBlendFuncDFactor(const BlendFunc val) { m_regVal.fields.blendFuncDFactor = val; }

        uint32_t serialize() const { return m_regVal.data; }

    private:
        union RegVal
        {
            struct __attribute__ ((__packed__)) RegContent
            {
                RegContent() :
                    depthFunc(TestFunc::LESS),
                    alphaFunc(TestFunc::ALWAYS),
                    referenceAlphaValue(0xff),
                    depthMask(false),
                    colorMaskA(true),
                    colorMaskB(true),
                    colorMaskG(true),
                    colorMaskR(true),
                    blendFuncSFactor(BlendFunc::ONE),
                    blendFuncDFactor(BlendFunc::ZERO)
                { }

                TestFunc depthFunc : 3;
                TestFunc alphaFunc : 3;
                uint8_t referenceAlphaValue : 8;
                bool depthMask : 1;
                bool colorMaskA : 1;
                bool colorMaskB : 1;
                bool colorMaskG : 1;
                bool colorMaskR : 1;
                BlendFunc blendFuncSFactor : 4;
                BlendFunc blendFuncDFactor : 4;
            } fields {};
            uint32_t data;
        } m_regVal;
    };

    class FeatureEnableConf
    {
    public:
        FeatureEnableConf() = default;
        void setEnableFog(const bool val) { m_regVal.fields.fog = val; }
        void setEnableBlending(const bool val) { m_regVal.fields.blending = val; }
        void setEnableDepthTest(const bool val) { m_regVal.fields.depthTest = val; }
        void setEnableAlphaTest(const bool val) { m_regVal.fields.alphaTest = val; }
        void setEnableTmu0(const bool val) { m_regVal.fields.tmu0 = val; }
        void setEnableScissor(const bool val) { m_regVal.fields.scissor = val; }

        uint32_t serialize() const { return m_regVal.data; }
    private:
        union RegVal
        {
            struct __attribute__ ((__packed__)) RegContent
            {
                RegContent()
                    : fog(false)
                    , blending(false)
                    , depthTest(false)
                    , alphaTest(false)
                    , tmu0(false)
                    , scissor(false)
                { }

                bool fog : 1;
                bool blending : 1;
                bool depthTest : 1;
                bool alphaTest : 1;
                bool tmu0 : 1;
                bool scissor : 1;
            } fields {};
            uint32_t data;
        } m_regVal;
    };

    enum class TextureWrapMode
    {
        REPEAT,
        CLAMP_TO_EDGE
    };

    /// @brief Will render a triangle which is constructed with the given parameters
    /// @return true if the triangle was rendered, otherwise the display list was full and the triangle can't be added
    virtual bool drawTriangle(const Vec4& v0,
                              const Vec4& v1,
                              const Vec4& v2,
                              const Vec4& tc0,
                              const Vec4& tc1,
                              const Vec4& tc2,
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
    /// @param texWrapModeS The wrapping mode of the texture in s direction
    /// @param texWrapModeT The wrapping mode of the texture in t direction
    /// @param enableMagFilter Enables magnification filter of the texture (GL_LINEAR)
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool updateTexture(const uint16_t texId, 
                               std::shared_ptr<const uint16_t> pixels, 
                               const uint16_t texWidth,
                               const uint16_t texHeight,
                               const TextureWrapMode texWrapModeS,
                               const TextureWrapMode texWrapModeT,
                               const bool enableMagFilter) = 0;
    
    /// @brief Activates a texture which then is used for rendering
    /// @param target is used TMU
    /// @param texId The id of the texture to use
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool useTexture(const TMU target, const uint16_t texId) = 0; 

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

    /// @brief Configures the texture environment. See glTexEnv()
    /// @param target is used TMU
    /// @param pname parameter name of the env parameter
    /// @param param Function of the tex env
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setTexEnv(const TMU target, const TexEnvConf& texEnvConfig) = 0;

    /// @brief Set a static color for the tex environment
    /// @param color the color in ABGR
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setTexEnvColor(const Vec4i& color) = 0;

    /// @brief Updates the fragment pipeline configuration 
    /// @param pipelineConf The new pipeline configuration 
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setFragmentPipelineConfig(const FragmentPipelineConf& pipelineConf) = 0;

    /// @brief Set the fog color
    /// @param color the color in ABGR
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setFogColor(const Vec4i& color) = 0;

    /// @brief Sets the fog LUT. Note that the fog uses the w value as index (the distance from eye to the polygon znear..zfar)
    /// and not z (clamped value of 0.0..1.0)
    /// @param fogLut the fog lookup table
    /// The fog LUT uses a exponential distribution of w, means fogLut[0] = f(1), fogLut[1] = f(2), fogLut[2] = f(4), fogLut[3] = f(8).
    /// The fog values between start and end must not exceed 1.0f
    /// @param start the start value of the fog
    /// @param end the end value of the fog
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setFogLut(const std::array<float, 33>& fogLut, float start, float end) = 0;

    /// @brief Enable or disable a feature
    /// @param featureEnable The enabled features
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setFeatureEnableConfig(const FeatureEnableConf& featureEnable) = 0;

    /// @brief Sets the scissor box parameter
    /// @param x X coordinate of the box
    /// @param y Y coordinate of the box
    /// @param width Width of the box
    /// @param height Height of the box
    /// @return true if success
    virtual bool setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height) = 0;
};

#endif // IRENDERER_HPP
