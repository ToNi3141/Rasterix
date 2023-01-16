
- [Design](#design)
  - [Software Flow](#software-flow)
  - [FPGA Flow](#fpga-flow)

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
