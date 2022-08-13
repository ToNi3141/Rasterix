#ifndef VERILATORBUSCONNECTOR_H
#define VERILATORBUSCONNECTOR_H

#include "verilated.h"
#include "IBusConnector.hpp"
#include "Vtop.h"

template <typename FBType>
class VerilatorBusConnector : public IBusConnector
{
public:
    virtual ~VerilatorBusConnector() = default;

    VerilatorBusConnector(FBType *framebuffer, const uint16_t resolutionW = 128, const uint16_t resolutionH = 128)
        : m_resolutionW(resolutionW), m_resolutionH(resolutionH), m_framebuffer(framebuffer)
    {
        m_top.m_framebuffer_axis_tready = 1;
        m_top.s_cmd_axis_tvalid = 0;

        m_top.resetn = 0;
        clk();
        clk(); // It needs an additional clock cycle, otherwise it will not work. Currently i dont know why
        m_top.resetn = 1;
        clk();
    }

    virtual void writeData(const uint8_t* data, const uint32_t bytes) override
    {
        // Convert data to 32 bit variables to ease the access
        const uint64_t *data64 = reinterpret_cast<const uint64_t*>(data);
        const uint64_t bytes64 = bytes / 8;
        for (uint16_t i = 0; i < bytes64; )
        {
            if (m_top.s_cmd_axis_tready)
            {
                m_top.s_cmd_axis_tdata = data64[i];
                m_top.s_cmd_axis_tvalid = 1;
                i++;
            }
            clk();
        }
        m_top.s_cmd_axis_tvalid = 0;
    }

    virtual bool clearToSend() override
    {
        return true;
    }

    virtual void startColorBufferTransfer(const uint8_t) override
    {
    }

    void clk()
    {
        m_top.aclk = 1;
        m_top.eval();
        m_top.aclk = 0;
        m_top.eval();

        if (m_top.resetn == 0)
            return;

       if (m_top.m_framebuffer_axis_tvalid && (m_framebuffer != nullptr))
       {
           m_framebuffer[m_streamAddr] = m_top.m_framebuffer_axis_tdata;
           m_streamAddr++;
       }

       if (m_streamAddr == (m_resolutionW * m_resolutionH / (sizeof(FBType) / 2))) // 4 for 32 bit color
       {
           m_streamAddr = 0;
       }
    }
private:
    const uint16_t m_resolutionW = 128;
    const uint16_t m_resolutionH = 128;
    FBType *m_framebuffer = nullptr;
    uint32_t m_streamAddr = 0;
    Vtop m_top;
};

#endif // VERILATORBUSCONNECTOR_H
