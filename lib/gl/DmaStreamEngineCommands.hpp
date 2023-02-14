#ifndef _DMA_STREAM_ENGINE_COMMANDS_HPP_
#define _DMA_STREAM_ENGINE_COMMANDS_HPP_

#include <cstdint>

namespace rr
{

namespace DSEC
{
// OPs for the DMA Stream Engine
static constexpr uint32_t NOP { 0x0000'0000 };
static constexpr uint32_t STORE { 0xD000'0000 };
static constexpr uint32_t LOAD { 0xB000'0000 };
static constexpr uint32_t STREAM { 0x9000'0000 };
static constexpr uint32_t COMMIT_TO_STREAM { 0x6000'0000 };
static constexpr uint32_t COMMIT_TO_MEMORY { 0xE000'0000 };

struct Transfer
{
    uint32_t addr;
    uint32_t size;
};

} // namespace DSEC

} // namespace rr

#endif // _DMA_STREAM_ENGINE_COMMANDS_HPP_