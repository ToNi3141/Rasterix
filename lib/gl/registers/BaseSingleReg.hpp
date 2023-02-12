#ifndef _BASE_SINGLE_REG_
#define _BASE_SINGLE_REG_

#include <cstdint>
#include "Veci.hpp"

namespace rr
{
template <uint32_t MASK>
class BaseSingleReg
{
public:
    BaseSingleReg() = default;

    void setValue(const uint32_t val) { m_regVal = val & MASK; }

    uint32_t getValue() const { return m_regVal; }

    uint32_t serialize() const { return m_regVal; }
private:
    uint32_t m_regVal { 0 };
};
} // namespace rr

#endif // _BASE_SINGLE_REG_
