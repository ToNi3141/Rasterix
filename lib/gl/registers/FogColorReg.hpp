#ifndef _FOG_COLOR_REG_
#define _FOG_COLOR_REG_

#include "registers/BaseColorReg.hpp"

namespace rr
{
class FogColorReg : public BaseColorReg
{
public:
    static constexpr uint32_t getAddr() { return 0x4; }
};
} // namespace rr

#endif // _FOG_COLOR_REG_
