// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2023 ToNi3141

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <stdint.h>
#include <array>
#include <optional>
#include "math/Vec.hpp"
#include "Renderer.hpp"
#include "IBusConnector.hpp"
#include "DisplayList.hpp"
#include "Rasterizer.hpp"
#include <string.h>
#include "DisplayListAssembler.hpp"
#include <algorithm>
#include "TextureMemoryManager.hpp"
#include <limits>

#include "registers/BaseColorReg.hpp"
#include "registers/TexEnvReg.hpp"
#include "registers/FragmentPipelineReg.hpp"
#include "registers/FeatureEnableReg.hpp"
#include "registers/TmuTextureReg.hpp"
#include "registers/ColorBufferClearColorReg.hpp"
#include "registers/DepthBufferClearDepthReg.hpp"
#include "registers/FogColorReg.hpp"
#include "registers/RenderResolutionReg.hpp"
#include "registers/ScissorEndReg.hpp"
#include "registers/ScissorStartReg.hpp"
#include "registers/TexEnvColorReg.hpp"
#include "registers/YOffsetReg.hpp"
#include "registers/StencilReg.hpp"
#include "registers/ColorBufferAddrReg.hpp"
#include "registers/DepthBufferAddrReg.hpp"
#include "registers/StencilBufferAddrReg.hpp"
#include "commands/TriangleStreamCmd.hpp"
#include "commands/FogLutStreamCmd.hpp"
#include "commands/FramebufferCmd.hpp"
#include "commands/TextureStreamCmd.hpp"
#include "commands/WriteRegisterCmd.hpp"
#include "RenderConfigs.hpp"

namespace rr
{
// Screen
// <-----------------X_RESOLUTION--------------------------->
// +--------------------------------------------------------+ ^
// |        ^                                               | |
// |        | m_yLineResolution      m_displayLines         | |
// |        |                                               | |
// |        v                                               | |
// |<------------------------------------------------------>| Y
// |                                                        | _
// |                                 m_displayLines         | R
// |                                                        | E
// |                                                        | S
// |<------------------------------------------------------>| O
// |                                                        | L
// |                                 m_displayLines         | U
// |                                                        | T
// |                                                        | I
// |<------------------------------------------------------>| O
// |                                                        | N
// |                                 m_displayLines         | |
// |                                                        | |
// |                                                        | |
// +--------------------------------------------------------+ v
// This renderer collects all triangles in a single display list. It will create for each display line a unique display list where
// all triangles and operations are stored, which belonging to this display line. This is probably the fastest method to do this
// but requires much more memory because of lots of duplicated data.
// The RenderConfig::CMD_STREAM_WIDTH is used to calculate the alignment in the display list.
class Renderer
{
    static constexpr std::size_t DISPLAY_LINES { ((RenderConfig::MAX_DISPLAY_WIDTH * RenderConfig::MAX_DISPLAY_HEIGHT) == RenderConfig::FRAMEBUFFER_SIZE_IN_WORDS) ? 1 
                                                                                                                                                                : ((RenderConfig::MAX_DISPLAY_WIDTH * RenderConfig::MAX_DISPLAY_HEIGHT) / RenderConfig::FRAMEBUFFER_SIZE_IN_WORDS) + 1 };
public:
    using TextureWrapMode = TmuTextureReg::TextureWrapMode;
    Renderer(IBusConnector& busConnector);

    ~Renderer();

    /// @brief Will render a triangle which is constructed with the given parameters
    /// @return true if the triangle was rendered, otherwise the display list was full and the triangle can't be added
    bool drawTriangle(const TransformedTriangle& triangle);

    /// @brief Starts the rendering process by uploading textures and the displaylist and also swapping
    /// the framebuffers
    void swapDisplayList();

    /// @brief Uploads the display list to the hardware
    void uploadDisplayList();

    /// @brief Will clear a buffer
    /// @param frameBuffer Will clear the frame buffer
    /// @param zBuffer Will clear the z buffer
    /// @param stencilBuffer Will clear the stencil buffer
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool clear(const bool colorBuffer, const bool depthBuffer, const bool stencilBuffer);

    /// @brief Will set a color which is used as clear color for the frame buffer
    /// when clear() is called
    /// @param color the color in ABGR
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setClearColor(const Vec4i& color);

    /// @brief Will set a depth which is used as initial value for the depth buffer when clear() is called
    /// @param depth The depth which is used by calling clear()
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setClearDepth(uint16_t depth);

    /// @brief Updates the fragment pipeline configuration 
    /// @param pipelineConf The new pipeline configuration 
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setFragmentPipelineConfig(const FragmentPipelineReg& pipelineConf) { return writeReg(pipelineConf); }

    /// @brief Configures the texture environment. See glTexEnv()
    /// @param pname parameter name of the env parameter
    /// @param param Function of the tex env
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setTexEnv(const TexEnvReg& texEnvConfig) { return writeReg(texEnvConfig); }

    /// @brief Set a static color for the tex environment
    /// @param target is used TMU
    /// @param color the color in ABGR
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setTexEnvColor(const std::size_t target, const Vec4i& color);

    /// @brief Set the fog color
    /// @param color the color in ABGR
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setFogColor(const Vec4i& color);

    /// @brief Sets the fog LUT. Note that the fog uses the w value as index (the distance from eye to the polygon znear..zfar)
    /// and not z (clamped value of 0.0..1.0)
    /// @param fogLut the fog lookup table
    /// The fog LUT uses a exponential distribution of w, means fogLut[0] = f(1), fogLut[1] = f(2), fogLut[2] = f(4), fogLut[3] = f(8).
    /// The fog values between start and end must not exceed 1.0f
    /// @param start the start value of the fog
    /// @param end the end value of the fog
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setFogLut(const std::array<float, 33>& fogLut, float start, float end);

    /// @brief Creates a new texture 
    /// @return pair with the first value to indicate if the operation succeeded (true) and the second value with the id
    std::pair<bool, uint16_t> createTexture() { return m_textureManager.createTexture(); }

    /// @brief Creates a new texture for a given texture id
    /// @param texId the name of the new texture
    /// @return pair with the first value to indicate if the operation succeeded (true) and the second value with the id
    bool createTextureWithName(const uint16_t texId) { return m_textureManager.createTextureWithName(texId); }
    
    /// @brief This will update the texture data of the texture with the given id
    /// @param texId The texture id which texture has to be updated
    /// @param textureObject The object which contains the texture and all its meta data
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool updateTexture(const uint16_t texId, const TextureObjectMipmap& textureObject) { return m_textureManager.updateTexture(texId, textureObject); }

    /// @brief Returns a texture associated to the texId
    /// @param texId The texture id of the texture to get the data from
    /// @return The texture object
    TextureObjectMipmap getTexture(const uint16_t texId) { return m_textureManager.getTexture(texId); }

    /// @brief Queries if the current texture id is a valid texture
    /// @param texId Texture id to query
    /// @return true if the current texture id is mapped to a valid texture
    bool isTextureValid(const uint16_t texId) const { return m_textureManager.textureValid(texId); }

    /// @brief Activates a texture which then is used for rendering
    /// @param target The used TMU
    /// @param texId The id of the texture to use
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool useTexture(const std::size_t target, const uint16_t texId);

    /// @brief Deletes a texture 
    /// @param texId The id of the texture to delete
    /// @return true if succeeded
    bool deleteTexture(const uint16_t texId) { return m_textureManager.deleteTexture(texId); }

    /// @brief Enable or disable a feature
    /// @param featureEnable The enabled features
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setFeatureEnableConfig(const FeatureEnableReg& featureEnable);

    /// @brief Sets the scissor box parameter
    /// @param x X coordinate of the box
    /// @param y Y coordinate of the box
    /// @param width Width of the box
    /// @param height Height of the box
    /// @return true if success
    bool setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height);

    /// @brief The wrapping mode of the texture in s direction
    /// @param texId The texture from where to change the parameter
    /// @param mode The new mode
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setTextureWrapModeS(const uint16_t texId, TextureWrapMode mode);

    /// @brief The wrapping mode of the texture in t direction
    /// @param texId The texture from where to change the parameter
    /// @param mode The new mode
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setTextureWrapModeT(const uint16_t texId, TextureWrapMode mode);

    /// @brief Enables the texture filtering for magnification
    /// @param texId The texture from where to change the parameter
    /// @param filter True to enable the filter
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool enableTextureMagFiltering(const uint16_t texId, bool filter);

    /// @brief Enables the texture filtering for minification (mipmapping)
    /// @param texId The texture from where to change the parameter
    /// @param filter True to enable the filter
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool enableTextureMinFiltering(const uint16_t texId, bool filter);

    /// @brief Sets the resolution of the renderer
    /// @param x X is the width of the produced image
    /// @param y Y is the height of the produced image
    /// @return true if success
    bool setRenderResolution(const std::size_t x, const std::size_t y);

    /// @brief Queries the maximum texture size in pixels
    /// @return The maximum texture size in pixel
    std::size_t getMaxTextureSize() const { return RenderConfig::MAX_TEXTURE_SIZE; }
    
    /// @brief Queries the maximum number of TMUs available for the hardware
    /// @brief The number of TMUs available
    std::size_t getTmuCount() const { return RenderConfig::TMU_COUNT; }

    /// @brief Queries of mip mapping is available on hardware
    /// @return true when mipmapping is available
    bool isMipmappingAvailable() const { return RenderConfig::ENABLE_MIPMAPPING; }

    /// @brief Configures the stencil buffer
    /// @param stencilConf The config of the stencil buffer
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setStencilBufferConfig(const StencilReg& stencilConf) { return writeReg(stencilConf); }

private:
    static constexpr std::size_t TEXTURE_NUMBER_OF_TEXTURES { RenderConfig::NUMBER_OF_TEXTURE_PAGES }; // Have as many pages as textures can exist. Probably the most reasonable value for the number of pages.

    using ListAssembler = DisplayListAssembler<RenderConfig>;
    using TextureManager = TextureMemoryManager<RenderConfig>; 

    template <typename TArg>
    bool writeReg(const TArg& regVal)
    {
        bool ret = true;
        for (std::size_t i = 0; i < m_displayLines; i++)
        {
            ret = ret && addCommand(i, WriteRegisterCmd { regVal });
        }
        return ret;
    }

    template <typename Command>
    bool addCommand(const std::size_t index, const Command& cmd)
    {
        bool ret = m_displayListAssembler[index + (DISPLAY_LINES * m_backList)].addCommand(cmd);
        if (!ret && (m_displayLines == 1))
        {
            intermediateUpload();
            ret = m_displayListAssembler[index + (DISPLAY_LINES * m_backList)].addCommand(cmd);
        }
        return ret;
    }

    void enableColorBufferInMemory() { m_colorBufferUseMemory = true; }
    void enableColorBufferStream() { m_colorBufferUseMemory = false; }
    bool setDepthBufferAddress(const uint32_t addr) { return writeReg(DepthBufferAddrReg { addr + RenderConfig::GRAM_MEMORY_LOC }); }
    bool setStencilBufferAddress(const uint32_t addr) { return writeReg(StencilBufferAddrReg { addr + RenderConfig::GRAM_MEMORY_LOC }); }
    bool writeToTextureConfig(const uint16_t texId, TmuTextureReg tmuConfig);
    bool setColorBufferAddress(const uint32_t addr);
    void uploadTextures();
    std::optional<FramebufferCmd> createCommitFramebufferCommand(const uint32_t i);
    FramebufferCmd createSwapFramebufferCommand();
    void clearAndInitDisplayList(const uint32_t i);
    void swapFramebuffer();
    void intermediateUpload();

    bool m_colorBufferUseMemory { true };
    uint32_t m_colorBufferAddr {};

    std::array<ListAssembler, DISPLAY_LINES * 2> m_displayListAssembler;
    std::size_t m_frontList = 0;
    std::size_t m_backList = 1;

    // Optimization for the scissor test to filter unecessary clean calls
    bool m_scissorEnabled { false };
    int32_t m_scissorYStart { 0 };
    int32_t m_scissorYEnd { 0 };

    std::size_t m_yLineResolution { 128 };
    std::size_t m_xResolution { 640 };
    std::size_t m_displayLines { DISPLAY_LINES };

    IBusConnector& m_busConnector;
    TextureManager m_textureManager;
    Rasterizer m_rasterizer { !RenderConfig::USE_FLOAT_INTERPOLATION };

    // Mapping of texture id and TMU
    std::array<uint16_t, RenderConfig::TMU_COUNT> m_boundTextures {};

    bool m_switchColorBuffer { true };
};

} // namespace rr
#endif // RENDERER_HPP
