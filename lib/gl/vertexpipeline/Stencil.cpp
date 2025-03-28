// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2024 ToNi3141

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

#include "Stencil.hpp"
#include "pixelpipeline/PixelPipeline.hpp"
#include "renderer/Rasterizer.hpp"
#include <spdlog/spdlog.h>

namespace rr::stencil
{
StencilSetter::StencilSetter(PixelPipeline& renderer, StencilData& stencilData)
    : m_renderer { renderer }
    , m_data { stencilData }
{
    m_renderer.setStencilBufferConfig(m_stencilConf);
}

StencilReg& StencilSetter::stencilConfig()
{
    if (m_data.enableTwoSideStencil)
    {
        if (m_stencilFace == StencilFace::FRONT)
        {
            return m_data.stencilConfFront;
        }
        return m_data.stencilConfBack;
    }
    return m_stencilConf;
}

bool StencilSetter::update()
{
    bool ret { true };

    // if (!config.enableTwoSideStencil && m_stencilConfUploaded.serialize() != m_stencilConf.serialize())
    {
        ret = ret && m_renderer.setStencilBufferConfig(m_stencilConf);
        m_stencilConfUploaded = m_stencilConf;
    }

    return ret;
}

} // namespace rr::stencil
