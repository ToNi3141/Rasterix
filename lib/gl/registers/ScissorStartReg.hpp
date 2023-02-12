#ifndef _SCISSOR_START_REG_
#define _SCISSOR_START_REG_

#include "registers/BaseXYReg.hpp"

namespace rr
{
class ScissorStartReg : public BaseXYReg<0x7ff, 0x7ff>
{
public:
    static constexpr uint32_t getAddr() { return 0x5; }
};
} // namespace rr

#endif // _SCISSOR_START_REG_
