#include "Fog.hpp"

namespace rr
{

Fog::Fog(IRenderer& renderer)
    : m_renderer(renderer)
{
}

void Fog::setFogMode(const FogMode val)
{
    if (m_fogMode != val)
    {
        m_fogMode = val;
        m_fogDirty = true;
    }
}

void Fog::setFogStart(const float val)
{
    if (m_fogStart != val)
    {
        m_fogStart = val;
        m_fogDirty = true;
    }
}

void Fog::setFogEnd(const float val)
{
    if (m_fogEnd != val)
    {
        m_fogEnd = val;
        m_fogDirty = true;
    }
}

void Fog::setFogDensity(const float val)
{
    if (m_fogDensity != val)
    {
        m_fogDensity = val;
        m_fogDirty = true;
    }
}

bool Fog::setFogColor(const Vec4& val)
{
    Vec4i color = Vec4i::createFromVec<8>(val.vec);
    color.clamp(0, 255);
    return m_renderer.setFogColor(color);
}

bool Fog::updateFogLut()
{
    if (!m_fogDirty) [[unlikely]]
    {
        return true;
    }
    m_fogDirty = false;

    std::function <float(float)> fogFunction;

    // Set fog function
    switch (m_fogMode) {
    case FogMode::LINEAR:
        fogFunction = [&](float z) {
            float f = (m_fogEnd - z) / (m_fogEnd - m_fogStart);
            return f;
        };
        break;
    case FogMode::EXP:
        fogFunction = [&](float z) {
            float f = expf(-(m_fogDensity * z));
            return f;
        };
        break;
    case FogMode::EXP2:
        fogFunction = [&](float z) {
            float f = expf(powf(-(m_fogDensity * z), 2));
            return f;
        };
        break;
    default:
        fogFunction = [](float) {
            return 1.0f;
        };
        break;
    }

    // Calculate fog LUT
    std::array<float, 33> lut;
    for (std::size_t i = 0; i < lut.size(); i++)
    {
        float f = fogFunction(powf(2, i));
        lut[i] = f;
    }

    // Set fog LUT
    return m_renderer.setFogLut(lut, m_fogStart, m_fogEnd);
}

} // namespace rr