#ifndef _FOG_LUT_STREAM_DESC_
#define _FOG_LUT_STREAM_DESC_

#include <cstdint>
#include <array>
#include <tcb/span.hpp>

namespace rr
{

class FogLutStreamDesc
{
    static constexpr std::size_t LUT_SIZE { 33 };
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

    tcb::span<const uint64_t> serialize() const { return { reinterpret_cast<const uint64_t*>(m_lut.data()), m_lut.size() }; }
    static constexpr std::size_t size() { return LUT_SIZE; }
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

#endif // _FOG_LUT_STREAM_DESC_
