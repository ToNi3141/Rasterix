// Rasterix
// https://github.com/ToNi3141/Rasterix
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

#ifndef DISPLAYLISTDISPATCHER_HPP
#define DISPLAYLISTDISPATCHER_HPP

#include <stdint.h>
#include <tcb/span.hpp>

namespace rr::displaylist
{

template <typename RenderConfig, typename TDisplayListAssembler>
class DisplayListDispatcher
{
public:
    template <typename Command>
    bool addCommand(const std::size_t index, const Command& cmd)
    {
        return m_displayListAssembler[index].addCommand(cmd);
    }

    template <typename Command>
    bool addLastCommand(const Command& cmd)
    {
        return m_displayListAssembler[0].addCommand(cmd);
    }

    template <typename Command>
    bool addCommand(const Command& cmd)
    {
        bool ret = true;
        for (std::size_t i = 0; i < m_displayLines; i++)
        {
            ret = ret && addCommand(i, cmd);
        }
        return ret;
    }

    template <typename Factory>
    bool addCommandWithFactory(const Factory& commandFactory)
    {
        return addCommandWithFactory_if(commandFactory, [](std::size_t, std::size_t, std::size_t, std::size_t){ return true; });
    }

    template <typename Factory, typename Pred>
    bool addCommandWithFactory_if(const Factory& commandFactory, const Pred& pred)
    {
        bool ret = true;
        for (std::size_t i = 0; i < m_displayLines; i++)
        {
            if (pred(i, m_displayLines, m_xResolution, m_yLineResolution))
            {
                ret = ret && addCommand(i, commandFactory(i, m_displayLines, m_xResolution, m_yLineResolution));
            }
        }
        return ret;
    }

    template <typename Function>
    bool displayListLooper(const Function& func)
    {
        bool ret = true;
        for (std::size_t i = 0; i < m_displayLines; i++)
        {
            ret = ret && func(*this, i, m_displayLines, m_xResolution, m_yLineResolution);
        }
        return ret;
    }

    bool setResolution(const std::size_t x, const std::size_t y)
    {
        const std::size_t framebufferSize = x * y;
        const std::size_t framebufferLines = (framebufferSize / RenderConfig::FRAMEBUFFER_SIZE_IN_WORDS) + ((framebufferSize % RenderConfig::FRAMEBUFFER_SIZE_IN_WORDS) ? 1 : 0);
        if (framebufferLines > getDisplayLines())
        {
            // More lines required than lines available
            return false;
        }

        m_yLineResolution = y / framebufferLines;
        m_xResolution = x;
        m_displayLines = framebufferLines;
        return true;
    }

    std::size_t getYLineResolution() const 
    {
        return m_yLineResolution;
    }

    static constexpr bool singleList()
    {
        return getDisplayLines() == 1;
    }

    void beginFrame()
    {
        for (std::size_t i = 0; i < m_displayLines; i++)
        {
            m_displayListAssembler[i].begin();
        }
    }

    bool beginLastFrame()
    {
        return m_displayListAssembler[0].begin();
    }

    void endFrame()
    {
        for (std::size_t i = 0; i < m_displayLines; i++)
        {
            m_displayListAssembler[i].end();
        }
    }

    void endLastFrame()
    {
        m_displayListAssembler[0].end();
    }

    void saveSectionStart()
    {
        for (std::size_t i = 0; i < m_displayLines; i++)
        {
            m_displayListAssembler[i].saveSectionStart();
        }
    }

    void removeSection()
    {
        for (std::size_t i = 0; i < m_displayLines; i++)
        {
            m_displayListAssembler[i].removeSection();
        } 
    }

    void clearDisplayListAssembler()
    {
        for (int32_t i = m_displayLines - 1; i >= 0; i--)
        {
            m_displayListAssembler[i].clearAssembler();
        }
    }

    std::size_t getDisplayListBufferId(std::size_t displayList) const
    {
        return m_displayListAssembler[displayList].getDisplayListBufferId();
    }

    std::size_t getDisplayListSize(std::size_t displayList) const
    {
        return m_displayListAssembler[displayList].getDisplayListSize();
    }

    bool setBuffer(const std::size_t i, const tcb::span<uint8_t> buffer, const std::size_t bufferId)
    {
        if (i >= getDisplayLines())
            return false;
        m_displayListAssembler[i].setBuffer(buffer, bufferId);
        return true;
    }

private:
    static constexpr std::size_t getDisplayLines()
    {
        constexpr std::size_t MAX_FRAMEBUFFER_SIZE = RenderConfig::MAX_DISPLAY_WIDTH * RenderConfig::MAX_DISPLAY_HEIGHT;
        if  constexpr (MAX_FRAMEBUFFER_SIZE == RenderConfig::FRAMEBUFFER_SIZE_IN_WORDS)
        {
            return 1;
        }
        return (MAX_FRAMEBUFFER_SIZE / RenderConfig::FRAMEBUFFER_SIZE_IN_WORDS) + 1;
    }

    std::size_t m_yLineResolution { 128 };
    std::size_t m_xResolution { 640 };
    std::size_t m_displayLines { getDisplayLines() };
    std::array<TDisplayListAssembler, getDisplayLines()> m_displayListAssembler {};
};

} // namespace rr::displaylist
#endif // DISPLAYLISTDISPATCHER_HPP
