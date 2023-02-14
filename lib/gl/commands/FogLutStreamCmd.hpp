#ifndef _FOG_LUT_STREAM_CMD_HPP_
#define _FOG_LUT_STREAM_CMD_HPP_

#include <cstdint>
#include <array>
#include <tcb/span.hpp>

namespace rr
{

class FogLutStreamCmd
{
    static constexpr std::size_t LUT_SIZE { 33 };
    static constexpr uint32_t FOG_LUT_STREAM { 0x4000'0000 };
public:
    void setBounds(const float lower, const float upper)
    {
        m_lut[0].floats.lower = lower;
        m_lut[0].floats.upper = upper;
    }
    
    void setLutValue(const uint8_t index, const float m, const float b)
    {
        m_lut[index + 1].numbers.m = m;
        m_lut[index + 1].numbers.b = b;
    }

    using ValType = uint64_t;
    using Desc = std::array<tcb::span<ValType>, LUT_SIZE>;
    void serialize(Desc& desc) const 
    { 
        for (uint8_t i = 0; i < desc.size(); i++)
        {
            *(desc[i].data()) = m_lut[i].val;
        }
    }
    static constexpr std::size_t size() { return LUT_SIZE; }
    static constexpr uint32_t command() { return FOG_LUT_STREAM; }
private:
    union Value {
#pragma pack(push, 4)
        uint64_t val;
        struct {
            int32_t m;
            int32_t b;
        } numbers;
        struct {
            float lower;
            float upper;
        } floats;
#pragma pack(pop)
    };

    std::array<Value, LUT_SIZE> m_lut;
};

} // namespace rr

#endif // _FOG_LUT_STREAM_CMD_HPP_
