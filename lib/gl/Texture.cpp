#include "Texture.hpp"

namespace rr
{

Texture::Texture(IRenderer& renderer)
    : m_renderer(renderer)
{
    for (uint8_t i = 0; i < MAX_TMU_COUNT; i++)
    {
        m_tmuConf[i].texEnvConf.setTmu(i);
        m_renderer.setTexEnv(m_tmuConf[i].texEnvConf);
    }
}

bool Texture::uploadTexture()
{
    bool ret { true };

    for (uint8_t i = 0; i < MAX_TMU_COUNT; i++)
    {
        if ((m_tmuConf[i].texEnvMode == TexEnvMode::COMBINE) && (m_tmuConf[i].texEnvConfUploaded.serialize() != m_tmuConf[i].texEnvConf.serialize())) [[unlikely]]
        {
            m_tmuConf[i].texEnvConf.setTmu(i);
            ret = ret && m_renderer.setTexEnv(m_tmuConf[i].texEnvConf);
            m_tmuConf[i].texEnvConfUploaded = m_tmuConf[i].texEnvConf;
        }
    }

    if (m_textureObjectMipmap)
    {
        ret = m_renderer.updateTexture(m_tmuConf[m_tmu].boundTexture, *m_textureObjectMipmap);
        m_textureObjectMipmap = std::nullopt;

        // Rebind texture to update the rasterizer with the new texture meta information
        // TODO: Check if this is still required
        ret = ret && useTexture();
    }
    return ret;
}

TextureObjectMipmap& Texture::getTexture()
{
    if (!m_textureObjectMipmap)
    {
        m_textureObjectMipmap = std::make_optional<TextureObjectMipmap>(m_renderer.getTexture(m_tmuConf[m_tmu].boundTexture));
    }
    return *m_textureObjectMipmap;
}



bool Texture::useTexture()
{
    uploadTexture();
    return m_renderer.useTexture(m_tmu, m_tmuConf[m_tmu].boundTexture);
}

bool Texture::setTexEnvMode(const TexEnvMode mode)
{
    m_tmuConf[m_tmu].texEnvMode = mode;
    TexEnvReg texEnvConf {};
    switch (mode) {
    case TexEnvMode::DISABLE:
        texEnvConf.setCombineRgb(TexEnvReg::Combine::REPLACE);
        texEnvConf.setCombineRgb(TexEnvReg::Combine::REPLACE);
        texEnvConf.setCombineAlpha(TexEnvReg::Combine::REPLACE);
        texEnvConf.setSrcRegRgb0(TexEnvReg::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::PRIMARY_COLOR);
        if (m_tmu != 0) // Is this the right mode?
        {
            texEnvConf.setSrcRegRgb0(TexEnvReg::SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::REPLACE:
        texEnvConf.setCombineRgb(TexEnvReg::Combine::REPLACE);
        texEnvConf.setCombineAlpha(TexEnvReg::Combine::REPLACE);
        texEnvConf.setSrcRegRgb0(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::TEXTURE);
        break;
    case TexEnvMode::MODULATE:
        texEnvConf.setCombineRgb(TexEnvReg::Combine::MODULATE);
        texEnvConf.setCombineAlpha(TexEnvReg::Combine::MODULATE);
        texEnvConf.setSrcRegRgb0(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha1(TexEnvReg::SrcReg::PRIMARY_COLOR);
        if (m_tmu != 0) // Is this the right mode?
        {
            texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha1(TexEnvReg::SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::DECAL:
        texEnvConf.setCombineRgb(TexEnvReg::Combine::INTERPOLATE);
        texEnvConf.setCombineAlpha(TexEnvReg::Combine::REPLACE);
        texEnvConf.setSrcRegRgb0(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegRgb2(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::PRIMARY_COLOR);
        texEnvConf.setOperandRgb2(TexEnvReg::Operand::SRC_ALPHA);
        if (m_tmu != 0) // Is this the right mode? 
        {
            texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::BLEND:
        texEnvConf.setCombineRgb(TexEnvReg::Combine::INTERPOLATE);
        texEnvConf.setCombineAlpha(TexEnvReg::Combine::MODULATE);
        texEnvConf.setSrcRegRgb0(TexEnvReg::SrcReg::CONSTANT);
        texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegRgb2(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha1(TexEnvReg::SrcReg::TEXTURE);
        if (m_tmu != 0) // Is this the right mode?
        {
            texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::ADD:
        texEnvConf.setCombineRgb(TexEnvReg::Combine::ADD);
        texEnvConf.setCombineAlpha(TexEnvReg::Combine::ADD);
        texEnvConf.setSrcRegRgb0(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha1(TexEnvReg::SrcReg::PRIMARY_COLOR);
        if (m_tmu != 0) // Is this the right mode?
        {
            texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha1(TexEnvReg::SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::COMBINE:
        // Nothing to do here.
        break;
    default:
        break;
    }
    if (mode != TexEnvMode::COMBINE)
    {
        texEnvConf.setTmu(m_tmu);
        return m_renderer.setTexEnv(texEnvConf);
    }
    return true;
}

bool Texture::setTexEnvColor(const Vec4& color)
{
    return m_renderer.setTexEnvColor(m_tmu, { { static_cast<uint8_t>(color[0] * 255.0f),
                                                static_cast<uint8_t>(color[1] * 255.0f),
                                                static_cast<uint8_t>(color[2] * 255.0f),
                                                static_cast<uint8_t>(color[3] * 255.0f) } });
}
} // namespace rr