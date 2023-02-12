#ifndef _TEX_ENV_COLOR_REG_
#define _TEX_ENV_COLOR_REG_

#include "registers/BaseColorReg.hpp"

namespace rr
{
class TexEnvColorReg : public BaseColorReg
{
public:
    void setTmu(const uint8_t tmu) { m_offset = tmu * TMU_OFFSET; }
    uint32_t getAddr() const { return 0xA + m_offset; }
private:
    static constexpr uint8_t TMU_OFFSET { 3 };
    uint8_t m_offset { 0 };
};
} // namespace rr

#endif // _TEX_ENV_COLOR_REG_
