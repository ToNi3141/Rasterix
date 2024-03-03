
- [About this Project](#about-this-project)
  - [Area Usage](#area-usage)
- [Working Games](#working-games)
- [Checkout Repository](#checkout-repository)
- [Platforms](#platforms)
- [Port to a new platform](#port-to-a-new-platform)
  - [Port the driver](#port-the-driver)
  - [Port the FPGA implementation](#port-the-fpga-implementation)
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

# Port to a new platform 
Please have a look at `lib/driver`. There are already a few implementations to get inspired.

## Port the driver
To port the driver to a new interface (like SPI, async FT245, or others) use the following steps:
1. Create a new class which is derived from the `IBusConnector`. Implement the virtual methods.
2. Instantiate and use this class for the `Renderer`. If you use a MCU, you might lack `std::future`, then you can use `RendererMemoryOptimized`.
3. Create a `RenderConfig` which fits to the configuration of your hardware.
4. Add the whole `lib/gl`, `lib/3rdParty` and `lib/driver` directory to your build system.
5. Build

See also the example [here](/example/util/native/Runner.hpp).

The `Renderer` needs a `RenderConfig`. It contains the following information about the hardware:

| Property                  | Description |
|---------------------------|-------------|
| TMU_COUNT                 | Number of TMUs the hardware supports. Must be equal to the FPGA configuration. |
| MAX_TEXTURE_SIZE          | The maximum texture resolution the hardware supports. A valid values is 256 for 256x256px textures. Make sure that the texture fits in TEXTURE_BUFFER_SIZE of the FPGA. |
| ENABLE_MIPMAPPING         | Set this to `true` when mip mapping is available. Must be equal to the FPGA configuration |
| MAX_DISPLAY_WIDTH         | The maximum width if the screen. All integers are valid like 1024. To be most memory efficient, this should fit to your display resolution. |
| MAX_DISPLAY_HEIGHT        | The maximum height of the screen. All integers are valid like 600. To be most memory efficient, this should fit to your display resolution. |
| FRAMEBUFFER_SIZE_IN_WORDS | The size of the framebuffer in bytes. For the `rrxef` variant, use a value which fits at least the whole screen like 1024 * 600 * 2. For the `rrxif` variant, use the configuration size of the frame buffer. A valid value could be 65536 words. A word is the size of a pixel. Must be equal to the FPGA configuration. |
| USE_FLOAT_INTERPOLATION   | If `true`, it uploads triangle parameters in floating point format. If `false`, it uploads triangle parameters in fixed point format. Must be equal to the FPGA configuration. |
| CMD_STREAM_WIDTH          | Width of the command stream. Must be equal to the FPGA configuration. |
| NUMBER_OF_TEXTURE_PAGES   | The number of texture pages available. Combined with TEXTURE_PAGE_SIZE, it describes the size of the texture memory on the FPGA. This must never exceed the FPGAs available memory. |
| NUMBER_OF_TEXTURES        | Number of allowed textures. Lower value here can reduce the CPU utilization. Typically set this to the same value as NUMBER_OF_TEXTURE_PAGES. |
| TEXTURE_PAGE_SIZE         | The size of a texture page in bytes. Typical value is 4096. |
| GRAM_MEMORY_LOC           | Offset for the memory location. Typically this value is 0. Can be different when the memory is shared with other hardware, like in the Zynq platform. |
| FRAMEBUFFER_TYPE          | Configures the destination of the framebuffer. Must fit to the chosen variant. `FramebufferType::INTERNAL_*` is used for the `rrxif`, `FramebufferType::EXTERNAL_*` is used for `rrxef` |
| COLOR_BUFFER_LOC_1        | Location of the first framebuffer when FramebufferType::EXTERNAL_* is used and the destination when FramebufferType::INTERNAL_TO_MEMORY is used. |
| COLOR_BUFFER_LOC_2        | Second framebuffer when `FramebufferType::EXTERNAL_MEMORY_DOUBLE_BUFFER` is used. |
| DEPTH_BUFFER_LOC          | Depth buffer location when `FramebufferType::EXTERNAL_*` is used. |
| STENCIL_BUFFER_LOC        | Stencil buffer location when `FramebufferType::EXTERNAL_*` is used. |


## Port the FPGA implementation
Please use `rtl/top/Verilator/topMemory.v` as an simple example. Or have a look at the build script and the block diagram from the Nexys Video in `rtl/top/Xilinx/NexysVideo` to have a real world example.
1. Add the following directories to your project: `rtl/Rasterix`, `rtl/Util`, and `rtl/Float`.
2. Instantiate the `Rasterix` module. When configuring the module, make sure that the following parameters are equal to the template parameter from `Renderer.hpp`:
   1. Write a RenderConfig specifically for your design. See `RenderConfigs.hpp` for examples and documentation.  
3. Connect the `s_cmd_axis` interface to your command stream (this is the output from the `IBusConnector`).
4. Connect the `m_mem_axi` interface to a memory. Make sure to adapt the template parameter from the `Renderer.hpp` `Renderer::MAX_NUMBER_OF_TEXTURE_PAGES` to the size of the connected memory. One page has 4 kB. If you have a connected memory with 512kB, you should set `Renderer::MAX_NUMBER_OF_TEXTURE_PAGES` to 128.
5. Connect `m_framebuffer_axis` to an device, which can handle the color buffer stream (a framebuffer or a display for instance).
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
