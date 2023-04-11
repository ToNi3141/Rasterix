
- [Rasterix](#rasterix)
- [Working Games](#working-games)
- [Checkout Repository](#checkout-repository)
- [Nexys Video Build](#nexys-video-build)
  - [Hardware Setup](#hardware-setup)
    - [UMFT600X-B Eval Kit Preparation](#umft600x-b-eval-kit-preparation)
- [CMOD A7 Build](#cmod-a7-build)
  - [Hardware Setup](#hardware-setup-1)
- [Simulation Build](#simulation-build)
- [Unit-Tests](#unit-tests)
- [OS X Build](#os-x-build)
- [Windows Build](#windows-build)
  - [Run Warcraft 3](#run-warcraft-3)
- [RP2040 Build](#rp2040-build)
- [Port to a new platform](#port-to-a-new-platform)
  - [Port the driver](#port-the-driver)
  - [Port the FPGA implementation](#port-the-fpga-implementation)
- [Missing Features](#missing-features)
- [Next Steps](#next-steps)

# Rasterix
Rasterix is a rasterizer implementation for FPGAs written in Verilog. It implements a mostly OpenGL 1.3 compatible fixed function pixel pipeline with a maximum of two TMUs and register combiners in hardware. The vertex pipeline is implemented in software.
The renderer is currently able to produce __100MPixel__ and __200MTexel__ at a clockspeed of 100MHz.

The current implementations lacks several important features like mip mapping, logic ops and so on. It has more the nature of an prototype.

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

This build uses two TMUs.

To build the binaries use the following commands.
```
cd rtl/top/Xilinx/NexysVideo
/Xilinx/Vivado/2020.1/bin/vivado -mode batch -source build.tcl
```
You will find `rasterix.bin` and `rasterix.bit` in the synth directory. Use Vivado to program the FPGA or to flash the binary into the flash.

## Hardware Setup
Connect the Nexys Video via USB 3.0 to your computer (via the `UMFT600X-B` eval board). Connect to your Nexys Video a 1024x600 px monitor. If you don't have a monitor at hand with this resolution, you have to change the resolution in the `rtl/Display/Dvi.v` wrapper and in the software (for instance in `example/minimal/main.cpp`).

<img src="screenshots/nexysvideo.jpg" width="40%"> 

### UMFT600X-B Eval Kit Preparation
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

<img src="pictures/FT600.png" width="450" height="400">

# CMOD A7 Build
The build target is a CMOD A7 board with an `XC7A35` FPGA. The interface used to connect the FPGA with a host is an SPI interface with additional CTS pin for flow control (in software).

This build uses only one TMU with a maximum texture resolution of 128x128px.

To build the binaries, use the following commands.
```
cd rtl/top/Xilinx/CmodA7
/Xilinx/Vivado/2020.1/bin/vivado -mode batch -source build.tcl
```
You will find `rasterix.bin` and `rasterix.bit` in the synth directory. Use Vivado to program the FPGA or to flash the binary into the flash.

## Hardware Setup
The following hardware setup shows an Raspberry Pi Pico connected to an CMOD A7 with an `XC7A35` FPGA connected and an 320x240 pixel display with an `ILI9341` chipset.

<img src="screenshots/cmod7.jpg" width="50%"> 

The Pico is connected via SPI to the CMOD. Have a look at the following table to connect them:
| Port Name | Pico | CMOD A7 |
|-----------|------|---------|
| MISO      | GP16 | 48      |
| CSN       | GP17 | 46      |
| SCK       | GP18 | 47      |
| MOSI      | GP19 | 45      |
| CTS       | GP20 | 44      |
| RSTN      | GP21 | 43      |

Supported SPI clock speed: Around 50MHz.

The display is directly connected to the FPGA via the 8080-I parallel interface. The FPGA automatically configures the display when reset is asserted. To connect the display use the following table:

| Port Name | CMOD A7 | ILI9341 |
|-----------|---------|---------|
| CS        | n/c     | GND     |
| C/D       | 12      | C/D     |
| WR        | 10      | WR      |
| RD        | 09      | RD      |
| RST       | n/c     | 3.3V    |
| DATA[7:0] | [1:8]   | D[7:0]  |


# Simulation Build
A simulation can be used to easily develop and debug the renderer. The simulation can be found under `example/qtDebug/qtRasterizerFpga`. There is a Qt project which can be opened with the QtCreator. This project supports also the real hardware, which can be selected with the `TARGET_BUILD` variable in the .pro file.

Before building the simulation, create the C++ code from the Verilog source via Verilator 4.036 2020-06-06 rev v4.034-208-g04c0fc8aa. Use the following commands:
```
cd rtl/top/Verilator
make
```
Then build the Qt project. If the build was successful, you will see the following image on the screen, when you have started the application:

![qtRasterizer screenshot](screenshots/qtRasterizer.png)

You see here a cube with enabled multi texturing and lighting. Below you can see RGB colors testing the scissor.

It is likely, that your verialtor installation has another path than it is configured in the `qtRasterizer.pro` file. Let the variable  `VERILATOR_PATH` point to your verilator installation and rebuild the project.

Note: Currently the build is only tested on OS X. The .pro file must be adapted for other operating systems.

There exists a second project `example/qtDebug/qtRasterizerGL`. This project uses the native OpenGL implementation of your machine. You can compare this output with the output of the simulation. The content must be equal.
# Unit-Tests 
Unit-tests for the Verilog code can be found under `./unittests`.

Just type `make` in the unit-tests directory. It will run all available tests.

# OS X Build
Before configuring and starting the build, download from FTDI (https://ftdichip.com/drivers/d3xx-drivers/) the 64bit X64 D3XX driver version 0.5.21. Unzip the archive and copy the `osx` directory to `lib/driver/ft60x/ftd3xx/`.

To build the library an the minimal example, switch to the source directory and type
```
cd <rasterix_directory>
cmake --preset native
cd build/native
make -j
```
To run the minimal example, type
```
./example/minimal/minimal
```
into your terminal. It should now show an image similar to the simulation.

# Windows Build
Before starting the build, download from FTDI (https://ftdichip.com/drivers/d3xx-drivers/) the 32bit X86 D3XX driver version 1.3.0.4. Unzip the archive and copy the `win` directory to `lib/driver/ft60x/ftd3xx/`.

Open a terminal. Use the following commands to create a 32bit Visual Studio Project:
```
cd <rasterix_directory>
cmake --preset win32
cd build/win32
```

Open the Visual Studio project in the `build/win32` directory and build it. Afterwards you will find a `wgl.dll`. The DLL is build for 32bit targets because games from that era are usually 32bit builds. To test the build, type
```
.\example\minimal\Release\minimal.exe
```
into your terminal. It should now show an image similar to the simulation.

## Run Warcraft 3 
Only classic Warcraft 3 will work. Reforged does not. 
- Prepare Warcraft 3. Set the resolution to something like 800x600 or below and set the texture quality to low (currently the Renderer supports only textures with a maximum size of 256x256).
- Rename `wgl.dll` into `OpenGL32.dll` and copy it together with the 32bit version of the `FTD3XX.dll` in the root directory of your Warcraft 3 installation. 
- Add a shortcut to the `Frozen Throne.exe` or `Warcraft III.exe` on your desktop. Edit the shortcut and write `-opengl` after the path of the exe (`"C:\path\to\warcraft3\Frozen Throne.exe" -opengl`) to start Warcraft 3 in OpenGL mode.
- Open Warcraft 3 via the shortcut. You should now see the menu on the screen connected to the FPGA.

Warcraft 3 runs on low settings with around 20-30FPS.

Switching the resolution and videos are currently not working.

# RP2040 Build
Before you start to build, have a look at the rp2040 SDK readme (https://github.com/raspberrypi/pico-sdk). You have several options, which are supported. The option documented there is based on a already cloned SDK on your computer.

Open a terminal. Use the following commands to build a rp2040 binary:
```
cd <rasterix_directory>
cmake --preset rp2040 -DPICO_SDK_PATH=<path_to_the_sdk>
cd build/rp2040
make -j
```
You will find a `minimal.uf2` file in the `build/rp2040/example/rp-pico` directory.

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
   1. `RenderConfig::CMD_STREAM_WIDTH` and `Rasterix::CMD_STREAM_WIDTH`. 
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
- Add the possibility to use more than one render context
- Implement Mip Mapping
- Implement logic ops
- Implement a texture cache to omit the `TextureBuffer`
- Implement higher texture resolutions
- Port to an Zynq board (XC7Z020 for now)
- ...
