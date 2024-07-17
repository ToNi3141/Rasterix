
- [About this Project](#about-this-project)
  - [Area Usage](#area-usage)
- [Working Games](#working-games)
- [Checkout Repository](#checkout-repository)
- [Platforms](#platforms)
- [How to integrate](#how-to-integrate)
  - [How to port the Driver](#how-to-port-the-driver)
  - [How to use the Core](#how-to-use-the-core)
- [Missing Features](#missing-features)

# About this Project
The Rasterix project is a rasterizer implementation for FPGAs written in Verilog. It implements a mostly OpenGL 1.3 compatible fixed function pixel pipeline with a maximum of two TMUs and register combiners in hardware. The vertex pipeline is implemented in software.
The renderer is able to produce __100MPixel__ and __200MTexel__ at a clockspeed of 100MHz.

The long term goal of this project is to recreate an open source fixed function renderer compatible with all fixed function APIs (OpenGL 1.5, OpenGL ES 1.1, Glide, Direct 3D 7.0) and is also suitable for embedded devices. The current focus is on OpenGL.

It comes in two variants, `RasterixIF` and `RasterixEF`. `IF` stands for internal framebuffer while `EF` stands for external framebuffer. Both variants have their advantages and drawbacks. But except of the framebuffer handling and resulting limitations (because of AXI strobe limitations: stencil is applied on the whole value, not per bit, color mask work only on the whole fragment, no alpha channel), they are completely equal.

`RasterixIF`: This variant is usually faster, because it only loosely depends on the memory subsystem of your FPGA since the rendering is completely executed in static RAM resources on your FPGA. But the drawback is (you might already guess) the occupation of a lot of RAM resources on your FPGA.
For a reasonable performance, you need at least 128kB + 128kB + 32kB = 288kB memory only for the framebuffers. Less is possible but only useful for smaller displays. More memory is generally recommended. 

The used memory is decoupled from the actual framebuffer size. If a framebuffer with a specific resolution won't fit into the internal framebuffer, then the framebuffer is rendered in several cycles where the internal framebuffer only contains a part of the whole framebuffer.

`RasterixEF`: The performance of this variant heavily depends on the performance of your memory subsystem since all framebuffers are on your system memory (typically DRAM). While the latency is not really important for the performance but the the number of memory request the system can execute, is even more. This is especially in the Xilinx MIG a big bottleneck for this design (because of this, it is around two times slower that the `RasterixIF`). Contrary to the `RasterixIF`, it don't uses FPGA memory resources for the framebuffers. They are free for other designs, but it needs a bit more additional logic to handle the memory requests.

Both variants can work either in fixed point or floating point arithmetic. The fixed point arithmetic has almost the same image quality and compatibility compared to the float arithmetic. All tested games are working perfectly fine with both, while the fixed point configuration only requires a fraction of the logic of the floating point configuration. To start, it might be reasonable to try the floating point version first to bring everything up and to avoid pitfalls. Later, when there is the need to optimize, try the fixed point version.

## Area Usage
With a generic configuration: 
  - 64 bit command bus / memory bus
  - 256px textures
  - 1 TMU
  - mip mapping
  - fix point interpolation with 25 bit multipliers
  - internal framebuffer (rrxif)

Then the core requires __around 10k LUTs__ on a Xilinx Series 7 device.

A full configuration 
  - 128 bit command / memory bus
  - 256px textures
  - 2 TMUs
  - mip mapping
  - float interpolation with 32 bit floats
  - external frame buffer (rrxef)

Then the core requires __around 36k LUTs__ on a Xilinx Series 7 device.

# Working Games
Tested games are tuxracer (please see https://github.com/ToNi3141/tuxracer.git and the Branch `RasterixPort`), Quake 3 Arena with SDL2 and glX (https://github.com/ToNi3141/Quake3e), Warcraft 3 with WGL and others.

![race screenshot](screenshots/tuxracerRaceStart.png)

# Checkout Repository
Use the following commands to checkout the repository:

```sh
git clone https://github.com/ToNi3141/Rasterix.git
cd Rasterix
git submodule init
git submodule update
```

# Platforms
The rasterizer is running on the following platforms:

- [Verilator Simulation](/rtl/top/Verilator/README.md)
- [Digilent Nexys Video](/rtl/top/Xilinx/NexysVideo/README.md)
- [Digilent CMod7](/rtl/top/Xilinx/CmodA7/README.md)
- [Digilent ArtyZ7-20](/rtl/top/Xilinx/ArtyZ7-20/README.md)

# How to integrate
To integrate it into your own project, first have a look at the already existing platforms. If you want to integrate it in a already existing SoC system, you may have a look at the ArtyZ7. If you want to use it as standalone, have a look at the Nexys Video or CMod7. It may be likely, that you can use the already existing code.

## How to port the Driver
To port the driver to a new interface (like SPI, async FT245, AXIS, or others) use the following steps:
1. Create a new class which is derived from the `IBusConnector`. Implement the virtual methods.
2. Set the build variables mentioned below in the table.
3. Add the whole `lib/gl`, `lib/3rdParty` and `lib/driver` directory to your build system. Or add this repository to your CMake project and include the library by adding `gl`.
4. Build

See also the example [here](/example/util/native/Runner.hpp).

The build system requires the following parameters to be set, and which shall be equal to the parameters set in the hardware.

| Property                           | Description |
|------------------------------------|-------------|
| RRX_CORE_TMU_COUNT                 | Number of TMUs the hardware supports. Must be equal to the FPGA configuration. |
| RRX_CORE_MAX_TEXTURE_SIZE          | The maximum texture resolution the hardware supports. A valid values is 256 for 256x256px textures. Make sure that the texture fits in TEXTURE_BUFFER_SIZE of the FPGA. |
| RRX_CORE_ENABLE_MIPMAPPING         | Set this to `true` when mip mapping is available. Must be equal to the FPGA configuration |
| RRX_CORE_MAX_DISPLAY_WIDTH         | The maximum width if the screen. All integers are valid like 1024. To be most memory efficient, this should fit to your display resolution. |
| RRX_CORE_MAX_DISPLAY_HEIGHT        | The maximum height of the screen. All integers are valid like 600. To be most memory efficient, this should fit to your display resolution. |
| RRX_CORE_FRAMEBUFFER_SIZE_IN_WORDS | The size of the framebuffer in bytes. For the `rrxef` variant, use a value which fits at least the whole screen like 1024 * 600 * 2. For the `rrxif` variant, use the configuration size of the frame buffer. A valid value could be 65536 words. A word is the size of a pixel. Must be equal to the FPGA configuration. |
| RRX_CORE_USE_FLOAT_INTERPOLATION   | If `true`, it uploads triangle parameters in floating point format. If `false`, it uploads triangle parameters in fixed point format. Must be equal to the FPGA configuration. |
| RRX_CORE_CMD_STREAM_WIDTH          | Width of the command stream. Must be equal to the FPGA configuration. |
| RRX_CORE_NUMBER_OF_TEXTURE_PAGES   | The number of texture pages available. Combined with TEXTURE_PAGE_SIZE, it describes the size of the texture memory on the FPGA. This must never exceed the FPGAs available memory. |
| RRX_CORE_NUMBER_OF_TEXTURES        | Number of allowed textures. Lower value here can reduce the CPU utilization. Typically set this to the same value as NUMBER_OF_TEXTURE_PAGES. |
| RRX_CORE_TEXTURE_PAGE_SIZE         | The size of a texture page in bytes. Typical value is 4096. |
| RRX_CORE_GRAM_MEMORY_LOC           | Offset for the memory location. Typically this value is 0. Can be different when the memory is shared with other hardware, like in the Zynq platform. |
| RRX_CORE_FRAMEBUFFER_TYPE          | Configures the destination of the framebuffer. Must fit to the chosen variant. `FramebufferType::INTERNAL_*` is used for the `rrxif`, `FramebufferType::EXTERNAL_*` is used for `rrxef` |
| RRX_CORE_COLOR_BUFFER_LOC_1        | Location of the first framebuffer when FramebufferType::EXTERNAL_* is used and the destination when FramebufferType::INTERNAL_TO_MEMORY is used. |
| RRX_CORE_COLOR_BUFFER_LOC_2        | Second framebuffer when `FramebufferType::EXTERNAL_MEMORY_DOUBLE_BUFFER` is used. |
| RRX_CORE_DEPTH_BUFFER_LOC          | Depth buffer location when `FramebufferType::EXTERNAL_*` is used. |
| RRX_CORE_STENCIL_BUFFER_LOC        | Stencil buffer location when `FramebufferType::EXTERNAL_*` is used. |


## How to use the Core
1. Add the following directories to your project: `rtl/Rasterix`, `rtl/Util`, and `rtl/Float`.
2. Instantiate the `RasterixIF` or `RasterixEF` module.  
3. Connect the `s_cmd_axis` interface to your command stream (this is the output from the `IBusConnector`).
4. Connect the `m_mem_axi` interface to a memory. In case of a configuration with external framebuffer, connect also the axi ports of the framebuffer to the memory.
5. Optionally connect `m_framebuffer_axis` to an device, which can handle the color buffer stream (a display for instance). When using a memory mapped framebuffer, then this port is unused.
6. Connect `resetn` to your reset line and `aclk` to your clock domain.
7. Synthesize.

The hardware has the following configuration options:

| Property                                | Description |
|-----------------------------------------|-------------|
| FRAMEBUFFER_SIZE_IN_WORDS               | The size of the internal framebuffer (in power of two). <br> Depth buffer word size: 16 bit. <br> Color buffer word size: FRAMEBUFFER_SUB_PIXEL_WIDTH * (FRAMEBUFFER_ENABLE_ALPHA_CHANNEL ? 4 : 3). |
| FB_MEM_DATA_WIDTH                       | `rrxef` only. Width of the framebuffer memory channel. |
| FRAMEBUFFER_SUB_PIXEL_WIDTH             | `rrxif` only. Sub pixel width in the internal framebuffer. |
| FRAMEBUFFER_ENABLE_ALPHA_CHANNEL        | `rrxif` only. Enables the alpha channel in the framebuffer. |
| ENABLE_STENCIL_BUFFER                   | Enables the stencil buffer. |
| TMU_COUNT                               | Number of TMU the hardware shall contain. Valid values are 1 and 2. |
| ENABLE_MIPMAPPING                       | Enables the mip mapping. |
| CMD_STREAM_WIDTH                        | Width of the AXIS command stream. |
| TEXTURE_BUFFER_SIZE                     | Size of the texture buffer in lg2(bytes). |
| ADDR_WIDTH                              | Width of the AXI address channel. |
| ID_WIDTH                                | Width of the AXI id property. |
| STRB_WIDTH                              | Width of the AXI strobe property. |
| FB_MEM_STRB_WIDTH                       | `rrxef` only. Width of the framebuffer AXI strobe property. |
| RASTERIZER_FLOAT_PRECISION              | `true` enables the floating point interpolation. `false` enables the fixed point interpolation. |
| RASTERIZER_FIXPOINT_PRECISION           | Defines the width of the multipliers used in the fixed point interpolation. Valid range: 16-25. |
| RASTERIZER_ENABLE_FLOAT_INTERPOLATION   | Precision of the floating point arithmetic. Valid range: 20-32. |


# Missing Features
The following features are currently missing compared to a real OpenGL implementation
- Logic Ops
- ...
