#ifndef _STENCILSHADOW_HPP_
#define _STENCILSHADOW_HPP_
#include <span>
#include "glu.h"
#include "../../../lib/gl/Vec.hpp"
#include "../../../lib/gl/Mat44.hpp"

class StencilShadow
{
public:
    void init(const uint32_t resolutionW, const uint32_t resolutionH)
    {
        // Setup viewport, depth range, and projection matrix
        glViewport(0, 0, resolutionW, resolutionH);
        glDepthRange(0.0, 1.0);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(30.0, (float)resolutionW/(float)resolutionH, 1.0, 111.0);

        // Enable depth test and depth mask
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);

        // Setup light
        static constexpr GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 0.0 };
        static constexpr GLfloat light_diffuse[] = { 1.5, 1.5, 1.5, 1.0 };
        static constexpr GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };

        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 8.0f);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
        glLightfv(GL_LIGHT0, GL_POSITION, m_lightPosition.vec.data());
        glEnable(GL_LIGHT0);

        glActiveTexture(GL_TEXTURE0);
        static constexpr GLfloat colors[4] = {0.0, 0.0, 0.0, 0.0};
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colors);

        // Create Torus with adjacent vertex list
        createTorus(m_torusVertex, m_torusNormal, 0.5, 3, 6, 6);
        createAdjacentArray(m_torusAdjacencies, m_torusVertex);

        // Position camera
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(12.0f, -2.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    }

    void draw()
    {
        // Clear framebuffer
        glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f, 0.0f);
        glStencilMask(0xf);
        glClearStencil(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glStencilMask(0);

        glPushMatrix();
        {
            glColor3ub(111, 111, 111);
            drawGround();
        }
        glPopMatrix();
        glPushMatrix();
        {
            m_torusAngleY += 1.0f;
            m_torusAngleZ += 0.7f;
            glRotatef(m_torusAngleY, 0.0f, 1.0f, 0.0f);
            glRotatef(m_torusAngleZ, 0.0f, 0.0f, 1.0f);
            glScalef(0.5f, 0.5f, 0.5f);
            drawTorus();
            drawTorusSilhouetteInStencilBuffer();
        }
        glPopMatrix();
        enableStencilTest();
        glPushMatrix();
        {
            glColor3ub(255, 255, 255);
            drawGround();
        }
        glPopMatrix();
        disableStencilTest();
    }

private:
    void drawGround()
    {
        static const std::array<rr::Vec3, 4> quadVerts {{
            {{ -1.0f, 1.0f, -1.0f }},
            {{ -1.0f,-1.0f, -1.0f }},
            {{  1.0f,-1.0f, -1.0f }},
            {{  1.0f, 1.0f, -1.0f }},
        }};
        glScalef(8, 8, 8);
        glTranslatef(-1.5, 0, 0);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, quadVerts.data());
        glDrawArrays(GL_QUADS, 0, quadVerts.size());
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    void drawTorus()
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, m_torusNormal.data());
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, m_torusVertex.data());
        glDrawArrays(GL_TRIANGLES, 0, m_torusVertex.size());
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
    }

    void drawTorusSilhouetteInStencilBuffer()
    {
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // glColor4ub(111, 111, 111, 111);
        enableStencilBufferDrawing();
        drawSilhouette(m_torusAdjacencies, m_lightPosition);
        disableStencilBufferDrawing();
        // glDisable(GL_BLEND);
    }

    void enableStencilBufferDrawing()
    {
        glDepthMask(0);
        glColorMask(0, 0, 0, 0);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);

        glActiveStencilFaceEXT(GL_BACK);
        glStencilOp(GL_KEEP,            // stencil test fail
                   GL_KEEP,            // depth test fail
                   GL_DECR_WRAP_EXT);  // depth test pass
        glStencilMask(0xf);
        glStencilFunc(GL_ALWAYS, 0, 0xf);

        glActiveStencilFaceEXT(GL_FRONT);
        glStencilOp(GL_KEEP,            // stencil test fail
                   GL_KEEP,            // depth test fail
                   GL_INCR_WRAP_EXT);  // depth test pass
        glStencilMask(0xf);
        glStencilFunc(GL_ALWAYS, 0, 0xf);
    }

    void disableStencilBufferDrawing()
    {
        glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
        glDisable(GL_STENCIL_TEST);
        glDepthMask(1);
        glColorMask(1,1,1,1);
        glStencilMask(0x0);
    }

    void enableStencilTest()
    {
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_EQUAL, 0, 0xf);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    }

    void disableStencilTest()
    {
        glDisable(GL_STENCIL_TEST);
    }

    void searchAndAddEdge(rr::Vec3* adjacent, const std::span<const rr::Vec3> verts, const rr::Vec3& ev0, const rr::Vec3& ev1, const uint32_t i)
    {
        for (uint32_t j = 0; j < verts.size(); j += 3)
        {
            if (i == j) // Avoid to add the own edge to the adjacent list
                continue;
            
            const rr::Vec3 av0 = verts[j];
            const rr::Vec3 av1 = verts[j + 1];
            const rr::Vec3 av2 = verts[j + 2];

            int found = -1;

            if ((ev0 == av0) && (ev1 == av1))
            {
                found = 2;
            }

            if ((ev0 == av1) && (ev1 == av2))
            {
                found = 0;
            }

            if ((ev0 == av2) && (ev1 == av0))
            {
                found = 1;
            }

            if (found != -1)
            {
                adjacent[0] = verts[j + found];
                return;
            }
        }
        adjacent[0] = verts[i];
        return;
    }

    void createAdjacentArray(std::span<rr::Vec3> adjacent, const std::span<const rr::Vec3> verts)
    {
        uint32_t j = 0;
        for (uint32_t i = 0; i < verts.size(); i += 3)
        {
            const rr::Vec3 v0 = verts[i];
            const rr::Vec3 v1 = verts[i + 1];
            const rr::Vec3 v2 = verts[i + 2];

            adjacent[(j * 6) + 0] = v0;
            adjacent[(j * 6) + 1] = v1;
            adjacent[(j * 6) + 2] = v2;

            searchAndAddEdge(&(adjacent[(j * 6) + 3]), verts, v1, v0, i); // switch direction from v0->v1 to v1->v0 because the other triangle will have the other direction
            searchAndAddEdge(&(adjacent[(j * 6) + 4]), verts, v2, v1, i);
            searchAndAddEdge(&(adjacent[(j * 6) + 5]), verts, v0, v2, i);

            j++;
        }
    }

    void emitQuad(const rr::Vec3& start, const rr::Vec3& end, const rr::Vec3& lightPos)
    {
        static constexpr float EPSILON { 0.0001F };

        rr::Vec3 lightDir = start - lightPos;
        lightDir.normalize();
        const rr::Vec3 pos2 = lightDir;
        lightDir *= EPSILON;
        rr::Vec3 pos1 = start + lightDir;


        lightDir = end - lightPos;
        lightDir.normalize();
        const rr::Vec3 pos4 = lightDir;
        lightDir *= EPSILON;
        rr::Vec3 pos3 = end + lightDir;

        const std::array<rr::Vec4, 4> quadVerts {{
            {{ pos1[0], pos1[1], pos1[2], 1.0f }},
            {{ pos2[0], pos2[1], pos2[2], 0.0f }},
            {{ pos4[0], pos4[1], pos4[2], 0.0f }},
            {{ pos3[0], pos3[1], pos3[2], 1.0f }},
        }};
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(4, GL_FLOAT, 0, quadVerts.data());
        glDrawArrays(GL_QUADS, 0, quadVerts.size());
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    void drawSilhouette(const std::span<const rr::Vec3> verts, const rr::Vec4& lightPos)
    {
        rr::Mat44 mInv;
        glGetFloatv(GL_MODELVIEW_MATRIX, &mInv.mat[0][0]);
        mInv.invert();
        rr::Vec4 tmpLightPos;
        mInv.transform(tmpLightPos, lightPos);
        rr::Vec3 tLightPos { tmpLightPos.vec.data() };
        for (uint32_t i = 0; i < verts.size() / 6; i++)
        {
            const rr::Vec3* line = &(verts[i * 6]);
            rr::Vec3 e1 = line[1] - line[0];
            rr::Vec3 e2 = line[2] - line[0];
            rr::Vec3 e3 = line[3] - line[0];
            rr::Vec3 e4 = line[4] - line[1];
            rr::Vec3 e5 = line[2] - line[1];
            rr::Vec3 e6 = line[5] - line[0];

            rr::Vec3 normal = e1;
            normal.cross(e2);
            rr::Vec3 lightDir = tLightPos - line[0];

            if (normal.dot(lightDir) > 0.00001)
            {
                normal = e3;
                normal.cross(e1);
                if (normal.dot(lightDir) <= 0)
                {
                    emitQuad(line[0], line[1], tLightPos);
                }

                normal = e4;
                normal.cross(e5);
                lightDir = tLightPos - line[1];
                if (normal.dot(lightDir) <= 0)
                {
                    emitQuad(line[1], line[2], tLightPos);
                }
                
                normal = e2;
                normal.cross(e6);
                lightDir = tLightPos - line[2];
                if (normal.dot(lightDir) <= 0)
                {
                    emitQuad(line[2], line[0], tLightPos);
                }
            }
        }
    }

    void mglut_sincos(float angle, float *sptr, float *cptr)
    {
        *sptr = sin(angle);
        *cptr = cos(angle);
    }

    void createTorus(std::span<rr::Vec3> vertex, std::span<rr::Vec3> normal, float inner_rad, float outer_rad, int sides, int rings)
    {
        static constexpr float __glPi = 3.14159265358979323846f;
        int i, j, k, gray;
        float x, y, z, s, t, u, v, phi, theta, sintheta, costheta, sinphi, cosphi;
        float du = 1.0f / (float)rings;
        float dv = 1.0f / (float)sides;

        std::array<rr::Vec3, 4> quadVerts;
        std::array<rr::Vec3, 4> quadNorm;

        uint32_t ptrIndex = 0;
        for(i = 0; i < rings; i++) 
        {
            u = i * du;
            for(j = 0; j < sides; j++) 
            {
                v = j * dv;
                for(k = 0; k < 4; k++) 
                {
                    gray = k ^ (k >> 1);
                    s = gray & 1 ? u + du : u;
                    t = gray & 2 ? v + dv : v;
                    theta = s * __glPi * 2.0f;
                    phi = t * __glPi * 2.0f;
                    mglut_sincos(theta, &sintheta, &costheta);
                    mglut_sincos(phi, &sinphi, &cosphi);
                    x = sintheta * sinphi;
                    y = costheta * sinphi;
                    z = cosphi;
                    quadNorm[k] = rr::Vec3{{x, y, z}};

                    x = x * inner_rad + sintheta * outer_rad;
                    y = y * inner_rad + costheta * outer_rad;
                    z *= inner_rad;
                    quadVerts[k] = rr::Vec3{{x, y, z}};
                }
                normal[ptrIndex] = quadNorm[0];
                vertex[ptrIndex] = quadVerts[0];
                ptrIndex++;
                normal[ptrIndex] = quadNorm[1];
                vertex[ptrIndex] = quadVerts[1];
                ptrIndex++;
                normal[ptrIndex] = quadNorm[2];
                vertex[ptrIndex] = quadVerts[2];
                ptrIndex++;

                normal[ptrIndex] = quadNorm[0];
                vertex[ptrIndex] = quadVerts[0];
                ptrIndex++;
                normal[ptrIndex] = quadNorm[2];
                vertex[ptrIndex] = quadVerts[2];
                ptrIndex++;
                normal[ptrIndex] = quadNorm[3];
                vertex[ptrIndex] = quadVerts[3];
                ptrIndex++;
            }
        }
    }

    rr::Vec4 m_lightPosition {{ 1.0, 3.0, 6.0, 0.0 }};

    std::array<rr::Vec3, 6 * 6 * 6> m_torusNormal;
    std::array<rr::Vec3, 6 * 6 * 6> m_torusVertex;
    std::array<rr::Vec3, 6 * 6 * 12> m_torusAdjacencies;

    float m_torusAngleY { 0.0f };
    float m_torusAngleZ { 0.0f };
};

#endif // _STENCILSHADOW_HPP_
