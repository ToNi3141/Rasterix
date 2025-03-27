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

#include "FixedSizeQueue.hpp"
#include "RenderObj.hpp"
#include "Stack.hpp"
#include "Types.hpp"
#include "math/Mat44.hpp"
#include "math/Vec.hpp"
#include "pixelpipeline/PixelPipeline.hpp"
#include "vertexpipeline/Clipper.hpp"
#include "vertexpipeline/Culling.hpp"
#include "vertexpipeline/Lighting.hpp"
#include "vertexpipeline/MatrixStore.hpp"
#include "vertexpipeline/PrimitiveAssembler.hpp"
#include "vertexpipeline/Stencil.hpp"
#include "vertexpipeline/TexGen.hpp"
#include "vertexpipeline/ViewPort.hpp"
#include <cstdint>

namespace rr
{
class VertexPipeline
{
public:
    VertexPipeline(PixelPipeline& renderer);

    bool drawObj(const RenderObj& obj);
    void setEnableNormalizing(const bool enable) { m_enableNormalizing = enable; }
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
    struct VertexContext
    {
        matrixstore::TransformMatricesData transformMatrices {};
        lighting::LightingData lighting {};
        viewport::ViewPortData viewPort {};
        culling::CullingData culling {};
        stencil::StencilData stencil {};
        std::array<texgen::TexGenData, RenderConfig::TMU_COUNT> texGen {};
        primitiveassembler::PrimitiveAssemblerData primitiveAssembler {};
    };

    bool drawTriangle(const primitiveassembler::PrimitiveAssemblerCalc::Triangle& triangle);
    void fetch(VertexParameter& parameter, const RenderObj& obj, std::size_t i);
    void transform(VertexParameter& parameter);
    bool drawClippedTriangleList(tcb::span<VertexParameter> list);
    bool drawUnclippedTriangle(const primitiveassembler::PrimitiveAssemblerCalc::Triangle& triangle);

    VertexContext m_vertexCtx {};

    bool m_enableNormalizing { true };

    // Current active TMU
    std::size_t m_tmu {};

    PixelPipeline& m_renderer;
    stencil::StencilSetter m_stencil { m_renderer, m_vertexCtx.stencil };
    lighting::LightingSetter m_lighting { m_vertexCtx.lighting };
    viewport::ViewPortSetter m_viewPort { m_vertexCtx.viewPort };
    matrixstore::MatrixStore m_matrixStore { m_vertexCtx.transformMatrices };
    culling::CullingSetter m_culling { m_vertexCtx.culling };
    std::array<texgen::TexGenSetter, RenderConfig::TMU_COUNT> m_texGen {};
    primitiveassembler::PrimitiveAssemblerSetter m_primitiveAssembler { m_vertexCtx.primitiveAssembler };
};

} // namespace rr
#endif // VERTEXPIPELINE_HPP
