#ifndef _RENDER_RESOLUTION_REG_
#define _RENDER_RESOLUTION_REG_

#include "registers/BaseXYReg.hpp"

namespace rr
{
class RenderResolutionReg : public BaseXYReg<0x7ff, 0x7ff>
{
public:
    static constexpr uint32_t getAddr() { return 0x8; }
};
} // namespace rr

#endif // _RENDER_RESOLUTION_REG_
