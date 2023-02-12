#ifndef _COLOR_BUFFER_CLEAR_COLOR_REG_
#define _COLOR_BUFFER_CLEAR_COLOR_REG_

#include "registers/BaseColorReg.hpp"

namespace rr
{
class ColorBufferClearColorReg : public BaseColorReg
{
public:
    static constexpr uint32_t getAddr() { return 0x1; }
};
} // namespace rr

#endif // _COLOR_BUFFER_CLEAR_COLOR_REG_
