#pragma once

class DisplayMetrics
{
public:
    DisplayMetrics(
        int width,
        int height
    )
        : m_width(width),
          m_height(height)
    {
    }

    int width() const
    {
        return m_width;
    }

    int height() const
    {
        return m_height;
    }

    int taskbarHeight() const
    {
        return 48;
    }

    int titleBarHeight() const
    {
        return 32;
    }

    int defaultSpacing() const
    {
        return 8;
    }

    int smallSpacing() const
    {
        return 4;
    }

    int largeSpacing() const
    {
        return 16;
    }

    float scale() const
    {
        return 1.0f;
    }

private:
    int m_width;
    int m_height;
};