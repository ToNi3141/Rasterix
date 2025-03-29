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

#include "VertexPipeline.hpp"
#include "math/Vec.hpp"
#include "math/Veci.hpp"
#include <cmath>
#include <spdlog/spdlog.h>
#include <stdlib.h>
#include <string.h>

// The Arduino IDE will produce compile errors when using std::min and std::max
#include <algorithm> // std::max
#define max std::max
#define min std::min

namespace rr
{
VertexPipeline::VertexPipeline(PixelPipeline& renderer)
    : m_renderer { renderer }
{
    for (std::size_t i = 0; i < m_texGen.size(); i++)
    {
        m_texGen[i].setNormalMat(m_vertexCtx.transformMatrices.normal);
        m_texGen[i].setTexGenData(m_vertexCtx.texGen[i]);
    }
    setEnableNormalizing(false);
}

VertexParameter VertexPipeline::fetch(const RenderObj& obj, std::size_t i)
{
    VertexParameter parameter;
    const std::size_t pos = obj.getIndex(i);
    parameter.vertex = obj.getVertex(pos);
    parameter.normal = obj.getNormal(pos);
    parameter.color = obj.getColor(pos);
    for (std::size_t tu = 0; tu < RenderConfig::TMU_COUNT; tu++)
    {
        parameter.tex[tu] = obj.getTexCoord(tu, pos);
    }
    return parameter;
}

bool VertexPipeline::drawObj(const RenderObj& obj)
{
    if (!obj.vertexArrayEnabled())
    {
        SPDLOG_INFO("drawObj(): Vertex array disabled. No primitive is rendered.");
        return true;
    }
    m_matrixStore.recalculateMatrices();
    stencil().update();
    if (!m_renderer.updatePipeline())
    {
        SPDLOG_ERROR("drawObj(): Cannot update pixel pipeline");
        return false;
    }
    obj.logCurrentConfig();

    m_primitiveAssembler.setDrawMode(obj.getDrawMode());
    m_primitiveAssembler.setExpectedPrimitiveCount(obj.getCount());

    for (std::size_t i = 0; i < RenderConfig::TMU_COUNT; i++)
    {
        m_vertexCtx.tmuEnabled[i] = m_renderer.featureEnable().getEnableTmu(i);
    }
    m_renderer.setVertexContext(m_vertexCtx);

    std::size_t count = obj.getCount();
    for (std::size_t it = 0; it < count; it++)
    {
        VertexParameter vertex = fetch(obj, it);
        m_renderer.pushVertex(vertex);
    }

    return true;
}

} // namespace rr
