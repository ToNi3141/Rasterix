#ifndef _BASE_COLOR_REG_
#define _BASE_COLOR_REG_

#include <cstdint>
#include "Veci.hpp"

namespace rr
{
class BaseColorReg
{
public:
    BaseColorReg() = default;

    void setColor(const Vec4i val) { setRed(val[0]); setGreen(val[1]); setBlue(val[2]); setAlpha(val[3]); }

    void setRed(const uint8_t val) { m_regVal.fields.red = val; }
    void setGreen(const uint8_t val) { m_regVal.fields.green = val; }
    void setBlue(const uint8_t val) { m_regVal.fields.blue = val; }
    void setAlpha(const uint8_t val) { m_regVal.fields.alpha = val; }

    uint8_t getRed() const { return m_regVal.fields.red; }
    uint8_t getGreen() const { return m_regVal.fields.green; }
    uint8_t getBlue() const { return m_regVal.fields.blue; }
    uint8_t getAlpha() const { return m_regVal.fields.alpha; }

    uint32_t serialize() const { return m_regVal.data; }
private:
    union RegVal
    {
        #pragma pack(push, 1)
        struct RegContent
        {
            RegContent()
                : alpha(0)
                , blue(0)
                , green(0)
                , red(0)
            { }

            uint32_t alpha : 8;
            uint32_t blue : 8;
            uint32_t green : 8;
            uint32_t red : 8;
        } fields {};
        uint32_t data;
        #pragma pack(pop)
    } m_regVal;
};
} // namespace rr

#endif // _BASE_COLOR_REG_
