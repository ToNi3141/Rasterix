#include "Lighting.hpp"

Lighting::Lighting()
{
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

void Lighting::calculateLights(Vec4& color,
                               const Vec4& triangleColor,
                               const Vec4& vertex,
                               const Vec3& normal) 
{
    if (m_lightingEnabled)
    {
        Vec4 colorTmp;
        calculateSceneLight(colorTmp,
                            (m_enableColorMaterialEmission) ? triangleColor : m_material.emissiveColor,
                            (m_enableColorMaterialAmbient) ? triangleColor : m_material.ambientColor,
                            m_material.ambientColorScene);

        for (auto& light : m_lights)
        {
            if (!light.enable)
                continue;
            calculateLight(colorTmp,
                           light,
                           m_material.specularExponent,
                           (m_enableColorMaterialAmbient) ? triangleColor : m_material.ambientColor,
                           (m_enableColorMaterialDiffuse) ? triangleColor : m_material.diffuseColor,
                           (m_enableColorMaterialSpecular) ? triangleColor : m_material.specularColor,
                           vertex,
                           normal);
        }

        color = colorTmp;
        color[3] = triangleColor[3];
    }
}

void Lighting::calculateLight(Vec4 &color,
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

void Lighting::calculateSceneLight(Vec4 &sceneLight, const Vec4& emissiveColor, const Vec4& ambientColor, const Vec4& ambientColorScene) const
{
    // Emission color of material
    sceneLight = emissiveColor;
    // Ambient Color Material and ambient scene color
    sceneLight += ambientColor * ambientColorScene;
}

void Lighting::enableLighting(bool enable)
{
    m_lightingEnabled = enable;
}

bool Lighting::lightingEnabled() const 
{
    return m_lightingEnabled;
}

void Lighting::enableLight(const uint8_t light, const bool enable)
{
    m_lights[light].enable = enable;
}

void Lighting::setAmbientColorLight(const uint8_t light, const Vec4 &color)
{
    m_lights[light].ambientColor = color;
}

void Lighting::setDiffuseColorLight(const uint8_t light, const Vec4 &color)
{
    m_lights[light].diffuseColor = color;
}

void Lighting::setSpecularColorLight(const uint8_t light, const Vec4 &color)
{
    m_lights[light].specularColor = color;
}

void Lighting::setPosLight(const uint8_t light, const Vec4 &pos)
{
    m_lights[light].position = pos;
    m_lights[light].preCalcVectors();
}

void Lighting::setConstantAttenuationLight(const uint8_t light, const float val)
{
    m_lights[light].constantAttenuation = val;
}

void Lighting::setLinearAttenuationLight(const uint8_t light, const float val)
{
    m_lights[light].linearAttenuation = val;
}

void Lighting::setQuadraticAttenuationLight(const uint8_t light, const float val)
{
    m_lights[light].quadraticAttenuation = val;
}

void Lighting::enableColorMaterial(bool emission, bool ambient, bool diffuse, bool specular)
{
    m_enableColorMaterialEmission = emission;
    m_enableColorMaterialAmbient = ambient;
    m_enableColorMaterialDiffuse = diffuse;
    m_enableColorMaterialSpecular = specular;
}

void Lighting::setEmissiveColorMaterial(const Vec4 &color)
{
    m_material.emissiveColor = color;
}

void Lighting::setAmbientColorMaterial(const Vec4 &color)
{
    m_material.ambientColor = color;
}

void Lighting::setAmbientColorScene(const Vec4 &color)
{
    m_material.ambientColorScene = color;
}

void Lighting::setDiffuseColorMaterial(const Vec4 &color)
{
    m_material.diffuseColor = color;
}

void Lighting::setSpecularColorMaterial(const Vec4 &color)
{
    m_material.specularColor = color;
}

void Lighting::setSpecularExponentMaterial(const float val)
{
    m_material.specularExponent = val;
}
