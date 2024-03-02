
- [About this Project](#about-this-project)
  - [Area](#area)
- [Working Games](#working-games)
- [Checkout Repository](#checkout-repository)
- [Platforms](#platforms)
- [Port to a new platform](#port-to-a-new-platform)
  - [Port the driver](#port-the-driver)
  - [Port the FPGA implementation](#port-the-fpga-implementation)
- [Missing Features](#missing-features)
- [Next Possible Steps](#next-possible-steps)

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

## Area
With a normal configuration: 
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
Tested games are tuxracer (please see https://github.com/ToNi3141/tuxracer.git and the Branch `RasterixPort`), Quake 3 Arena with SDL2 and glX (https://github.com/ToNi3141/Quake3e) and others.

![race screenshot](screenshots/tuxracerRaceStart.png)

Warcraft 3 (with WGL) under windows is also working.

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
2. Instantiate and use this class for the `Renderer`. If you use a microcontroller, you might lack `std::future`, then you can use `RendererMemoryOptimized`.
3. Add the whole `lib/gl`, `lib/3rdParty` and `lib/driver` directory to your build system.
4. Build

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

# Missing Features
The following features are currently missing compared to a real OpenGL implementation
- Logic Ops
- ...

# Next Possible Steps
- Add the possibility to use more than one render context
- Implement Logic Ops
- Implement a texture cache to reduce the memory consuption by omitting the `TextureBuffer`
- Implement higher texture resolutions
- ...
