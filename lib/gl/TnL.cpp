// Rasterix
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

#include "TnL.hpp"
#include "Vec.hpp"
#include "Veci.hpp"
#include <string.h>
#include <stdlib.h>
#include "Rasterizer.hpp"

// The Arduino IDE will produce compile errors when using std::min and std::max
#include <algorithm>    // std::max
#define max std::max
#define min std::min


TnL::TnL()
{
    m_t.identity();
    m_m.identity();
    m_n.identity();

    setEmissiveColorMaterial({{0.0f, 0.0f, 0.0f, 1.0f}});
    setAmbientColorMaterial({{0.2f, 0.2f, 0.2f, 1.0}});
    setAmbientColorScene({{0.2f, 0.2f, 0.2f, 1.0f}});
    setDiffuseColorMaterial({{0.8f, 0.8f, 0.8f, 1.0}});
    setSpecularColorMaterial({{0.0f, 0.0f, 0.0f, 1.0}});
    setSpecularExponentMaterial(0.0f);

    for (uint8_t i = 0; i < m_lights.size(); i++)
    {
        enableLight(i, false);
        setAmbientColorLight(i, {{0.0f, 0.0f, 0.0f, 1.0f}});
        setDiffuseColorLight(i, {{0.0f, 0.0f, 0.0f, 1.0f}});
        setSpecularColorLight(i, {{0.0f, 0.0f, 0.0f, 1.0f}});
        setPosLight(i, {{0.0f, 0.0f, 1.0f, 0.0f}});
        setConstantAttenuationLight(i, 1.0f);
        setLinearAttenuationLight(i, 0.0f);
        setQuadraticAttenuationLight(i, 0.0f);
    }
    setDiffuseColorLight(0, {{1.0f, 1.0f, 1.0f, 1.0f}}); // Light Zero has a slightly different configuration here
    setSpecularColorLight(0, {{1.0f, 1.0f, 1.0f, 1.0f}}); // Light Zero has a slightly different configuration here
}

bool TnL::drawObj(IRenderer &renderer, const TnL::RenderObj &obj)
{
    // TODO: It is possible to precompute all transformations and lights here and save it in an temporary array.
    // That avoids that we have to transform and light every vertex three times.
    // Then one can call drawTriangle() which then applies the clipping and sends it to the rasterizer.

    Triangle triangle;
    uint32_t index0 = 0;
    uint32_t index1 = 0;
    uint32_t index2 = 0;
    for (uint32_t i = 0; i < obj.count - 2; )
    {
        switch (obj.drawMode) {
        case RenderObj::DrawMode::TRIANGLES:
            index0 = obj.getIndex(i);
            index1 = obj.getIndex(i + 1);
            index2 = obj.getIndex(i + 2);
            i += 3;
            break;
        case RenderObj::DrawMode::TRIANGLE_FAN:
            index0 = obj.getIndex(0);
            index1 = obj.getIndex(i + 1);
            index2 = obj.getIndex(i + 2);
            i += 1;
            break;
        case RenderObj::DrawMode::TRIANGLE_STRIP:
            if (i & 0x1)
            {
                index0 = obj.getIndex(i + 1);
                index1 = obj.getIndex(i);
                index2 = obj.getIndex(i + 2);
            }
            else
            {
                index0 = obj.getIndex(i);
                index1 = obj.getIndex(i + 1);
                index2 = obj.getIndex(i + 2);
            }
            i += 1;
            break;
        default:
            break;
        }

        if (obj.vertexArrayEnabled)
        {
            obj.getVertex(triangle.v0, index0);
            obj.getVertex(triangle.v1, index1);
            obj.getVertex(triangle.v2, index2);
        }

        if (obj.texCoordArrayEnabled)
        {
            obj.getTexCoord(triangle.st0, index0);
            obj.getTexCoord(triangle.st1, index1);
            obj.getTexCoord(triangle.st2, index2);
        }

        if (obj.colorArrayEnabled)
        {
            obj.getColor(triangle.color0, index0);
            obj.getColor(triangle.color1, index1);
            obj.getColor(triangle.color2, index2);
        }
        else
        {
            // If no color is defined, use the globally setted color
            triangle.color0 = obj.vertexColor;
            triangle.color1 = obj.vertexColor;
            triangle.color2 = obj.vertexColor;
        }

        if (obj.normalArrayEnabled)
        {
            obj.getNormal(triangle.n0, index0);
            obj.getNormal(triangle.n1, index1);
            obj.getNormal(triangle.n2, index2);
        }

        if (!drawTriangle(renderer, triangle))
        {
            return false;
        }
    }
    return true;
}

bool TnL::drawTriangle(IRenderer &renderer, const Triangle& triangle)
{
    Vec4 color0;
    Vec4 color1;
    Vec4 color2;
    if (m_enableLighting)
    {
        Vec4 v0, v1, v2;
        Vec3 n0, n1, n2;

        m_n.transform(n0, triangle.n0);
        m_n.transform(n1, triangle.n1);
        m_n.transform(n2, triangle.n2);

        // In OpenGL this step can be turned on and off with GL_NORMALIZE, also there is GL_RESCALE_NORMAL which offers a faster way
        // which only works with uniform scales. For now this is constantly enabled because it is usually what someone want.
        n0.normalize();
        n1.normalize();
        n2.normalize();

        m_m.transform(v0, triangle.v0);
        m_m.transform(v1, triangle.v1);
        m_m.transform(v2, triangle.v2);

        calculateSceneLight(color0,
                            (m_enableColorMaterialEmission) ? triangle.color0 : m_material.emissiveColor,
                            (m_enableColorMaterialAmbient) ? triangle.color0 : m_material.ambientColor,
                            m_material.ambientColorScene);
        calculateSceneLight(color1,
                            (m_enableColorMaterialEmission) ? triangle.color1 : m_material.emissiveColor,
                            (m_enableColorMaterialAmbient) ? triangle.color1 : m_material.ambientColor,
                            m_material.ambientColorScene);
        calculateSceneLight(color2,
                            (m_enableColorMaterialEmission) ? triangle.color2 : m_material.emissiveColor,
                            (m_enableColorMaterialAmbient) ? triangle.color2 : m_material.ambientColor,
                            m_material.ambientColorScene);

        for (auto& light : m_lights)
        {
            if (!light.enable)
                continue;
            calculateLight(color0,
                           light,
                           m_material.specularExponent,
                           (m_enableColorMaterialAmbient) ? triangle.color0 : m_material.ambientColor,
                           (m_enableColorMaterialDiffuse) ? triangle.color0 : m_material.diffuseColor,
                           (m_enableColorMaterialSpecular) ? triangle.color0 : m_material.specularColor,
                           v0,
                           n0);
            calculateLight(color1,
                           light,
                           m_material.specularExponent,
                           (m_enableColorMaterialAmbient) ? triangle.color1 : m_material.ambientColor,
                           (m_enableColorMaterialDiffuse) ? triangle.color1 : m_material.diffuseColor,
                           (m_enableColorMaterialSpecular) ? triangle.color1 : m_material.specularColor,
                           v1,
                           n1);
            calculateLight(color2,
                           light,
                           m_material.specularExponent,
                           (m_enableColorMaterialAmbient) ? triangle.color2 : m_material.ambientColor,
                           (m_enableColorMaterialDiffuse) ? triangle.color2 : m_material.diffuseColor,
                           (m_enableColorMaterialSpecular) ? triangle.color2 : m_material.specularColor,
                           v2,
                           n2);
        }

        color0[3] = triangle.color0[3];
        color1[3] = triangle.color1[3];
        color2[3] = triangle.color2[3];
    }
    else
    {
        color0 = triangle.color0;
        color1 = triangle.color1;
        color2 = triangle.color2;
    }

    Clipper::ClipVertList vertList;
    Clipper::ClipStList stList;
    Clipper::ClipColorList colorList;
    Clipper::ClipVertList vertListBuffer;
    Clipper::ClipStList stListBuffer;
    Clipper::ClipColorList colorListBuffer;

    m_t.transform(vertList[0], triangle.v0);
    m_t.transform(vertList[1], triangle.v1);
    m_t.transform(vertList[2], triangle.v2);

    stList[0] = triangle.st0;
    stList[1] = triangle.st1;
    stList[2] = triangle.st2;

    colorList[0] = color0;
    colorList[1] = color1;
    colorList[2] = color2;

    calculateTexGenCoords(stList, triangle.v0, triangle.v1, triangle.v2);

    // Because if flat shading, the color doesn't have to be interpolated during clipping, so it can be ignored for now...
    auto [vertListSize, vertListClipped, stListClipped, colorListClipped] = Clipper::clip(vertList, vertListBuffer, stList, stListBuffer, colorList, colorListBuffer);

    // Calculate for every vertex the perspective division and also apply the viewport transformation
    for (uint8_t i = 0; i < vertListSize; i++)
    {
        perspectiveDivide(vertListClipped[i]);
        viewportTransform(vertListClipped[i]);
    }

    // Cull triangle
    if (m_enableCulling)
    {
        const float edgeVal = Rasterizer::edgeFunctionFloat(vertList[0], vertList[1], vertList[2]);
        const CullMode currentOrientation = (edgeVal <= 0.0f) ? CullMode::BACK : CullMode::FRONT;
        if (currentOrientation != m_cullMode)
            return true;
    }

    // Render the triangle
    for (uint8_t i = 3; i <= vertListSize; i++)
    {
        // For a triangle we need atleast 3 vertices. Also treat the clipped list from the clipping as a
        // triangle fan where vert zero is always the center of this fan
        const bool success = renderer.drawTriangle(vertListClipped[0],
                vertListClipped[i - 2],
                vertListClipped[i - 1],
                stListClipped[0],
                stListClipped[i - 2],
                stListClipped[i - 1],
                colorListClipped[0],
                colorListClipped[i - 2],
                colorListClipped[i - 1]);
        if (!success)
        {
            return false;
        }
    }
    return true;
}

void TnL::calculateTexGenCoords(Clipper::ClipStList &stList, const Vec4& v0, const Vec4& v1, const Vec4& v2) const
{
    if (m_texGenEnableS || m_texGenEnableT)
    {
        Vec4 v0Transformed;
        Vec4 v1Transformed;
        Vec4 v2Transformed;
        if ((m_texGenModeS == TexGenMode::EYE_LINEAR) || (m_texGenModeT == TexGenMode::EYE_LINEAR))
        {
            // TODO: We are transforming the vertexes twice, one time for the light and again here for the texture generation.
            // It would be convenient if we would do this only once.
            m_m.transform(v0Transformed, v0);
            m_m.transform(v1Transformed, v1);
            m_m.transform(v2Transformed, v2);
        }
        if (m_texGenEnableS)
        {
            switch (m_texGenModeS) {
            case TexGenMode::OBJECT_LINEAR:
                stList[0][0] = m_texGenVecObjS.dot(v0);
                stList[1][0] = m_texGenVecObjS.dot(v1);
                stList[2][0] = m_texGenVecObjS.dot(v2);
                break;
            case TexGenMode::EYE_LINEAR:
                stList[0][0] = m_texGenVecEyeS.dot(v0Transformed);
                stList[1][0] = m_texGenVecEyeS.dot(v1Transformed);
                stList[2][0] = m_texGenVecEyeS.dot(v2Transformed);
                break;
            case TexGenMode::SPHERE_MAP:
                // TODO: Implement
                break;
            default:
                break;
            }
        }
        if (m_texGenEnableT)
        {
            switch (m_texGenModeT) {
            case TexGenMode::OBJECT_LINEAR:
                stList[0][1] = m_texGenVecObjT.dot(v0);
                stList[1][1] = m_texGenVecObjT.dot(v1);
                stList[2][1] = m_texGenVecObjT.dot(v2);
                break;
            case TexGenMode::EYE_LINEAR:
                stList[0][1] = m_texGenVecEyeT.dot(v0Transformed);
                stList[1][1] = m_texGenVecEyeT.dot(v1Transformed);
                stList[2][1] = m_texGenVecEyeT.dot(v2Transformed);
                break;
            case TexGenMode::SPHERE_MAP:
                // TODO: Implement
                break;
            default:
                break;
            }
        }
    }
}

void TnL::calculateLight(Vec4 &color,
                         const LightConfig& lightConfig,
                         const float materialSpecularExponent,
                         const Vec4& materialAmbientColor,
                         const Vec4& materialDiffuseColor,
                         const Vec4& materialSpecularColor,
                         const Vec4& v0,
                         const Vec3& n0) const
{
    Vec4 n{{n0[0], n0[1], n0[2], 0}};

    if (lightConfig.enable)
    {
        // Calculate light from lights
        Vec4 dir;
        float att = 1.0f;

        // Calculate Diffuse Light
        // w unequal zero means: Point light
        // w equal to zero means: Directional light
        // It seems that mac os is interpolating between 0.0 and 1.0 so that the different
        // light sources can lerp to each other. We will not do this here.
        if (lightConfig.position[3] != 0.0f)
        {
            // Point light, is the normalized direction vector
            dir = lightConfig.position;
            dir -= v0;
            dir.normalize();

            const float dist = v0.dist(lightConfig.position);
            att = 1.0f / (lightConfig.constantAttenuation + (lightConfig.linearAttenuation * dist) + lightConfig.quadraticAttenuation * (dist * dist));
        }
        else
        {
            // Directional light, direction is the unit vector
            dir = lightConfig.preCalcDirectionalLightDir;
        }
        float dotDirDiffuse = n.dot(dir);
        dotDirDiffuse = (dotDirDiffuse < 0.01f) ? 0.0f : dotDirDiffuse;

        // Calculate specular light
        float f = 0.0f;
        if (dotDirDiffuse != 0.0f)
        {
            f = 1.0f;
        }

        // Convert now the direction in dir to the half way vector
        if (lightConfig.localViewer)
        {
            Vec4 dirEye{{0.0f, 0.0f, 0.0f, 1.0f}};
            dirEye -= v0;
            dirEye.normalize();
            dir += dirEye;
            dir.unit();
        }
        else
        {
            // Optimization: When position.w is equal to zero, then dirDiffuse is constant and
            // we can precompute with this constant value the half way vector.
            // Otherwise dirDiffuse depends on the vertex and no pre computation is possible
            if (lightConfig.position[3] != 0.0f)
            {
                const Vec4 pointEye{{0.0f, 0.0f, 1.0f, 1.0f}};
                dir += pointEye;
                dir.unit();
            }
            else
            {
                dir = lightConfig.preCalcHalfWayVectorInfinite;
            }
        }

        float dotDirSpecular = n.dot(dir);

        // Optimization: pows are expensive
        if (materialSpecularExponent == 0.0f) // x^0 == 1.0
        {
            dotDirSpecular = 1.0f;
        }
        else if (materialSpecularExponent != 1.0f) // x^1 == x
        {
            dotDirSpecular = powf(dotDirSpecular, materialSpecularExponent);
        }

        Vec4 ambientColor = lightConfig.ambientColor;
        ambientColor *= materialAmbientColor;
        Vec4 colorLight = lightConfig.diffuseColor;
        colorLight *= materialDiffuseColor;
        colorLight *= dotDirDiffuse;
        colorLight += ambientColor;
        Vec4 colorLightSpecular = lightConfig.specularColor;
        colorLightSpecular *= materialSpecularColor;
        colorLightSpecular *= (f * dotDirSpecular);
        colorLight += colorLightSpecular;

        colorLight *= att;
        // TODO: Spotlight has to be implemented. Please see in the OpenGL 1.5 spec equation 2.5.
        // Basically it is a val = dot(normalize(v0 - lightConfig.position), unit(lightConfig.spotlightDirectionLight))
        // spot = pow(val, lightConfig.spotlightExponentLight); when lightConfig.spotlightCutoffLight != 180 and val >= cos(lightConfig.spotlightCutoffLight)
        // spot = 0; when lightConfig.spotlightCutoffLight != 180 and val < cos(lightConfig.spotlightCutoffLight)
        // spot = 1; when lightConfig.spotlightCutoffLight == 180
        // colorLight0 *= spot;

        // Add light sums to final color
        color += colorLight;
    }
}


void TnL::viewportTransform(Vec4 &v0, Vec4 &v1, Vec4 &v2)
{
    v0[0] = ((v0[0] + 1.0f) * m_viewportWidth * 0.5f) + m_viewportX;
    v1[0] = ((v1[0] + 1.0f) * m_viewportWidth * 0.5f) + m_viewportX;
    v2[0] = ((v2[0] + 1.0f) * m_viewportWidth * 0.5f) + m_viewportX;

    v0[1] = ((v0[1] + 1.0f) * m_viewportHeight * 0.5f) + m_viewportY;
    v1[1] = ((v1[1] + 1.0f) * m_viewportHeight * 0.5f) + m_viewportY;
    v2[1] = ((v2[1] + 1.0f) * m_viewportHeight * 0.5f) + m_viewportY;

    v0[2] = (((v0[2] + 1.0f) * 0.25f)) * (m_depthRangeZFar - m_depthRangeZNear);
    v1[2] = (((v1[2] + 1.0f) * 0.25f)) * (m_depthRangeZFar - m_depthRangeZNear);
    v2[2] = (((v2[2] + 1.0f) * 0.25f)) * (m_depthRangeZFar - m_depthRangeZNear);

    // This is a possibility just to calculate the real z value but is not needed for the rasterizer
    //    float n = 0.1;
    //    float f = 100;
    //    float z_ndc0 = 2.0 * v0f[2] - 1.0;
    //    v0f[2] = 2.0 * n * f / (f + n - z_ndc0 * (f - n));
    //    float z_ndc1 = 2.0 * v1f[2] - 1.0;
    //    v1f[2] = 2.0 * n * f / (f + n - z_ndc1 * (f - n));
    //    float z_ndc2 = 2.0 * v2f[2] - 1.0;
    //    v2f[2] = 2.0 * n * f / (f + n - z_ndc2 * (f - n));

}

void TnL::viewportTransform(Vec4 &v)
{
    // v has the range from -1 to 1. When we multiply it, it has a range from -viewPortWidth/2 to viewPortWidth/2
    // With the addition we shift it from -viewPortWidth/2 to 0 + viewPortX
    v[0] = (((v[0] * m_viewportWidthHalf)) + m_viewportXShift);
    v[1] = (((v[1] * m_viewportHeightHalf)) + m_viewportYShift);
    // Alternative implementation which is basically doing the same but without precomputed variables
    // v[0] = (((v[0] + 1.0f) * m_viewportWidth * 0.5f) + m_viewportX);
    // v[1] = (((v[1] + 1.0f) * m_viewportHeight * 0.5f) + m_viewportY);
    v[2] = (((v[2] + 1.0f) * 0.25f)) * (m_depthRangeZFar - m_depthRangeZNear);
}

void TnL::perspectiveDivide(Vec4 &v)
{
    v[3] = 1.0f / v[3];
    v[0] = v[0] * v[3];
    v[1] = v[1] * v[3];
    v[2] = v[2] * v[3];
}

void TnL::calculateSceneLight(Vec4 &sceneLight, const Vec4& emissiveColor, const Vec4& ambientColor, const Vec4& ambientColorScene) const
{
    // Emission color of material
    sceneLight = emissiveColor;
    // Ambient Color Material and ambient scene color
    sceneLight += ambientColor * ambientColorScene;
}

void TnL::setViewport(const int16_t x, const int16_t y, const int16_t width, const int16_t height)
{
    // Note: The screen resolution is width and height. But during view port transformation we are clamping between
    // 0 and height which means a effective screen resolution of height + 1. For instance, we have a resolution of
    // 480 x 272. The view port transformation would go from 0 to 480 which are then 481px. Thats the reason why we
    // decrement here the resolution by one.
    m_viewportHeight = height - 1;
    m_viewportWidth = width - 1;
    m_viewportX = x;
    m_viewportY = y;

    m_viewportHeightHalf = m_viewportHeight / 2.0f;
    m_viewportWidthHalf = m_viewportWidth / 2.0f;
    m_viewportXShift = m_viewportX + m_viewportWidthHalf;
    m_viewportYShift = m_viewportY + m_viewportHeightHalf;

}

void TnL::setDepthRange(const float zNear, const float zFar)
{
    m_depthRangeZFar = zFar;
    m_depthRangeZNear = zNear;
}

void TnL::setModelProjectionMatrix(const Mat44 &m)
{
    m_t = m;
}

void TnL::setModelMatrix(const Mat44 &m)
{
    m_m = m;
}

void TnL::setNormalMatrix(const Mat44& m)
{
    m_n = m;
}

void TnL::enableLighting(bool enable)
{
    m_enableLighting = enable;
}

void TnL::enableLight(const uint8_t light, const bool enable)
{
    m_lights[light].enable = enable;
}

void TnL::setAmbientColorLight(const uint8_t light, const Vec4 &color)
{
    m_lights[light].ambientColor = color;
}

void TnL::setDiffuseColorLight(const uint8_t light, const Vec4 &color)
{
    m_lights[light].diffuseColor = color;
}

void TnL::setSpecularColorLight(const uint8_t light, const Vec4 &color)
{
    m_lights[light].specularColor = color;
}

void TnL::setPosLight(const uint8_t light, const Vec4 &pos)
{
    m_m.transform(m_lights[light].position, pos);
    m_lights[light].preCalcVectors();
}

void TnL::setConstantAttenuationLight(const uint8_t light, const float val)
{
    m_lights[light].constantAttenuation = val;
}

void TnL::setLinearAttenuationLight(const uint8_t light, const float val)
{
    m_lights[light].linearAttenuation = val;
}

void TnL::setQuadraticAttenuationLight(const uint8_t light, const float val)
{
    m_lights[light].quadraticAttenuation = val;
}

void TnL::enableColorMaterial(bool emission, bool ambient, bool diffuse, bool specular)
{
    m_enableColorMaterialEmission = emission;
    m_enableColorMaterialAmbient = ambient;
    m_enableColorMaterialDiffuse = diffuse;
    m_enableColorMaterialSpecular = specular;
}

void TnL::setEmissiveColorMaterial(const Vec4 &color)
{
    m_material.emissiveColor = color;
}

void TnL::setAmbientColorMaterial(const Vec4 &color)
{
    m_material.ambientColor = color;
}

void TnL::setAmbientColorScene(const Vec4 &color)
{
    m_material.ambientColorScene = color;
}

void TnL::setDiffuseColorMaterial(const Vec4 &color)
{
    m_material.diffuseColor = color;
}

void TnL::setSpecularColorMaterial(const Vec4 &color)
{
    m_material.specularColor = color;
}

void TnL::setSpecularExponentMaterial(const float val)
{
    m_material.specularExponent = val;
}

void TnL::enableTexGenS(bool enable)
{
    m_texGenEnableS = enable;
}

void TnL::enableTexGenT(bool enable)
{
    m_texGenEnableT = enable;
}

void TnL::setTexGenModeS(TexGenMode mode)
{
    m_texGenModeS = mode;
}

void TnL::setTexGenModeT(TexGenMode mode)
{
    m_texGenModeT = mode;
}

void TnL::setTexGenVecObjS(const Vec4 &val)
{
    m_texGenVecObjS = val;
}

void TnL::setTexGenVecObjT(const Vec4 &val)
{
    m_texGenVecObjT = val;
}

void TnL::setTexGenVecEyeS(const Vec4 &val)
{
    m_texGenVecEyeS = val;
}

void TnL::setTexGenVecEyeT(const Vec4 &val)
{
    m_texGenVecEyeT = val;
}

void TnL::setCullMode(TnL::CullMode mode)
{
    m_cullMode = mode;
}

void TnL::enableCulling(bool enable)
{
    m_enableCulling = enable;
}
