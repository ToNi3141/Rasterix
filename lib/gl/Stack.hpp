#include <array>

template <typename T, std::size_t DEPTH>
class Stack
{
public:
    bool push(const T& v)
    {
        if (m_stackPointer < m_stack.size())
        {
            m_stack[m_stackPointer] = v;
            m_stackPointer++;
            return true;
        }
        return false;
    }

    bool pop(T& v)
    {
        if (m_stackPointer > 0)
        {
            m_stackPointer--;
            v = m_stack[m_stackPointer];
            return true;
        }
        return false;
    }
private:
    std::size_t m_stackPointer { 0 };
    std::array<T, DEPTH> m_stack {};
};