
- [Design](#design)
  - [Software Flow](#software-flow)
  - [FPGA Flow](#fpga-flow)

# Design
For the s_cmd_axis command specification, please refer `rtl/Rasterix/RegisterAndDescriptorDefines.vh`.

The renderer uses a stream centric design. That means, the renderer does not query any data from RAM or other sources. It expects a stream of data which contains all needed data to render a triangle, like texture data, configurations, triangle parameters and so on. For that reason, the renderer has a few internal buffers:
- Texture Buffer: A buffer which holds a complete texture. Typically this is 128kB in size to contain a full 256x256x16 texture. Each TMU has their own texture buffer.
- Frame Buffer: The frame buffer used for rendering. There is a configurable 16bit color 16bit depth buffer.

When the rendering of the image is finished, the renderer will stream out the content from the internal frame buffer.

Normally the internal frame buffer in the renderer is too small to render a complete high resolution picture. To overcome this limitation, the renderer can render partial images. For instance, assume a screen resolution of 1024x768x16 (1536kB) and a internal frame buffer with the size of 256kB. The driver will then divide the image in 6 different 1024x128x16 parts. It will first render the image {(0, 0), (1023, 127)}, then {(0, 128), (1023, 255)} and so on. It will stream each subpart to the frame buffer in memory. This frame buffer is then responsible to stich the sub images together.

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
- `Rasterix`: This is the main core. This module works as stand alone and is used to integrate into block designs or custom FPGA SoCs.
  - `DmaStreamEngine`: DMA engine to write data into the RAM, stream data from the RAM to the renderer or pass through the stream from the FTDI to the renderer.
  - `RasterixRenderCore`: This is the top module of the renderer. It contains all necessary modules to produce images.
    - `CommandParser`: Reads the data from the CMD_AXIS port, decodes the commands and controls the renderer.
    - `Rasterizer`: Takes the triangle parameters from the `Rasterizer` class (see the section in the Software) and rasterizes the triangle by using the precalculated values/increments.
    - `AttributeInterpolator`: Interpolates the triangles attributes like color, textures and depth. Also applies the perspective correction to the textures.
    - `PixelPipeline`: Consumes the fragments from the `AttributeInterpolator`, and does the depth test, alpha test, blending and texenv calculations, texture clamping and so on.
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
