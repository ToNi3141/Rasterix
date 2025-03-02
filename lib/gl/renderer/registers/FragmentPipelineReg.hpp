// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2023 ToNi3141

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

#ifndef _FRAGMENT_PIPELINE_REG_
#define _FRAGMENT_PIPELINE_REG_

#include "TestFuncDef.hpp"
#include <cstdint>

namespace rr
{
class FragmentPipelineReg
{
public:
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

    FragmentPipelineReg() = default;

    void setDepthFunc(const TestFunc val) { m_regVal.fields.depthFunc = static_cast<uint32_t>(val); }
    void setAlphaFunc(const TestFunc val) { m_regVal.fields.alphaFunc = static_cast<uint32_t>(val); }
    void setRefAlphaValue(const uint8_t val) { m_regVal.fields.referenceAlphaValue = val; }
    void setDepthMask(const bool val) { m_regVal.fields.depthMask = val; }
    void setColorMaskA(const bool val) { m_regVal.fields.colorMaskA = val; }
    void setColorMaskR(const bool val) { m_regVal.fields.colorMaskR = val; }
    void setColorMaskG(const bool val) { m_regVal.fields.colorMaskG = val; }
    void setColorMaskB(const bool val) { m_regVal.fields.colorMaskB = val; }
    void setBlendFuncSFactor(const BlendFunc val) { m_regVal.fields.blendFuncSFactor = static_cast<uint32_t>(val); }
    void setBlendFuncDFactor(const BlendFunc val) { m_regVal.fields.blendFuncDFactor = static_cast<uint32_t>(val); }

    TestFunc getDepthFunc() const { return static_cast<TestFunc>(m_regVal.fields.depthFunc); }
    TestFunc getAlphaFunc() const { return static_cast<TestFunc>(m_regVal.fields.alphaFunc); }
    uint8_t getRefAlphaValue() const { return m_regVal.fields.referenceAlphaValue; }
    bool getDepthMask() const { return m_regVal.fields.depthMask; }
    bool getColorMaskA() const { return m_regVal.fields.colorMaskA; }
    bool getColorMaskR() const { return m_regVal.fields.colorMaskR; }
    bool getColorMaskG() const { return m_regVal.fields.colorMaskG; }
    bool getColorMaskB() const { return m_regVal.fields.colorMaskB; }
    BlendFunc getBlendFuncSFactor() const { return static_cast<BlendFunc>(m_regVal.fields.blendFuncSFactor); }
    BlendFunc getBlendFuncDFactor() const { return static_cast<BlendFunc>(m_regVal.fields.blendFuncDFactor); }

    uint32_t serialize() const { return m_regVal.data; }
    void deserialize(const uint32_t data) { m_regVal.data = data; }
    static constexpr uint32_t getAddr() { return 0x3; }

private:
    union RegVal
    {
#pragma pack(push, 1)
        struct RegContent
        {
            RegContent()
                : depthFunc { static_cast<uint32_t>(TestFunc::LESS) }
                , alphaFunc { static_cast<uint32_t>(TestFunc::ALWAYS) }
                , referenceAlphaValue { 0xff }
                , depthMask { false }
                , colorMaskA { true }
                , colorMaskB { true }
                , colorMaskG { true }
                , colorMaskR { true }
                , blendFuncSFactor { static_cast<uint32_t>(BlendFunc::ONE) }
                , blendFuncDFactor { static_cast<uint32_t>(BlendFunc::ZERO) }
            {
            }

            uint32_t depthFunc : 3;
            uint32_t alphaFunc : 3;
            uint32_t referenceAlphaValue : 8;
            uint32_t depthMask : 1;
            uint32_t colorMaskA : 1;
            uint32_t colorMaskB : 1;
            uint32_t colorMaskG : 1;
            uint32_t colorMaskR : 1;
            uint32_t blendFuncSFactor : 4;
            uint32_t blendFuncDFactor : 4;
        } fields {};
        uint32_t data;
#pragma pack(pop)
    } m_regVal;
};
} // namespace rr

#endif // _FRAGMENT_PIPELINE_REG_
