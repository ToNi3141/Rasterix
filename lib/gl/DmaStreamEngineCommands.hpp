#ifndef _DMA_STREAM_ENGINE_COMMANDS_HPP_
#define _DMA_STREAM_ENGINE_COMMANDS_HPP_

#include <cstdint>

namespace rr
{

namespace DSEC
{
using SCT = uint32_t;

// OPs for the DMA Stream Engine
// Refer the DmaStreamEngine.v for more details. In short the anatomy of a command:
// Standard operation
// Beat 1:
// +----------+---------+-----------------------------+
// | 2'hx out | 2'hx in | 28'h stream length in bytes |
// +----------+---------+-----------------------------+
// Beat 2:
// +--------------------------------------------------+
// | 32'h addr (only evaluated when mem_axi is used)  |
// +--------------------------------------------------+
// Beat 3 .. n:
// +--------------------------------------------------+
// | STREAM_WIDTH'h payload                           |
// +--------------------------------------------------+

static constexpr SCT OP_NOP { 0x0000'0000 };
static constexpr SCT OP_STORE { 0xD000'0000 };
static constexpr SCT OP_LOAD { 0xB000'0000 };
static constexpr SCT OP_STREAM { 0x9000'0000 };
static constexpr SCT OP_COMMIT_TO_STREAM { 0x6000'0000 };
static constexpr SCT OP_COMMIT_TO_MEMORY { 0xE000'0000 };

static constexpr std::size_t DEVICE_MIN_TRANSFER_SIZE { 512 }; // The DSE only supports transfers as a multiple of this size and 4 byte aligned.

struct Transfer
{
    uint32_t addr;
    uint32_t size;
};

} // namespace DSEC

} // namespace rr

#endif // _DMA_STREAM_ENGINE_COMMANDS_HPP_
