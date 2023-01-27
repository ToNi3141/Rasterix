
- [Rasterix](#rasterix)
- [Working Games](#working-games)
- [Checkout Repository](#checkout-repository)
- [Nexys Video Build](#nexys-video-build)
  - [PC Connection](#pc-connection)
- [Simulation Build](#simulation-build)
- [Unit-Tests](#unit-tests)
- [DLL Build](#dll-build)
  - [OS X Build](#os-x-build)
  - [Windows Build](#windows-build)
    - [Run Warcraft 3](#run-warcraft-3)
- [Port to a new platform](#port-to-a-new-platform)
  - [Port the driver](#port-the-driver)
  - [Port the FPGA implementation](#port-the-fpga-implementation)
- [Missing Features](#missing-features)
- [Next Steps](#next-steps)

# Rasterix
Rasterix is a rasterizer implementation for FPGAs written in Verilog. It implements a mostly OpenGL 1.3 compatible fixed function pixel pipeline with a maximum of two TMUs and register combiners in hardware. The vertex pipeline is implemented in software.
The renderer is currently able to produce __100MPixel__ and __200MTexel__ at a clockspeed of 100MHz.

The current implementations lacks several important features like setting the screen resolution via registers, Mip Mapping and so on. It has more the nature of an prototype.

# Working Games
Tested games are tuxracer (please see https://github.com/ToNi3141/tuxracer.git and the Branch `RasterixPort`)

![race screenshot](screenshots/tuxracerRaceStart.png)

and under Windows Warcraft 3 (with WGL).

# Checkout Repository
Use the following commands to checkout the repository:

```
git clone https://github.com/ToNi3141/Rasterix.git
cd Rasterix
git submodule init
git submodule update
```

# Nexys Video Build
The build target is a Nexys Video board with an `XC7A200` FPGA. The interface used to connect the FPGA with the PC is an 16bit synchronous FT245 protocol on the Nexys FMC connector.

To build the binaries use the following commands.
```
cd rtl/top/Xilinx/NexysVideo
/Xilinx/Vivado/2020.1/bin/vivado -mode batch -source build.tcl
```
You will find `rasterix.bin` and `rasterix.bit` in the synth directory. Use Vivado to program the FPGA or to flash the binary into the flash.

## PC Connection
The current implementation uses a `UMFT600X-B` eval board from FTDI with an FT600 to connect the Nexys with an PC. It offers a USB 3.0 connection and can be connected via the FMC connector.
 
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
A simulation can be used to easily develop and debug the renderer. The simulation can be found under `unittests/qtRasterizer`. There is a Qt project which can be opened with the QtCreator. This project supports also the real hardware, and can be selected with the `TARGET_BUILD` variable in the .pro file.

First generate the C++ code from the Verilog source via Verilator 4.036 2020-06-06 rev v4.034-208-g04c0fc8aa. Use the following commands:
```
cd rtl/top/Verilator
make
```
Then build the Qt project. If the build was successful, you will see the following image on the screen, when you have started the application:

![qtRasterizer screenshot](screenshots/qtRasterizer.png)

You see here a cube with enabled multi texturing and lighting. Below you can see RGB colors testing the scissor.

It is likely, that your verialtor installation has another path than it is configured in the `qtRasterizer.pro` file. Let the variable  `VERILATOR_PATH` point to your verilator installation and rebuild the project.

Note: Currently the build is only tested on OS X. The .pro file must be adapted for other operating systems.

# Unit-Tests 
Unit-tests for the Verilog code can be found under `./unittests`.

Just type `make` in the unit-tests directory. It will run all available tests.

# DLL Build
To run games like Warcraft 3 a DLL is required to forward the OpenGL calls to the FPGA. This repository contains a cmake project to build the DLL.

Note: Only WGL is partially supported, therefore only Windows games which using WGL will probably run. Linux and OS X games won't run currently.

## OS X Build
Before configuring and starting the build, download from FTDI (https://ftdichip.com/drivers/d3xx-drivers/) the 64bit X64 D3XX driver version 0.5.21. Unzip the archive and copy the `osx` directory to `lib/driver/ft60x/ftd3xx/`.

Open `cmake-gui` to create a makefile project. Use as build directory `./build`.

Switch to the `build` directory and type `make` to build the project. To test the build, run `./example/minimal/minimal`. It should initialize the renderer and draw a rotating cube.

## Windows Build
Before starting the build, download from FTDI (https://ftdichip.com/drivers/d3xx-drivers/) the 32bit X86 D3XX driver version 1.3.0.4. Unzip the archive and copy the `win` directory to `lib/driver/ft60x/ftd3xx/`.

Open `cmake-gui` to create a Visual Studio project. Use as build directory `./build`.

Go to the `build` directory, open the Visual Studio project and build it. Afterwards you will find a `wgl.dll`. The DLL is build for 32bit targets because games from that era are usually 32bit builds. To test the build, run `.\example\minimal\Release\minimal.exe`. It should initialize the renderer and draw a rotating cube.


### Run Warcraft 3 
Only classic Warcraft 3 will work. Reforged does not. 
- Prepare Warcraft 3. Set the resolution to something like 800x600 or below and set the texture quality to low (currently the Renderer supports only textures with a maximum size of 256x256).
- Rename `wgl.dll` into `OpenGL32.dll` and copy it together with the 32bit version of the `FTD3XX.dll` in the root directory of your Warcraft 3 installation. 
- Add a shortcut to the `Frozen Throne.exe` or `Warcraft III.exe` on your desktop. Edit the shortcut and write `-opengl` after the path of the exe (`"C:\path\to\warcraft3\Frozen Throne.exe" -opengl`) to start Warcraft 3 in OpenGL mode.
- Open Warcraft 3 via the shortcut. You should now see the menu on the screen connected to the FPGA.

Warcraft 3 runs on low settings with around 20-30FPS.

Switching the resolution and videos are currently not working.

# Port to a new platform 
Please have a look at `lib/driver`. There are already a few implementations to get inspired.
## Port the driver
To port the driver to a new interface (like SPI, async FT245, or others) use the following steps:
1. Create a new class which is derived from the `IBusConnector`. Implement the virtual methods. This is also performance critical. Use (if possible) non blocking methods. Otherwise the rendering is slowed down because the data transfer blocks further processing.
2. Instantiate and use this class for the `Renderer`.
3. Add the whole `lib/gl`, `lib/3rdParty` and `lib/driver` directory to your build system.
4. Build

## Port the FPGA implementation
Please use `rtl/top/Verilator/topMemory.v` as an simple example. Or have a look at the build script and the block diagram from the Nexys Video in `rtl/top/Xilinx/NexysVideo` to have a real world example.
1. Add the following directories to your project: `rtl/Rasterix`, `rtl/Util`, and `rtl/Float`.
2. Instantiate the `Rasterix` module. When configuring the module, make sure that the following parameters are equal to the template parameter from `Renderer.hpp`:
   1. `Renderer::CMD_STREAM_WIDTH` and `Rasterix::CMD_STREAM_WIDTH`. 
   2. `Renderer::Y_LINE_RESOLUTION` and `Rasterix::Y_LINE_RESOLUTION`.
3. Connect the `s_cmd_axis` interface to your command stream (this is the output from the `IBusConnector`).
4. Connect the `m_mem_axi` interface to a memory. Make sure to adapt the template parameter from the `Renderer.hpp` `Renderer::MAX_NUMBER_OF_TEXTURE_PAGES` to the size of the connected memory. One page has 4 kB. If you have a connected memory with 512kB, you should set `Renderer::MAX_NUMBER_OF_TEXTURE_PAGES` to 128.
5. Connect `m_framebuffer_axis` to an device, which can handle the color buffer stream (a framebuffer or a display for instance).
6. Connect `resetn` to your reset line and `aclk` to your clock domain.
7. Synthesize.

# Missing Features
The following features are currently missing compared to a real OpenGL implementation
- Logic Ops
- Mip Mapping
- ...

# Next Steps
- Implement dynamic screen resolutions
- Add the possibility to use more than one render context
- Implement Mip Mapping
- Implement logic ops
- Implement a texture cache to omit the `TextureBuffer`
- Implement higher texture resolutions
- Port to an Artix XC7A35
- Port to an Zynq board (XC7Z020 for now)
- Make driver usable with microcontrollers
- ...
