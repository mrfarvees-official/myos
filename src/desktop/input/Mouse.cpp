#include "Mouse.hpp"

#include <algorithm>

Mouse::Mouse(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth),
      m_screenHeight(screenHeight)
{
    m_position = Point {
        screenWidth / 2,
        screenHeight / 2
    };
}

void Mouse::moveRelative(int deltaX, int deltaY)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_position.x += deltaX;
    m_position.y += deltaY;

    m_position.x = std::clamp(m_position.x, 0, m_screenWidth - 1);
    m_position.y = std::clamp(m_position.y, 0, m_screenHeight - 1);
}

Point Mouse::position() const 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_position;
}