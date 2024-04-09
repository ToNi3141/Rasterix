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

#ifndef VERTEXPIPELINE_HPP
#define VERTEXPIPELINE_HPP

#include "Vec.hpp"
#include "PixelPipeline.hpp"
#include "Mat44.hpp"
#include "Clipper.hpp"
#include "Lighting.hpp"
#include "ViewPort.hpp"
#include "TexGen.hpp"
#include "RenderObj.hpp"
#include "IRenderer.hpp"
#include "Stack.hpp"
#include "MatrixStack.hpp"
#include "Culling.hpp"
#include "Types.hpp"
#include "FixedSizeQueue.hpp"
#include "PrimitiveAssembler.hpp"

namespace rr
{
class VertexPipeline
{
public:

    VertexPipeline(PixelPipeline& renderer);

    bool drawObj(const RenderObj &obj);

    void setEnableNormalizing(const bool enable) { m_enableNormalizing = enable; }
    
    void setLineWidth(const float width);

    void activateTmu(const uint8_t tmu) { m_tmu = tmu; m_matrixStack.setTmu(tmu); }

    Lighting& getLighting();
    TexGen& getTexGen();
    ViewPort& getViewPort();
    MatrixStack& getMatrixStack();
    Culling& getCulling();

private:
    bool drawTriangle(const PrimitiveAssembler::Triangle &triangle);
    void fetch(Vec4& vertex, Vec4& color, std::array<Vec4, IRenderer::MAX_TMU_COUNT>& tex, Vec3& normal, const RenderObj &obj, const uint32_t index);
    void transform(RenderObj::VertexParameter& parameter, const RenderObj& obj);

    bool m_enableNormalizing { true };

    // Current active TMU
    uint8_t m_tmu {};

    PixelPipeline& m_renderer;
    Lighting m_lighting;
    ViewPort m_viewPort;
    MatrixStack m_matrixStack;
    Culling m_culling;
    std::array<TexGen, IRenderer::MAX_TMU_COUNT> m_texGen;
    PrimitiveAssembler m_primitiveAssembler { m_viewPort };
};

} // namespace rr
#endif // VERTEXPIPELINE_HPP
