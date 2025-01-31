// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2024 ToNi3141

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

#include <future>

namespace rr
{

template <typename Renderer>
class ThreadedRenderer
{
public:
    ThreadedRenderer()
    {
        // Initialize the render thread by running it once
        m_renderThread = std::async(
            [&]()
            { return true; });
    }

    ~ThreadedRenderer()
    {
    }

    void setRenderer(Renderer* renderer)
    {
        m_renderer = renderer;
    }

    void waitForThread()
    {
        if (m_renderThread.valid() && (m_renderThread.get() != true))
        {
            return;
        }
    }

    void render()
    {
        m_renderer->swapDisplayList();
        m_renderThread = std::async(
            [this]()
            {
                m_renderer->uploadDisplayList();
                return true;
            });
    }

private:
    Renderer* m_renderer { nullptr };
    std::future<bool> m_renderThread;
};

} // namespace rr