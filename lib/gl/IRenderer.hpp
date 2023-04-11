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

#ifndef IRENDERER_HPP
#define IRENDERER_HPP

#include "Veci.hpp"
#include <stdint.h>
#include <array>
#include <utility>
#include <memory>
#include "Vec.hpp"
#include "registers/TexEnvReg.hpp"
#include "registers/FragmentPipelineReg.hpp"
#include "registers/FeatureEnableReg.hpp"
#include "registers/TmuTextureReg.hpp"
#include "registers/StencilReg.hpp"
#include "Triangle.hpp"

namespace rr
{
class IRenderer
{
public:
    static constexpr std::size_t MAX_TMU_COUNT { 2 };

    using TMU = uint8_t;
    using TextureWrapMode = TmuTextureReg::TextureWrapMode;
    
    struct TextureObject
    {
        using PixelFormat = TmuTextureReg::PixelFormat;
        enum class IntendedInternalPixelFormat
        {
            ALPHA,
            LUMINANCE,
            INTENSITY,
            LUMINANCE_ALPHA,
            RGB,
            RGBA,
            RGBA1,
        };

        uint16_t convertColor(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) const
        {
            uint16_t color {};
            switch (intendedPixelFormat)
            {
                case IntendedInternalPixelFormat::ALPHA: // RGBA4444
                    color = static_cast<uint16_t>(a >> 4);
                    break;
                case IntendedInternalPixelFormat::LUMINANCE: // RGB565
                    color |= static_cast<uint16_t>(r >> 3) << 11;
                    color |= static_cast<uint16_t>(r >> 2) << 5;
                    color |= static_cast<uint16_t>(r >> 3) << 0;
                    break;
                case IntendedInternalPixelFormat::INTENSITY: // RGBA4444
                    color |= static_cast<uint16_t>(r >> 4) << 12;
                    color |= static_cast<uint16_t>(r >> 4) << 8;
                    color |= static_cast<uint16_t>(r >> 4) << 4;
                    color |= static_cast<uint16_t>(r >> 4) << 0;
                    break;
                case IntendedInternalPixelFormat::LUMINANCE_ALPHA: // RGBA4444
                    color |= static_cast<uint16_t>(r >> 4) << 12;
                    color |= static_cast<uint16_t>(r >> 4) << 8;
                    color |= static_cast<uint16_t>(r >> 4) << 4;
                    color |= static_cast<uint16_t>(a >> 4) << 0;
                    break;
                case IntendedInternalPixelFormat::RGB: // RGB565
                    color |= static_cast<uint16_t>(r >> 3) << 11;
                    color |= static_cast<uint16_t>(g >> 2) << 5;
                    color |= static_cast<uint16_t>(b >> 3) << 0;
                    break;
                case IntendedInternalPixelFormat::RGBA: // RGBA4444
                    color |= static_cast<uint16_t>(r >> 4) << 12;
                    color |= static_cast<uint16_t>(g >> 4) << 8;
                    color |= static_cast<uint16_t>(b >> 4) << 4;
                    color |= static_cast<uint16_t>(a >> 4) << 0;
                    break;
                case IntendedInternalPixelFormat::RGBA1: // RGBA5551
                    color |= static_cast<uint16_t>(r >> 3) << 11;
                    color |= static_cast<uint16_t>(g >> 3) << 6;
                    color |= static_cast<uint16_t>(b >> 3) << 1;
                    color |= static_cast<uint16_t>(a >> 7) << 0;
                    break;
                default:
                break;
            }
            return color;
        }

        PixelFormat getPixelFormat() const
        {
            PixelFormat format {};
            switch (intendedPixelFormat)
            {
                case IntendedInternalPixelFormat::ALPHA:
                    format = PixelFormat::RGBA4444;
                    break;
                case IntendedInternalPixelFormat::LUMINANCE:
                    format = PixelFormat::RGB565;
                    break;
                case IntendedInternalPixelFormat::INTENSITY:
                    format = PixelFormat::RGBA4444;
                    break;
                case IntendedInternalPixelFormat::LUMINANCE_ALPHA:
                    format = PixelFormat::RGBA4444;
                    break;
                case IntendedInternalPixelFormat::RGB:
                    format = PixelFormat::RGB565;
                    break;
                case IntendedInternalPixelFormat::RGBA:
                    format = PixelFormat::RGBA4444;
                    break;
                case IntendedInternalPixelFormat::RGBA1:
                    format = PixelFormat::RGBA5551;
                    break;
                default:
                break;
            }
            return format;
        }

        std::shared_ptr<const uint16_t> pixels {}; ///< The texture in the format defined by PixelFormat
        const uint16_t width {}; ///< The width of the texture
        const uint16_t height {}; ///< The height of the texture
        const IntendedInternalPixelFormat intendedPixelFormat {}; ///< The intended pixel format which is converted to a type of PixelFormat
    };

    /// @brief Will render a triangle which is constructed with the given parameters
    /// @return true if the triangle was rendered, otherwise the display list was full and the triangle can't be added
    virtual bool drawTriangle(const Triangle& triangle) = 0;

    /// @brief Switches between the front and the back list. The front list is the list which is uploaded to the
    /// device. The back list is used to compile a new list.
    /// @note: Only call this method when uploadDisplayList() has finished.
    virtual void swapDisplayList() = 0;

    /// @brief Uploads the front display list. It is safe to use this method in another thread. Just make sure 
    /// that swapDisplayList() and this method are locked. Only call swapDisplayList() when this method has finished.
    /// @see swapDisplayList()
    virtual void uploadDisplayList() = 0;

    /// @brief Creates a new texture 
    /// @return pair with the first value to indicate if the operation succeeded (true) and the second value with the id
    virtual std::pair<bool, uint16_t> createTexture() = 0;

    /// @brief This will update the texture data of the texture with the given id
    /// @param texId The texture id which texture has to be updated
    /// @param textureObject The object which contains the texture and all its meta data
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool updateTexture(const uint16_t texId, const TextureObject& textureObject) = 0;

    /// @brief Returns a texture associated to the texId
    /// @param texId The texture id of the texture to get the data from
    /// @return The texture object
    virtual TextureObject getTexture(const uint16_t texId) = 0;
    
    /// @brief Activates a texture which then is used for rendering
    /// @param target The used TMU
    /// @param texId The id of the texture to use
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool useTexture(const TMU target, const uint16_t texId) = 0; 

    /// @brief The wrapping mode of the texture in s direction
    /// @param texId The texture from where to change the parameter
    /// @param mode The new mode
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setTextureWrapModeS(const uint16_t texId, TextureWrapMode mode) = 0;

    /// @brief The wrapping mode of the texture in t direction
    /// @param texId The texture from where to change the parameter
    /// @param mode The new mode
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setTextureWrapModeT(const uint16_t texId, TextureWrapMode mode) = 0;

    /// @brief Enables the texture filtering
    /// @param texId The texture from where to change the parameter
    /// @param filter True to enable the filter
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool enableTextureMagFiltering(const uint16_t texId, bool filter) = 0;

    /// @brief Deletes a texture 
    /// @param texId The id of the texture to delete
    /// @return true if succeeded
    virtual bool deleteTexture(const uint16_t texId) = 0;

    /// @brief Will clear a buffer
    /// @param frameBuffer Will clear the frame buffer
    /// @param zBuffer Will clear the z buffer
    /// @param stencilBuffer Will clear the stencil buffer
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool clear(const bool frameBuffer, const bool zBuffer, const bool stencilBuffer) = 0;

    /// @brief Will set a color which is used as clear color for the frame buffer
    /// when clear() is called
    /// @param color the color in ABGR
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setClearColor(const Vec4i& color) = 0;

    /// @brief Will set a depth which is used as initial value for the depth buffer when clear() is called
    /// @param depth The depth which is used by calling clear()
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setClearDepth(uint16_t depth) = 0;

    /// @brief Configures the texture environment. See glTexEnv()
    /// @param pname parameter name of the env parameter
    /// @param param Function of the tex env
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setTexEnv(const TexEnvReg& texEnvConfig) = 0;

    /// @brief Set a static color for the tex environment
    /// @param target is used TMU
    /// @param color the color in ABGR
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setTexEnvColor(const TMU target, const Vec4i& color) = 0;

    /// @brief Updates the fragment pipeline configuration 
    /// @param pipelineConf The new pipeline configuration 
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setFragmentPipelineConfig(const FragmentPipelineReg& pipelineConf) = 0;

    /// @brief Set the fog color
    /// @param color the color in ABGR
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setFogColor(const Vec4i& color) = 0;

    /// @brief Sets the fog LUT. Note that the fog uses the w value as index (the distance from eye to the polygon znear..zfar)
    /// and not z (clamped value of 0.0..1.0)
    /// @param fogLut the fog lookup table
    /// The fog LUT uses a exponential distribution of w, means fogLut[0] = f(1), fogLut[1] = f(2), fogLut[2] = f(4), fogLut[3] = f(8).
    /// The fog values between start and end must not exceed 1.0f
    /// @param start the start value of the fog
    /// @param end the end value of the fog
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setFogLut(const std::array<float, 33>& fogLut, float start, float end) = 0;

    /// @brief Enable or disable a feature
    /// @param featureEnable The enabled features
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setFeatureEnableConfig(const FeatureEnableReg& featureEnable) = 0;

    /// @brief Sets the scissor box parameter
    /// @param x X coordinate of the box
    /// @param y Y coordinate of the box
    /// @param width Width of the box
    /// @param height Height of the box
    /// @return true if success
    virtual bool setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height) = 0;

    /// @brief Sets the resolution of the renderer
    /// @param x X is the width of the produced image
    /// @param y Y is the height of the produced image
    /// @return true if success
    virtual bool setRenderResolution(const uint16_t x, const uint16_t y) = 0;

    /// @brief Enables a color buffer in memory. All rendered images will then be stored in this area.
    /// @param addr The address of the color buffer.
    virtual void enableColorBufferInMemory(const uint32_t addr) = 0;

    /// @brief Enables the stream port of the hardware. All rendered images will be directly streamed.
    /// The color buffer in memory is disabled.
    virtual void enableColorBufferStream() = 0;

    /// @brief Queries the maximum texture size in pixels
    /// @return The maximum texture size in pixel
    virtual uint16_t getMaxTextureSize() const = 0;

    /// @brief Queries the maximum number of TMUs available for the hardware
    /// @brief The number of TMUs available
    virtual TMU getTmuCount() const = 0;

    /// @brief Configures the stencil buffer
    /// @param stencilConf The config of the stencil buffer
    /// @return true if succeeded, false if it was not possible to apply this command (for instance, displaylist was out if memory)
    virtual bool setStencilBufferConfig(const StencilReg& stencilConf) = 0;
};

} // namespace rr
#endif // IRENDERER_HPP
