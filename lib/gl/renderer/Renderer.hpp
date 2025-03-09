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

#include "Rasterizer.hpp"
#include "Renderer.hpp"
#include "TextureMemoryManager.hpp"
#include "displaylist/DisplayList.hpp"
#include "displaylist/DisplayListAssembler.hpp"
#include "displaylist/DisplayListDispatcher.hpp"
#include "displaylist/DisplayListDoubleBuffer.hpp"
#include "math/Vec.hpp"
#include "renderer/IDevice.hpp"
#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <stdint.h>
#include <string.h>

#include "RenderConfigs.hpp"
#include "commands/FogLutStreamCmd.hpp"
#include "commands/FramebufferCmd.hpp"
#include "commands/NopCmd.hpp"
#include "commands/RegularTriangleCmd.hpp"
#include "commands/TextureStreamCmd.hpp"
#include "commands/TriangleStreamCmd.hpp"
#include "commands/WriteRegisterCmd.hpp"
#include "registers/BaseColorReg.hpp"
#include "registers/ColorBufferAddrReg.hpp"
#include "registers/ColorBufferClearColorReg.hpp"
#include "registers/DepthBufferAddrReg.hpp"
#include "registers/DepthBufferClearDepthReg.hpp"
#include "registers/FeatureEnableReg.hpp"
#include "registers/FogColorReg.hpp"
#include "registers/FragmentPipelineReg.hpp"
#include "registers/RenderResolutionReg.hpp"
#include "registers/ScissorEndReg.hpp"
#include "registers/ScissorStartReg.hpp"
#include "registers/StencilBufferAddrReg.hpp"
#include "registers/StencilReg.hpp"
#include "registers/TexEnvColorReg.hpp"
#include "registers/TexEnvReg.hpp"
#include "registers/TmuTextureReg.hpp"
#include "registers/YOffsetReg.hpp"

namespace rr
{

class Renderer
{
public:
    Renderer(IDevice& device);

    ~Renderer();

    /// @brief Will render a triangle which is constructed with the given parameters
    /// @return true if the triangle was rendered, otherwise the display list was full and the triangle can't be added
    bool drawTriangle(const TransformedTriangle& triangle);

    /// @brief Starts the rendering process by uploading textures and the displaylist and also swapping
    /// the framebuffers
    void swapDisplayList();

    /// @brief Uploads the display list to the hardware
    void uploadDisplayList();

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

    /// @brief The wrapping mode of the texture in s direction
    /// @param target The used TMU
    /// @param texId The texture from where to change the parameter
    /// @param mode The new mode
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setTextureWrapModeS(const std::size_t target, const uint16_t texId, TextureWrapMode mode);

    /// @brief The wrapping mode of the texture in t direction
    /// @param target The used TMU
    /// @param texId The texture from where to change the parameter
    /// @param mode The new mode
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setTextureWrapModeT(const std::size_t target, const uint16_t texId, TextureWrapMode mode);

    /// @brief Enables the texture filtering for magnification
    /// @param target The used TMU
    /// @param texId The texture from where to change the parameter
    /// @param filter True to enable the filter
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool enableTextureMagFiltering(const std::size_t target, const uint16_t texId, bool filter);

    /// @brief Enables the texture filtering for minification (mipmapping)
    /// @param target The used TMU
    /// @param texId The texture from where to change the parameter
    /// @param filter True to enable the filter
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool enableTextureMinFiltering(const std::size_t target, const uint16_t texId, bool filter);

    /// @brief Sets the resolution of the renderer
    /// @param x X is the width of the produced image
    /// @param y Y is the height of the produced image
    /// @return true if success
    bool setRenderResolution(const std::size_t x, const std::size_t y);

    /// @brief Sets the scissor box parameter
    /// @param x X coordinate of the box
    /// @param y Y coordinate of the box
    /// @param width Width of the box
    /// @param height Height of the box
    /// @return true if success

    bool setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height);

    /// @brief Sets the fog LUT. Note that the fog uses the w value as index (the distance from eye to the polygon znear..zfar)
    /// and not z (clamped value of 0.0..1.0)
    /// @param fogLut the fog lookup table
    /// The fog LUT uses a exponential distribution of w, means fogLut[0] = f(1), fogLut[1] = f(2), fogLut[2] = f(4), fogLut[3] = f(8).
    /// The fog values between start and end must not exceed 1.0f
    /// @param start the start value of the fog
    /// @param end the end value of the fog
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool setFogLut(const std::array<float, 33>& fogLut, float start, float end) { return addCommand(FogLutStreamCmd { fogLut, start, end }); }

    /// @brief Will clear a buffer
    /// @param frameBuffer Will clear the frame buffer
    /// @param zBuffer Will clear the z buffer
    /// @param stencilBuffer Will clear the stencil buffer
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    bool clear(const bool colorBuffer, const bool depthBuffer, const bool stencilBuffer);

    bool setStencilBufferConfig(const StencilReg& stencilConf) { return writeReg(stencilConf); }
    bool setClearColor(const ColorBufferClearColorReg& color) { return writeReg(color); }
    bool setClearDepth(const DepthBufferClearDepthReg& depth) { return writeReg(depth); }
    bool setFragmentPipelineConfig(const FragmentPipelineReg& pipelineConf) { return writeReg(pipelineConf); }
    bool setTexEnv(const TexEnvReg& texEnvConfig) { return writeReg(texEnvConfig); }
    bool setTexEnvColor(const TexEnvColorReg& color) { return writeReg(color); }
    bool setFogColor(const FogColorReg& color) { return writeReg(color); }
    bool setFeatureEnableConfig(const FeatureEnableReg& featureEnable);

private:
    static constexpr std::size_t TEXTURE_NUMBER_OF_TEXTURES { RenderConfig::NUMBER_OF_TEXTURE_PAGES }; // Have as many pages as textures can exist. Probably the most reasonable value for the number of pages.

    static constexpr uint8_t ALIGNMENT { 4 }; // 4 bytes alignment (for the 32 bit AXIS)
    using DisplayListAssemblerType = displaylist::DisplayListAssembler<RenderConfig::TMU_COUNT, displaylist::DisplayList>;
    using DisplayListAssemblerArrayType = std::array<DisplayListAssemblerType, RenderConfig::getDisplayLines()>;
    using TextureManagerType = TextureMemoryManager<RenderConfig>;
    using DisplayListDispatcherType = displaylist::DisplayListDispatcher<RenderConfig, DisplayListAssemblerArrayType>;
    using DisplayListDoubleBufferType = displaylist::DisplayListDoubleBuffer<DisplayListDispatcherType>;

    template <typename TArg>
    bool writeReg(const TArg& regVal)
    {
        return addCommand(WriteRegisterCmd { regVal });
    }

    template <typename Command>
    bool addCommand(const Command& cmd)
    {
        bool ret = m_displayListBuffer.getBack().addCommand(cmd);
        if (!ret && m_displayListBuffer.getBack().singleList())
        {
            intermediateUpload();
            ret = m_displayListBuffer.getBack().addCommand(cmd);
        }
        return ret;
    }

    template <typename Command>
    bool addLastCommand(const Command& cmd)
    {
        return m_displayListBuffer.getBack().addLastCommand(cmd);
    }

    template <typename Factory>
    bool addLastCommandWithFactory(const Factory& commandFactory)
    {
        return m_displayListBuffer.getBack().addLastCommandWithFactory_if(commandFactory,
            [](std::size_t, std::size_t, std::size_t, std::size_t)
            { return true; });
    }

    template <typename Factory>
    bool addCommandWithFactory(const Factory& commandFactory)
    {
        return addCommandWithFactory_if(commandFactory,
            [](std::size_t, std::size_t, std::size_t, std::size_t)
            { return true; });
    }

    template <typename Factory, typename Pred>
    bool addCommandWithFactory_if(const Factory& commandFactory, const Pred& pred)
    {
        return m_displayListBuffer.getBack().addCommandWithFactory_if(commandFactory, pred);
    }

    template <typename Function>
    bool displayListLooper(const Function& func)
    {
        return m_displayListBuffer.getBack().displayListLooper(func);
    }

    void clearDisplayListAssembler()
    {
        m_displayListBuffer.getBack().clearDisplayListAssembler();
    }

    void switchDisplayLists()
    {
        m_displayListBuffer.swap();
    }

    template <typename TriangleCmd>
    bool addMultiListTriangle(TriangleCmd& triangleCmd)
    {
        const auto factory = [&triangleCmd](DisplayListDispatcherType& dispatcher, const std::size_t i, const std::size_t, const std::size_t, const std::size_t resY)
        {
            const std::size_t currentScreenPositionStart = i * resY;
            const std::size_t currentScreenPositionEnd = currentScreenPositionStart + resY;
            if (triangleCmd.isInBounds(currentScreenPositionStart, currentScreenPositionEnd))
            {
                // The floating point rasterizer can automatically increment all attributes
                if constexpr (RenderConfig::USE_FLOAT_INTERPOLATION)
                {
                    return dispatcher.addCommand(i, triangleCmd);
                }
                else
                {
                    return dispatcher.addCommand(i, triangleCmd.getIncremented(currentScreenPositionStart, currentScreenPositionEnd));
                }
            }
            return true;
        };
        return displayListLooper(factory);
    }

    template <typename TriangleCmd>
    bool addTriangleCmd(TriangleCmd& triangleCmd)
    {
        if (!triangleCmd.isVisible())
        {
            return true;
        }

        if constexpr (DisplayListDispatcherType::singleList())
        {
            return addCommand(triangleCmd);
        }
        else
        {
            return addMultiListTriangle(triangleCmd);
        }
        return true;
    }

    bool setDepthBufferAddress(const uint32_t addr) { return writeReg(DepthBufferAddrReg { addr }); }
    bool setStencilBufferAddress(const uint32_t addr) { return writeReg(StencilBufferAddrReg { addr }); }
    bool writeToTextureConfig(const std::size_t target, const uint16_t texId, TmuTextureReg tmuConfig);
    bool setColorBufferAddress(const uint32_t addr);
    void uploadTextures();
    void swapFramebuffer();
    void intermediateUpload();
    void setYOffset();
    void initDisplayLists();
    void addLineColorBufferAddresses();
    void addCommitFramebufferCommand();
    void addColorBufferAddressOfTheScreen();
    void swapScreenToNewColorBuffer();

    uint32_t m_colorBufferAddr {};
    bool m_switchColorBuffer { true };

    // Optimization for the scissor test to filter unecessary clean calls
    bool m_scissorEnabled { false };
    int32_t m_scissorYStart { 0 };
    int32_t m_scissorYEnd { 0 };

    IDevice& m_device;
    TextureManagerType m_textureManager;
    Rasterizer m_rasterizer { !RenderConfig::USE_FLOAT_INTERPOLATION };

    // Instantiation of the displaylist assemblers
    std::array<DisplayListAssemblerArrayType, 2> m_displayListAssembler {};
    std::array<DisplayListDispatcherType, 2> m_displayListDispatcher { m_displayListAssembler[0], m_displayListAssembler[1] };
    DisplayListDoubleBufferType m_displayListBuffer { m_displayListDispatcher[0], m_displayListDispatcher[1] };
};

} // namespace rr
#endif // RENDERER_HPP
