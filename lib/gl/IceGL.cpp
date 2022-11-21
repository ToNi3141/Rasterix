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
#include <spdlog/spdlog.h>
#include "spdlog/sinks/basic_file_sink.h"

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
    static auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("basic_logger", "basic-log.txt");
    file_sink->set_level(spdlog::level::trace);
    auto logger = std::make_shared<spdlog::logger>("basic_logger", file_sink);
    logger->set_level(spdlog::level::trace);
    logger->info("IceGL started");

    // or you can even set multi_sink logger as default logger
    spdlog::set_default_logger(logger);

}

void IceGL::commit()
{
    SPDLOG_INFO("Commit called");
    m_renderer.commit();
}
