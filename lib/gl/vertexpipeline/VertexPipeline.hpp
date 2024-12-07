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

#include "math/Vec.hpp"
#include "pixelpipeline/PixelPipeline.hpp"
#include "math/Mat44.hpp"
#include "vertexpipeline/Clipper.hpp"
#include "vertexpipeline/Lighting.hpp"
#include "vertexpipeline/ViewPort.hpp"
#include "vertexpipeline/TexGen.hpp"
#include "RenderObj.hpp"
#include "Stack.hpp"
#include "vertexpipeline/MatrixStack.hpp"
#include "vertexpipeline/Culling.hpp"
#include "Types.hpp"
#include "FixedSizeQueue.hpp"
#include "vertexpipeline/PrimitiveAssembler.hpp"

namespace rr
{
class VertexPipeline
{
public:

    VertexPipeline(PixelPipeline& renderer);

    bool drawObj(const RenderObj &obj);
    void setEnableNormalizing(const bool enable) { m_enableNormalizing = enable; }
    void activateTmu(const std::size_t tmu) { m_tmu = tmu; m_matrixStack.setTmu(tmu); }

    Lighting& getLighting() { return m_lighting; }
    TexGen& getTexGen() { return m_texGen[m_tmu]; }
    ViewPort& getViewPort() { return m_viewPort; }
    MatrixStack& getMatrixStack() { return m_matrixStack; }
    Culling& getCulling() { return m_culling; }
    PrimitiveAssembler& getPrimitiveAssembler() { return m_primitiveAssembler; }

private:
    bool drawTriangle(const PrimitiveAssembler::Triangle &triangle);
    void fetchAndTransform(VertexParameter& parameter, const RenderObj& obj, std::size_t i);
    bool drawClippedTriangleList(tcb::span<VertexParameter> list);
    bool drawUnclippedTriangle(const PrimitiveAssembler::Triangle& triangle);

    bool m_enableNormalizing { true };

    // Current active TMU
    std::size_t m_tmu {};

    PixelPipeline& m_renderer;
    Lighting m_lighting;
    ViewPort m_viewPort;
    MatrixStack m_matrixStack;
    Culling m_culling;
    std::array<TexGen, RenderConfig::TMU_COUNT> m_texGen;
    PrimitiveAssembler m_primitiveAssembler { m_viewPort };
};

} // namespace rr
#endif // VERTEXPIPELINE_HPP
