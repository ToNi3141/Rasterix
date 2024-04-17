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

#ifndef FRAGMENT_PIPELINE_HPP_
#define FRAGMENT_PIPELINE_HPP_

#include "IRenderer.hpp"
#include "Vec.hpp"
#include <optional>

namespace rr
{
class FragmentPipeline
{
public:
    using PipelineConfig = FragmentPipelineReg;

    FragmentPipeline(IRenderer& renderer) : m_renderer(renderer)
    {
        m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf);
    }

    PipelineConfig& config() { return m_fragmentPipelineConf; }

    bool update()
    {
        bool ret { true };

        if (m_fragmentPipelineConfUploaded.serialize() != m_fragmentPipelineConf.serialize()) [[unlikely]]
        {
            ret = ret && m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf);
            m_fragmentPipelineConfUploaded = m_fragmentPipelineConf;
        }

        return ret;
    }

private:
    IRenderer& m_renderer;
    PipelineConfig m_fragmentPipelineConf {};
    PipelineConfig m_fragmentPipelineConfUploaded {};
};

} // namespace rr
#endif // FRAGMENT_PIPELINE_HPP_
