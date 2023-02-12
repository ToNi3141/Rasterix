#ifndef _SCISSOR_END_REG_
#define _SCISSOR_END_REG_

#include "registers/BaseXYReg.hpp"

namespace rr
{
class ScissorEndReg : public BaseXYReg<0x7ff, 0x7ff>
{
public:
    static constexpr uint32_t getAddr() { return 0x6; }
};
} // namespace rr

#endif // _SCISSOR_END_REG_
