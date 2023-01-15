
- [Rasterix](#rasterix)
- [Working games](#working-games)
- [Nexys Video Build](#nexys-video-build)
  - [PC Connection](#pc-connection)
- [Simulation Build](#simulation-build)
- [Unit-Tests](#unit-tests)
- [DLL Build](#dll-build)
  - [OS X Build](#os-x-build)
  - [Windows Build](#windows-build)
    - [Run Warcraft 3](#run-warcraft-3)
- [Design](#design)
  - [Software Flow](#software-flow)
  - [FPGA Flow](#fpga-flow)
- [Port to a new platform](#port-to-a-new-platform)
  - [Port the driver](#port-the-driver)
  - [Port the FPGA implementation](#port-the-fpga-implementation)
- [Missing features](#missing-features)
- [Next Steps:](#next-steps)

# Rasterix
The Rasterix is a rasterizer implementation for FPGAs written in Verilog. It implements a mostly OpenGL 1.3 compatible fixed function pixel pipeline with a maximum of two TMUs and register combiners in hardware. The vertex pipeline is implemented in software.

The current implementations lacks several important features like setting the screen resolution via registers, Mip Mapping and so on. It has more the nature of an prototype.

# Working games
Tested games are tuxracer 
![race screenshot](screenshots/tuxracerRaceStart.png)
and under Windows Warcraft 3 (with WGL).

# Nexys Video Build
The build target is a Nexys Video board with an `XC7A200` FPGA. The interface used to connect the FPGA with the PC is an 16 bit synchronous FT245 protocol on the Nexys FMC connector.
```
cd rtl/top/Xilinx/NexysVideo
/Xilinx/Vivado/2020.1/bin/vivado -mode batch -source build.tcl
```
You will find `rasterix.bin` and `rasterix.bit` in the synth directory. Use Vivado to program the FPGA or to flash the binary into the flash.

## PC Connection
The current implementation uses a `UMFT600X-B` eval board from FTDI with an FT600 to connect the Nexys with an PC. It offers a USB 3.0 connection and can be connected via the FMC connector to the Nexys.
 
The `UMFT600X-B` must be prepared:
- JP1: 1-2 (VBUS)
- JP2: 2-3 (Select powered by FIFO master Board(default))
- JP3: 1-2 (VCCIO=2.5V(default))
- JP4: Open (Multi-Channel FIFO)
- JP5: Open (Multi-Channel FIFO)
- JP6: Open

Also the following solder bridges must be applied:
- R26: Short (GPIO: Triggers the reset line)
- R27: Short (GPIO)

![ft600 picture](pictures/FT600.png)

# Simulation Build
One can find in `unittests/qtRasterizer` a Qt project. This can be used to simulate and debug the Rasterix and to get a live image of the output. There is also a `TARGET_BUILD` which targets synthesized rasterizer in the FPGA. 
Before the simulation can be build, the library for the simulation has to be build. This can be done with Verilator 4.036 2020-06-06 rev v4.034-208-g04c0fc8aa. Use the following code:
```
cd rtl/top/Verilator
make
```
After that, the simulation can be build an executed. You should see the following image on your screen:

![qtRasterizer screenshot](screenshots/qtRasterizer.png)

The image on the FPGA must be equal. You see here a cube with enabled multitexturing and lighting. Below you can see RGB colors testing the scissor.

It is likely, that your verialtor installation has another path as it is configured in the `qtRasterizer.pro` file. Let the variable  `VERILATOR_PATH` point to your verilator installation and rebuild the project.

Note: Currently the build is only tested on OS X. The .pro file must be adapted for other operating systems.

# Unit-Tests 
Unit-tests for the FPGA can be found under `./unittests`.

Just type `make` in the unit-tests directory. It will run all available tests.

# DLL Build
To run games like Warcraft 3 a DLL is required to forward the OpenGL calls to the FPGA. This repository contains a cmake project. The easiest way to configure is to use `cmake-gui` and set your target (for Windows probably a Visual Studio project, under OS X or Linux a makefile project).

Note: Only WGL is partially supported, therefor only Windows games which using WGL will probably run. Linux and OS X games won't run currently.

Lets assume, the project is generated in `./build`. 

## OS X Build
To build under Linux or OS X, change to the `build` directory and type `make`. To test the build, run `./example/minimal/minimal`. It should initialize the Renderer and should draw a rotating cube.

Note: The Linux build probably not succeed, because the FTDI library for linux is missing in the project.

## Windows Build
To build the project under Windows, go to the `build` directory, open the Visual Studio project and build it. Afterwards you will find a `wgl.dll`. The DLL is build for 32 bit targets because usually games from that area are 32 bit builds.

### Run Warcraft 3 
Only classic Warcraft 3 will run. Reforged does not. 
- Prepare Warcraft 3. Set the resolution to something like 800x600 or below and set the texture quality to low (currently the Renderer supports only textures with a maximum size of 256x256).
- Rename `wgl.dll` into `OpenGL32.dll` and copy it together with the 32 bit version of the `FTD3XX.dll` in the root directory of Warcraft 3. 
- Add a shortcut to the `war3.exe` on your desktop. Edit the shortcut and write `-opengl` after the path to the exe to start Warcraft 3 in OpenGL mode.
- Open Warcraft 3 via the shortcut. You should now see the menu on the screen connected to the FPGA.

Warcraft 3 runs on low settings with around 20-30 FPS.

# Design
For the s_cmd_axis command specification, please refer ```rtl/Rasterix/RegisterAndDescriptorDefines.vh```
## Software Flow
The following diagram shows roughly the flow a triangle takes, until it is seen on the screen.
![software flow diagram](pictures/softwareFlow.drawio.png)

The driver is build with the following components:
- `Application`: The application is the user of the library to draw 3D images.
- `IceGL`: Main entrypoint of the library. Creates and initializes all necessary classes for the library.
- `VertexPipeline`: Implements the geometry transformation, clipping and lighting.
- `PixelPipeline`: Controls the pixel pipeline of the hardware.
- `Renderer`: Implements the IRenderer interface, executes the rasterization, compiles display lists and sends them via the `IBusConnecter` to the Rasterix.
- `Rasterizer`: This basically is the rasterizer. It implements the edge equation to calculate barycentric coordinates and also calculates increments which are later used in the hardware to rasterize the triangle. This is also done for texture coordinates and w.
- `DisplayList`: Contains all render commands produced from the Renderer and buffers them, before they are streamed to the Rasterix.
- `TextureMemoryManager`: Manager for the texture memory on the device.
- `BusConnector`: This is an interface from the driver to the renderer. It is used to transfer the data via the defined interface like USB.
## FPGA Flow
![fpga flow diagram](pictures/fpgaFlow.drawio.png)
- `ftdi_245fifo` (3rd party): Implements the ft245 interface.
- `DmaStreamEngine`: DMA engine to write data into the RAM, stream data from the RAM to the renderer or pass through the stream from the FTDI to the renderer.
- `Rasterix`: This is the graphic core. It has an CMD_AXIS port where it receives the commands to render triangles, set render modes, upload textures and so on. It also has an FRAMEBUFFER_AXIS port where it streams out the data from the color buffer. Alternatively both AXIS ports can also be connected to other devices like DMAs, if you want to integrate the renderer in your own project.
- `CommandParser`: Reads the data from the CMD_AXIS port, decodes the commands and controls the renderer.
- `Rasterizer`: Takes the triangle parameters from the `Rasterizer` class (see the section in the Software) and rasterizes the triangle by using the precalculated values/increments.
- `AttributeInterpolator`: Interpolates the triangles attributes like color, textures and depth.
- `PixelPipeline`: The pipeline which takes the interpolated values and calculates the fragment color (blending, fogging, texturing, ...) and does the depth test.
- `FragmentPipeline`: Consumes the fragments from the Rasterizer, does perspective correction, depth test, blend and texenv calculations, texture clamping and so on.
- `TextureMappingUnit`: The texture mapping unit to texture the fragment. 
  - `TextureSampler`: Samples a texture from the `TextureBuffer`.
  - `TextureFilter`: Filters the texel from the `TextureSampler`.
  - `TexEnv`: Calculates the texture environment.
- `Fog`: Calculates the fog color of the fragment.
- `PerFragmentPipeline`: Calculates the per fragment operations like color blending and alpha / depth tests.
  - `ColorBlender`: Implements the color blending modes.
  - `TestFunc`: Implements the test functions for the alpha and depth test.
- `TextureBuffer`: Buffer which contains the complete texture.
- `FrameBuffer`: Contains the depth and color buffer.

# Port to a new platform 
Please have a look at the minimal example at `example/minimal/main.cpp`.
## Port the driver
To port the driver to connect to a new interface (like SPI, async FT245, or others)
1. Create a new class which derived from the `IBusConnector`. Implement implement the virtual methods. This is also performance critical. Use (if possible) non blocking methods. Otherwise the rendering is slowed down because the data transfer blocks further processing.
2. Instantiate and use this class for the `Renderer`.
3. Add the whole `lib/gl` directory to your build system.
4. Build

## Port the FPGA implementation
Please use the `topMemory.v` in the Verilator directory as an simple example. Or have a look at the build script and the block diagram from the `NexysVideo` to have a real world example.
1. Instantiate the `Rasterix` and a `DmaStreamEngine` in your project. When instantiating, the parameter values make sure that the template parameter `Renderer::CMD_STREAM_WIDTH` and the parameter `DmaStreamEngine::STREAM_WIDTH` and `Rasterix::CMD_STREAM_WIDTH` are equal. Make sure that template parameter `Renderer::Y_LINE_RESOLUTION` and parameter `Rasterix::Y_LINE_RESOLUTION` are equal.
2. Connect the command AXIS channel (`s_cmd_axis`) from the `Rasterix` to the `m_cmd_axis` channel of the `DmaStreamEngine`.
3. Connect the `s_cmd_axis` channel from the `DmaStreamEngine` to your device, which streams the command stream (basically the output from the `IBusConnector`).
4. Connect the `m_mem_axi` from the `DmaStreamEngine` to a memory. Make sure to adapt the template parameter `Renderer::MAX_NUMBER_OF_TEXTURE_PAGES` to the size of the connected memory. One page has 4 kB.
5. Connect the AXIS channel from the `Rasterix` color buffer (`m_framebuffer_axis`) to an device, which can handle the color buffer stream (a framebuffer for instance).
6. Connect `nreset` to your reset line and `aclk` to your clock domain.
7. Add everything in `rtl/Rasterix` and `rtl/Float` to your build system.
8. Synthesize.
# Missing features
The following features are currently missing compared to a real OpenGL implementation
- Logic Ops
- Mip Mapping
- ...

# Next Steps:
- Implement dynamic screen resolutions
- Implement Mip Mapping
- Implement logic ops
- Implement a texture cache to omit the `TextureBuffer`
- Implement higher texture resolutions
- Port to an Artix XC7A35
- Port to an Zynq board (XC7Z020 for now)
- Make driver usable with microcontrollers
- ...
