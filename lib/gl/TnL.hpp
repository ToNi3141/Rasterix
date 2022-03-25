// RasteriCEr
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2022 ToNi3141

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

#ifndef TNL_HPP
#define TNL_HPP

#include "Vec.hpp"
#include "IRenderer.hpp"
#include <tuple>
#include <array>
#include "Mat44.hpp"

class TnL
{
public:
    static constexpr uint8_t MAX_LIGHTS = 8;

    enum TexGenMode
    {
        OBJECT_LINEAR,
        EYE_LINEAR,
        SPHERE_MAP
    };

    enum CullMode
    {
        BACK,
        FRONT,
        FRONT_AND_BACK
    };

    struct RenderObj
    {
        enum DrawMode
        {
            TRIANGLES,
            TRIANGLE_FAN,
            TRIANGLE_STRIP
        };

        enum Type
        {
            BYTE,
            UNSIGNED_BYTE,
            SHORT,
            UNSIGNED_SHORT,
            FLOAT,
            UNSIGNED_INT
        };

        DrawMode drawMode;
        uint32_t count;
        Vec4 vertexColor;

        bool vertexArrayEnabled;
        uint8_t vertexSize;
        Type vertexType;
        uint32_t vertexStride;
        const void* vertexPointer;

        bool texCoordArrayEnabled;
        uint8_t texCoordSize;
        Type texCoordType;
        uint32_t texCoordStride;
        const void* texCoordPointer;

        bool normalArrayEnabled;
        Type normalType;
        uint32_t normalStride;
        const void* normalPointer;

        bool colorArrayEnabled;
        uint8_t colorSize;
        Type colorType;
        uint32_t colorStride;
        const void* colorPointer;

        bool indicesEnabled;
        Type indicesType;
        const void* indicesPointer;

        uint32_t arrayOffset;

        template <typename T>
        bool getFromArray(T& vec, const Type type, const void* arr, const uint32_t stride, uint8_t size, const uint32_t index) const
        {
            if (arr)
            {
                const uint32_t indexWithStride = (stride == 0) ? index * size : index * stride;
                const int8_t* a = reinterpret_cast<const int8_t*>(arr);
                if (stride == 0) {
                    switch (type)
                    {
                    case Type::BYTE:
                        vec.fromArray(a + (indexWithStride * 1), size);
                        break;
                    case Type::UNSIGNED_BYTE:
                        vec.fromArray(reinterpret_cast<const uint8_t*>(a + (indexWithStride * 1)), size);
                        break;
                    case Type::SHORT:
                        vec.fromArray(reinterpret_cast<const int16_t*>(a + (indexWithStride * 2)), size);
                        break;
                    case Type::UNSIGNED_SHORT:
                        vec.fromArray(reinterpret_cast<const uint16_t*>(a + (indexWithStride * 2)), size);
                        break;
                    case Type::UNSIGNED_INT:
                        vec.fromArray(reinterpret_cast<const int32_t*>(a + (indexWithStride * 4)), size);
                        break;
                    default:
                        vec.fromArray(reinterpret_cast<const float*>(a + (indexWithStride * 4)), size);
                        break;
                    }
                }
                else
                {
                    switch (type)
                    {
                    case Type::BYTE:
                        vec.fromArray(a + indexWithStride, size);
                        break;
                    case Type::UNSIGNED_BYTE:
                        vec.fromArray(reinterpret_cast<const uint8_t*>(a + indexWithStride), size);
                        break;
                    case Type::SHORT:
                        vec.fromArray(reinterpret_cast<const int16_t*>(a + indexWithStride), size);
                        break;
                    case Type::UNSIGNED_SHORT:
                        vec.fromArray(reinterpret_cast<const uint16_t*>(a + indexWithStride), size);
                        break;
                    case Type::UNSIGNED_INT:
                        vec.fromArray(reinterpret_cast<const int32_t*>(a + indexWithStride), size);
                        break;
                    default:
                        vec.fromArray(reinterpret_cast<const float*>(a + indexWithStride), size);
                        break;
                    }
                }

                return true;
            }
            return false;
        }

        bool getVertex(Vec4& vec, const uint32_t index) const
        {
            vec.initHomogeneous();
            return getFromArray(vec, vertexType, vertexPointer, vertexStride, vertexSize, index);
        }

        bool getTexCoord(Vec2& vec, const uint32_t index) const
        {
            return getFromArray(vec, texCoordType, texCoordPointer, texCoordStride, texCoordSize, index);
        }

        bool getColor(Vec4& vec, const uint32_t index) const
        {
            bool retVal = getFromArray(vec, colorType, colorPointer, colorStride, colorSize, index);
            if (retVal)
            {
                switch (colorType) {
                case Type::UNSIGNED_BYTE:
                case Type::UNSIGNED_SHORT:
                case Type::UNSIGNED_INT:
                    // Map unsigned values to 0.0 .. 1.0
                    vec *= 1.0f / 255.0f;
                    break;
                case Type::BYTE:
                case Type::SHORT:
                    // Map signed values to -1.0 .. 1.0
                    vec *= 1.0f / 127.0f;
                    break;
                default:
                    // Other types like floats can be used as they are
                    break;
                }
            }
            return retVal;
        }

        bool getNormal(Vec3& vec, const uint32_t index) const
        {
            return getFromArray(vec, normalType, normalPointer, normalStride, 3, index);
        }

        uint32_t getIndex(const uint32_t index) const
        {
            if (indicesEnabled)
            {
                switch (indicesType)
                {
                case Type::BYTE:
                case Type::UNSIGNED_BYTE:
                    return static_cast<const uint8_t*>(indicesPointer)[index];
                case Type::SHORT:
                case Type::UNSIGNED_SHORT:
                    return static_cast<const uint16_t*>(indicesPointer)[index];
                case Type::UNSIGNED_INT:
                    return static_cast<const uint32_t*>(indicesPointer)[index];
                default:
                    return index;
                }
            }
            return index;
        }
    };

    struct Triangle
    {
        Vec4 v0;
        Vec4 v1;
        Vec4 v2;
        Vec2 st0;
        Vec2 st1;
        Vec2 st2;
        Vec3 n0;
        Vec3 n1;
        Vec3 n2;
        Vec4 color0;
        Vec4 color1;
        Vec4 color2;
    };

    TnL();

    bool drawObj(IRenderer& renderer, const RenderObj& obj);
    bool drawTriangle(IRenderer &renderer, const Triangle& triangle);
    void setViewport(const int16_t x, const int16_t y, const int16_t width, const int16_t height);
    void setDepthRange(const float zNear, const float zFar);
    void setModelProjectionMatrix(const Mat44& m);
    void setModelMatrix(const Mat44& m);
    void setNormalMatrix(const Mat44& m);

    void enableLighting(bool enable);
    void setEmissiveColorMaterial(const Vec4& color);
    void setAmbientColorMaterial(const Vec4& color);
    void setAmbientColorScene(const Vec4& color);
    void setDiffuseColorMaterial(const Vec4& color);
    void setSpecularColorMaterial(const Vec4& color);
    void setSpecularExponentMaterial(const float val);
    void enableLight(const uint8_t light, const bool enable);
    void setAmbientColorLight(const uint8_t light, const Vec4& color);
    void setDiffuseColorLight(const uint8_t light, const Vec4& color);
    void setSpecularColorLight(const uint8_t light, const Vec4& color);
    void setPosLight(const uint8_t light, const Vec4& pos);
    void setConstantAttenuationLight(const uint8_t light, const float val);
    void setLinearAttenuationLight(const uint8_t light, const float val);
    void setQuadraticAttenuationLight(const uint8_t light, const float val);
    void enableColorMaterial(bool emission, bool ambient, bool diffuse, bool specular);

    void enableTexGenS(bool enable);
    void enableTexGenT(bool enable);
    void setTexGenModeS(TexGenMode mode);
    void setTexGenModeT(TexGenMode mode);
    void setTexGenVecObjS(const Vec4& val);
    void setTexGenVecObjT(const Vec4& val);
    void setTexGenVecEyeS(const Vec4& val);
    void setTexGenVecEyeT(const Vec4& val);

    void enableCulling(bool enable);
    void setCullMode(CullMode mode);
private:
    // Each clipping plane can potentally introduce one more vertex, so, we have 3 vertexes, plus 6 possible planes, results in 9 vertexes
    using ClipVertList = std::array<Vec4, 9>;
    using ClipStList = std::array<Vec2, 9>;
    using ClipColorList = std::array<Vec4, 9>;

    enum OutCode
    {
        NONE    = 0x00,
        NEAR    = 0x01,
        FAR     = 0x02,
        TOP     = 0x04,
        BOTTOM  = 0x08,
        LEFT    = 0x10,
        RIGHT   = 0x20
    };

    struct MaterialConfig
    {
        Vec4 emissiveColor{{0.0f, 0.0f, 0.0f, 1.0f}};
        Vec4 ambientColor{{0.2f, 0.2f, 0.2f, 1.0}};
        Vec4 ambientColorScene{{0.2f, 0.2f, 0.2f, 1.0f}};
        Vec4 diffuseColor{{0.8f, 0.8f, 0.8f, 1.0}};
        Vec4 specularColor{{0.0f, 0.0f, 0.0f, 1.0}};
        float specularExponent{0.0f};
    };

    struct LightConfig
    {
        bool enable{false};
        Vec4 ambientColor{{0.0f, 0.0f, 0.0f, 1.0f}};
        Vec4 diffuseColor{{1.0f, 1.0f, 1.0f, 1.0f}}; // For other lights than light0 the default value is {{0.0f, 0.0f, 0.0f, 1.0f}}
        Vec4 specularColor{{1.0f, 1.0f, 1.0f, 1.0f}}; // For other lights than light0 the default value is {{0.0f, 0.0f, 0.0f, 1.0f}}
        Vec4 position{{0.0f, 0.0f, 1.0f, 0.0f}};
        Vec3 spotlightDirection{{0.0f, 0.0f, -1.0f}};
        float spotlightExponent{0.0f};
        float spotlightCutoff{180.0f};
        float constantAttenuation{1.0f};
        float linearAttenuation{0.0f};
        float quadraticAttenuation{0.0f};

        bool localViewer{false}; // Not necessary, local viewer is not supported in OpenGL ES because of performance degradation (GL_LIGHT_MODEL_LOCAL_VIEWER)

        Vec4 preCalcDirectionalLightDir;
        Vec4 preCalcHalfWayVectorInfinite;

        void preCalcVectors()
        {
            // Directional Light Direction
            preCalcDirectionalLightDir = position;
            preCalcDirectionalLightDir.unit();

            // Half Way Vector from infinite viewer
            const Vec4 pointEye{{0.0f, 0.0f, 1.0f, 1.0f}};
            preCalcHalfWayVectorInfinite = preCalcDirectionalLightDir + pointEye;
            preCalcHalfWayVectorInfinite.unit();
        }
    };

    float lerpAmt(OutCode plane, const Vec4 &v0, const Vec4 &v1);
    void lerpVert(Vec4& vOut, const Vec4& v0, const Vec4& v1, const float amt);
    void lerpSt(Vec2& vOut, const Vec2& v0, const Vec2& v1, const float amt);
    OutCode outCode(const Vec4 &v);
    std::tuple<const uint32_t, ClipVertList &, ClipStList &, ClipColorList &> clip(ClipVertList& vertList,
                                                                                   ClipVertList& vertListBuffer,
                                                                                   ClipStList& stList,
                                                                                   ClipStList& stListBuffer,
                                                                                   ClipColorList& colorList,
                                                                                   ClipColorList& colorListBuffer);
    uint32_t clipAgainstPlane(ClipVertList& vertListOut,
                              ClipStList& stListOut,
                              ClipColorList &colorListOut,
                              const OutCode clipPlane,
                              const ClipVertList& vertListIn,
                              const ClipStList& stListIn,
                              const ClipColorList &colorListIn,
                              const uint32_t listInSize);

    inline void viewportTransform(Vec4 &v0, Vec4 &v1, Vec4 &v2);
    inline void viewportTransform(Vec4 &v);
    inline void perspectiveDivide(Vec4& v);


    void calculateSceneLight(Vec4& sceneLight, const Vec4 &emissiveColor, const Vec4 &ambientColor, const Vec4 &ambientColorScene) const;
    void calculateLight(Vec4 &color,
                        const LightConfig& lightConfig,
                        const float materialSpecularExponent,
                        const Vec4& materialAmbientColor,
                        const Vec4& materialDiffuseColor,
                        const Vec4& materialSpecularColor,
                        const Vec4& v0,
                        const Vec3& n0) const;
    void calculateTexGenCoords(ClipStList& stList, const Vec4& v0, const Vec4& v1, const Vec4& v2) const;

    Mat44 m_t; // ModelViewProjection
    Mat44 m_m; // ModelView
    Mat44 m_n; // Normal

    float m_depthRangeZNear = 0.0f;
    float m_depthRangeZFar = 1.0f;
    int16_t m_viewportX = 0;
    int16_t m_viewportY = 0;
    int16_t m_viewportHeight = 0;
    int16_t m_viewportWidth = 0;
    float m_viewportXShift = 0.0f;
    float m_viewportYShift = 0.0f;
    float m_viewportHeightHalf = 0.0f;
    float m_viewportWidthHalf = 0.0f;

    std::array<LightConfig, MAX_LIGHTS> m_lights;
    MaterialConfig m_material{};
    bool m_enableLighting{false};
    bool m_enableColorMaterialEmission{false};
    bool m_enableColorMaterialAmbient{false};
    bool m_enableColorMaterialDiffuse{false};
    bool m_enableColorMaterialSpecular{false};

    bool m_texGenEnableS{false};
    bool m_texGenEnableT{false};
    TexGenMode m_texGenModeS{TexGenMode::EYE_LINEAR};
    TexGenMode m_texGenModeT{TexGenMode::EYE_LINEAR};
    Vec4 m_texGenVecObjS{{1.0f, 0.0f, 0.0f, 0.0f}};
    Vec4 m_texGenVecObjT{{0.0f, 1.0f, 0.0f, 0.0f}};
    Vec4 m_texGenVecEyeS{{1.0f, 0.0f, 0.0f, 0.0f}};
    Vec4 m_texGenVecEyeT{{0.0f, 1.0f, 0.0f, 0.0f}};

    bool m_enableCulling{false};
    CullMode m_cullMode{CullMode::BACK};

    friend TnL::OutCode operator|=(TnL::OutCode& lhs, TnL::OutCode rhs);
};

TnL::OutCode operator|=(TnL::OutCode& lhs, TnL::OutCode rhs);

#endif // TNL_HPP
