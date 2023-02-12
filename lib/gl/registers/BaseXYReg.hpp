#ifndef _BASE_XY_REG_
#define _BASE_XY_REG_

#include <cstdint>
#include "Veci.hpp"

namespace rr
{
template <uint16_t MASK_X, uint16_t MASK_Y>
class BaseXYReg
{
public:
    BaseXYReg() = default;

    void setX(const uint16_t val) { m_regValX = val & MASK_X; }
    void setY(const uint16_t val) { m_regValY = val & MASK_Y; }

    uint32_t getX() const { return m_regValX; }
    uint32_t getY() const { return m_regValY; }

    uint32_t serialize() const { return (static_cast<uint32_t>(m_regValY) << 16) | (static_cast<uint32_t>(m_regValX)); }
private:
    uint16_t m_regValX { 0 };
    uint16_t m_regValY { 0 };
};
} // namespace rr

#endif // _BASE_XY_REG_
