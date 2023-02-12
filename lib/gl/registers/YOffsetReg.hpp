#ifndef _Y_OFFSET_REG_
#define _Y_OFFSET_REG_

#include "registers/BaseXYReg.hpp"

namespace rr
{
class YOffsetReg : public BaseXYReg<0x0, 0x7ff>
{
public:
    static constexpr uint32_t getAddr() { return 0x7; }
};
} // namespace rr

#endif // _Y_OFFSET_REG_
