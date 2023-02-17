#ifndef _WRITE_REGISTER_CMD_HPP_
#define _WRITE_REGISTER_CMD_HPP_

#include <cstdint>
#include <array>
#include <tcb/span.hpp>

namespace rr
{

template <class TRegister>
class WriteRegisterCmd
{
    static constexpr uint32_t OP_RENDER_CONFIG = 0x1000'0000;
public:
    WriteRegisterCmd(const TRegister& reg)
    {
        m_val = reg.serialize();
        m_op = OP_RENDER_CONFIG | reg.getAddr();
    }

    using Desc = std::array<tcb::span<uint32_t>, 1>;
    void serialize(Desc& desc) const { desc[0][0] = m_val; }
    uint32_t command() const { return m_op; }

private:
    uint32_t m_op {};
    uint32_t m_val;
};

} // namespace rr

#endif // _WRITE_REGISTER_CMD_HPP_
