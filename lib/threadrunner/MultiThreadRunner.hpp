// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

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

#ifndef MULTITHREADRUNNER_HPP
#define MULTITHREADRUNNER_HPP

#include "IThreadRunner.hpp"
#include <future>

namespace rr
{

class MultiThreadRunner : public IThreadRunner
{
public:
    MultiThreadRunner()
    {
        // Initialize the render thread by running it once
        m_renderThread = std::async(
            [&]()
            { return true; });
    }

    void wait() override
    {
        if (m_renderThread.valid())
        {
            m_renderThread.get();
        }
    }

    void run(const std::function<bool()>& operation) override
    {
        m_renderThread = std::async(operation);
    }

private:
    std::future<bool> m_renderThread;
};

} // namespace rr

#endif // MULTITHREADRUNNER_HPP