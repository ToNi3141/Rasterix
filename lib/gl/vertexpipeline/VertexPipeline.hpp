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

#include "RenderObj.hpp"
#include "math/Mat44.hpp"
#include "math/Vec.hpp"
#include "pixelpipeline/PixelPipeline.hpp"
#include "transform/Clipper.hpp"
#include "transform/Culling.hpp"
#include "transform/Lighting.hpp"
#include "transform/MatrixStore.hpp"
#include "transform/PrimitiveAssembler.hpp"
#include "transform/Stencil.hpp"
#include "transform/TexGen.hpp"
#include "transform/VertexTransforming.hpp"
#include "transform/ViewPort.hpp"
#include <cstdint>

namespace rr
{
class VertexPipeline
{
public:
    VertexPipeline(PixelPipeline& renderer);

    bool drawObj(const RenderObj& obj);
    void setEnableNormalizing(const bool enable) { m_vertexCtx.normalizeLightNormal = enable; }
    void activateTmu(const std::size_t tmu)
    {
        m_tmu = tmu;
        m_matrixStore.setTmu(tmu);
    }

    stencil::StencilSetter& stencil() { return m_stencil; }
    lighting::LightingSetter& getLighting() { return m_lighting; }
    texgen::TexGenSetter& getTexGen() { return m_texGen[m_tmu]; }
    viewport::ViewPortSetter& getViewPort() { return m_viewPort; }
    matrixstore::MatrixStore& getMatrixStore() { return m_matrixStore; }
    culling::CullingSetter& getCulling() { return m_culling; }
    primitiveassembler::PrimitiveAssemblerSetter& getPrimitiveAssembler() { return m_primitiveAssembler; }

private:
    bool drawTriangle(const primitiveassembler::PrimitiveAssemblerCalc::Triangle& triangle);
    VertexParameter fetch(const RenderObj& obj, std::size_t i);

    vertextransforming::VertexTransformingData m_vertexCtx {};

    // Current active TMU
    std::size_t m_tmu {};

    PixelPipeline& m_renderer;
    stencil::StencilSetter m_stencil { [this](const StencilReg& reg)
        { return m_renderer.setStencilBufferConfig(reg); },
        m_vertexCtx.stencil };
    lighting::LightingSetter m_lighting { m_vertexCtx.lighting };
    viewport::ViewPortSetter m_viewPort { m_vertexCtx.viewPort };
    matrixstore::MatrixStore m_matrixStore { m_vertexCtx.transformMatrices };
    culling::CullingSetter m_culling { m_vertexCtx.culling };
    std::array<texgen::TexGenSetter, RenderConfig::TMU_COUNT> m_texGen {};
    primitiveassembler::PrimitiveAssemblerSetter m_primitiveAssembler { m_vertexCtx.primitiveAssembler };
};

} // namespace rr
#endif // VERTEXPIPELINE_HPP
