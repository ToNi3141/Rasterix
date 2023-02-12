#ifndef _DEPTH_BUFFER_CLEAR_DEPTH_REG_
#define _DEPTH_BUFFER_CLEAR_DEPTH_REG_

#include "registers/BaseSingleReg.hpp"

namespace rr
{
class DepthBufferClearDepthReg : public BaseSingleReg<0xffff>
{
public:
    static constexpr uint32_t getAddr() { return 0x2; }
};
} // namespace rr

#endif // _DEPTH_BUFFER_CLEAR_DEPTH_REG_
