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

#include "IceGL.hpp"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

IceGL* instance { nullptr };

IceGL& IceGL::getInstance()
{
    return *instance;
}

bool IceGL::createInstance(IRenderer& renderer)
{
    if (instance)
    {
        delete instance;
    }
    instance = new IceGL { renderer };
    return instance != nullptr;
}

IceGL::IceGL(IRenderer &renderer)
    : m_renderer(renderer)
    , m_pixelPipeline(renderer)
    , m_vertexPipeline(m_pixelPipeline)
{
    // Preallocate the first texture. This is the default texture which also can't be deleted.
    m_renderer.createTexture();
}

void IceGL::commit()
{
    m_renderer.commit();
}
