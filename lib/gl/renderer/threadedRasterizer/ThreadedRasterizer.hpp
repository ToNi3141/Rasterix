// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

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

#ifndef _THREADED_RASTERIZER_HPP_
#define _THREADED_RASTERIZER_HPP_

#include "renderer/IDevice.hpp"
#include "renderer/displaylist/DisplayList.hpp"
#include "renderer/displaylist/DisplayListAssembler.hpp"
#include "renderer/displaylist/DisplayListDoubleBuffer.hpp"
#include "renderer/displaylist/RRXDisplayListAssembler.hpp"
#include <cstdint>
#include <tcb/span.hpp>

#include "renderer/commands/FogLutStreamCmd.hpp"
#include "renderer/commands/FramebufferCmd.hpp"
#include "renderer/commands/NopCmd.hpp"
#include "renderer/commands/PushVertexCmd.hpp"
#include "renderer/commands/RegularTriangleCmd.hpp"
#include "renderer/commands/SetVertexCtxCmd.hpp"
#include "renderer/commands/TextureStreamCmd.hpp"
#include "renderer/commands/TriangleStreamCmd.hpp"
#include "renderer/commands/WriteRegisterCmd.hpp"

#include "renderer/Rasterizer.hpp"
#include "renderer/registers/BaseColorReg.hpp"

#include "renderer/registers/FeatureEnableReg.hpp"
#include "renderer/registers/ScissorEndReg.hpp"
#include "renderer/registers/ScissorStartReg.hpp"

#include <spdlog/spdlog.h>

namespace rr
{

template <std::size_t BUFFER_COUNT, std::size_t BUFFER_SIZE>
class ThreadedRasterizer : public IDevice
{
public:
    ThreadedRasterizer(IDevice& device)
        : m_device { device }
    {
        m_displayListAssembler[0].setBuffer(m_device.requestDisplayListBuffer(0), 0);
        m_displayListAssembler[1].setBuffer(m_device.requestDisplayListBuffer(1), 1);
        m_rasterizer.enableTmu(0, true);
        SPDLOG_INFO("Treaded rasterization enabled");
    }

    void streamDisplayList(const uint8_t index, const uint32_t size) override
    {
        displaylist::DisplayList srcList {};
        srcList.setBuffer(requestDisplayListBuffer(index));
        srcList.resetGet();
        srcList.setCurrentSize(size);

        while (!srcList.atEnd())
        {
            if (!decodeAndCopyCommand(srcList))
            {
                swapAndPrepareDisplayList();
            }
        }
        if (m_displayLists.getBack().getDisplayListSize() > 0)
        {
            swapAndPrepareDisplayList();
        }
    }

    void writeToDeviceMemory(tcb::span<const uint8_t> data, const uint32_t addr) override
    {
        waitTillBusIsFree();
        m_device.writeToDeviceMemory(data, addr);
    }

    bool clearToSend() override
    {
        return true;
    }

    tcb::span<uint8_t> requestDisplayListBuffer(const uint8_t index) override
    {
        return { m_buffer[index] };
    }

    uint8_t getDisplayListBufferCount() const override
    {
        return m_buffer.size();
    }

private:
    // It is advantageous to split the display list in smaller chunks for uploading.
    // The goal is to start a rendering as early as possible and to run the rendering
    // and transforming in parallel.
    static constexpr std::size_t VERTEX_INTERMEDIATE_UPLOAD_SIZE { 100 };

    // This describes the free capacity in the display list for the triangles.
    // When the triangle is transformed and clipped, around 9 new triangles can
    // be generated. This variable keeps a minimum of 10 (to be safe) triangle
    // commands in the display list free for the worst case.
    static constexpr std::size_t TRIANGLE_RESERVE_CAPACITY { 10 };

    template <typename TCmd>
    bool copyCmd(displaylist::DisplayList& src)
    {
        return m_displayLists.getBack().copyCommand<TCmd>(src);
    }

    bool setVertexCtx(displaylist::DisplayList& src)
    {
        using PayloadType = typename std::remove_const<typename std::remove_reference<decltype(SetVertexCtxCmd {}.payload()[0])>::type>::type;
        src.getNext<typename SetVertexCtxCmd::CommandType>();
        const PayloadType* t = src.getNext<PayloadType>();

        new (&m_vertexTransform) vertextransforming::VertexTransformingCalc<decltype(drawTriangleLambda), decltype(setStencilBufferConfigLambda)> {
            t->ctx,
            drawTriangleLambda,
            setStencilBufferConfigLambda,
        };

        m_verticesInList = 0;

        return true;
    }

    bool pushVertex(displaylist::DisplayList& src)
    {
        if (m_displayLists.getBack().getFreeSpace() < (m_displayLists.getBack().getCommandSize<TriangleStreamCmd>(1) * TRIANGLE_RESERVE_CAPACITY)
            || (m_verticesInList >= VERTEX_INTERMEDIATE_UPLOAD_SIZE))
        {
            swapAndPrepareDisplayList();
            m_verticesInList = 0;
        }
        using PayloadType = typename std::remove_const<typename std::remove_reference<decltype(PushVertexCmd {}.payload()[0])>::type>::type;
        src.getNext<typename PushVertexCmd::CommandType>();
        return m_vertexTransform.pushVertex(src.getNext<PayloadType>()->vertex);
    }

    bool addTriangleCmd(displaylist::DisplayList& src)
    {
        using PayloadType = typename std::remove_const<typename std::remove_reference<decltype(RegularTriangleCmd {}.payload()[0])>::type>::type;
        if (m_displayLists.getBack().getFreeSpace()
            < m_displayLists.getBack().getCommandSize<TriangleStreamCmd>(1))
        {
            return false;
        }
        src.getNext<typename RegularTriangleCmd::CommandType>();
        const PayloadType* t = src.getNext<PayloadType>();

        return addTriangleCmd({
                                  t->vertex0,
                                  t->vertex1,
                                  t->vertex2,
                                  t->texture0,
                                  t->texture1,
                                  t->texture2,
                                  t->color0,
                                  t->color1,
                                  t->color2,
                              },
            t->lineStart, t->lineEnd);
    }

    bool addTriangleCmd(const TransformedTriangle& triangle, const std::size_t lineStart, const std::size_t lineEnd)
    {
        TriangleStreamCmd tsc { m_rasterizer, triangle };

        if (tsc.isVisible())
        {
            return m_displayLists.getBack().addCommand(tsc.getIncremented(lineStart, lineEnd));
        }
        return true;
    }

    bool addTriangleCmd(const TransformedTriangle& triangle)
    {
        TriangleStreamCmd tsc { m_rasterizer, triangle };

        if (tsc.isVisible())
        {
            return m_displayLists.getBack().addCommand(tsc);
        }
        return true;
    }

    void updateRasterizer(const displaylist::DisplayList& src)
    {
        const uint32_t op = *(src.lookAhead<uint32_t>(1));
        const uint32_t regData = *(src.lookAhead<uint32_t>(2));
        switch (WriteRegisterCmd<BaseColorReg>::getRegAddr(op))
        {
        case FeatureEnableReg::getAddr():
        {
            FeatureEnableReg reg {};
            reg.deserialize(regData);
            m_rasterizer.enableScissor(reg.getEnableScissor());
            m_rasterizer.enableTmu(0, reg.getEnableTmu(0));
            m_rasterizer.enableTmu(1, reg.getEnableTmu(1));
        }
        break;
        case ScissorStartReg::getAddr():
        {
            ScissorStartReg reg {};
            reg.deserialize(regData);
            m_rasterizer.setScissorStart(reg.getX(), reg.getY());
        }
        break;
        case ScissorEndReg::getAddr():
        {
            ScissorEndReg reg {};
            reg.deserialize(regData);
            m_rasterizer.setScissorEnd(reg.getX(), reg.getY());
        }
        break;

        default:
            break;
        }
    }

    bool decodeAndCopyCommand(displaylist::DisplayList& srcList)
    {
        const uint32_t op = *(srcList.lookAhead<uint32_t>());
        bool ret = true;
        if (RegularTriangleCmd::isThis(op))
        {
            ret = addTriangleCmd(srcList);
        }
        else if (TriangleStreamCmd::isThis(op))
        {
            ret = copyCmd<TriangleStreamCmd>(srcList);
        }
        else if (SetVertexCtxCmd::isThis(op))
        {
            ret = setVertexCtx(srcList);
        }
        else if (PushVertexCmd::isThis(op))
        {
            ret = pushVertex(srcList);
        }
        else if (WriteRegisterCmd<BaseColorReg>::isThis(op))
        {
            updateRasterizer(srcList);
            ret = copyCmd<WriteRegisterCmd<BaseColorReg>>(srcList);
        }
        else if (NopCmd::isThis(op))
        {
            ret = copyCmd<NopCmd>(srcList);
        }
        else if (TextureStreamCmd::isThis(op))
        {
            ret = copyCmd<TextureStreamCmd>(srcList);
        }
        else if (FramebufferCmd::isThis(op))
        {
            ret = copyCmd<FramebufferCmd>(srcList);
        }
        else if (FogLutStreamCmd::isThis(op))
        {
            ret = copyCmd<FogLutStreamCmd>(srcList);
        }
        else
        {
            SPDLOG_CRITICAL("Unknown command (0x{:X})found. This might cause the renderer to crash ...", op);
        }
        return ret;
    }

    void waitTillBusIsFree()
    {
        while (!m_device.clearToSend())
            ;
    }

    void swapAndPrepareDisplayList()
    {
        waitTillBusIsFree();
        m_device.streamDisplayList(m_displayLists.getBack().getDisplayListBufferId(), m_displayLists.getBack().getDisplayListSize());
        m_displayLists.swap();
        m_displayLists.getBack().clearAssembler();
    }

    bool setStencilBufferConfig(const StencilReg& stencilConf)
    {
        return m_displayLists.getBack().addCommand(WriteRegisterCmd<StencilReg> { stencilConf });
    }

    using ConcreteDisplayListAssembler = displaylist::DisplayListAssembler<RenderConfig::TMU_COUNT, displaylist::DisplayList, false>;

    IDevice& m_device;
    std::array<std::array<uint8_t, BUFFER_SIZE>, BUFFER_COUNT> m_buffer;
    std::array<ConcreteDisplayListAssembler, 2> m_displayListAssembler {};
    displaylist::DisplayListDoubleBuffer<ConcreteDisplayListAssembler> m_displayLists {
        m_displayListAssembler[0],
        m_displayListAssembler[1],
    };
    Rasterizer m_rasterizer { !RenderConfig::USE_FLOAT_INTERPOLATION };

    const std::function<bool(const TransformedTriangle&)> drawTriangleLambda = [this](const TransformedTriangle& triangle)
    {
        m_verticesInList++;
        return addTriangleCmd(triangle);
    };
    const std::function<bool(const StencilReg&)> setStencilBufferConfigLambda = [this](const StencilReg& stencilConf)
    { return setStencilBufferConfig(stencilConf); };

    vertextransforming::VertexTransformingCalc<decltype(drawTriangleLambda), decltype(setStencilBufferConfigLambda)> m_vertexTransform {
        {},
        drawTriangleLambda,
        setStencilBufferConfigLambda,
    };

    std::size_t m_verticesInList { 0 };
};

} // namespace rr

#endif // _THREADED_RASTERIZER_HPP_
