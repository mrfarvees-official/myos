#include "graphics/Surface.hpp"

#include <algorithm>

namespace myos::graphics {
    Surface::Surface(int width, int height)
        : width_(width),
          height_(height),
          pixels_(
            static_cast<std::size_t>(width) *
            static_cast<std::size_t>(height),
            0
          )
    {
    }

    int Surface::width() const
    {
        return width_;
    }

    int Surface::height() const
    {
        return height_;
    }

    uint32_t *Surface::pixels()
    {
        return pixels_.data();
    }

    const uint32_t *Surface::pixels() const
    {
        return pixels_.data();
    }

    void Surface::clear(uint32_t color)
    {
        std::fill(pixels_.begin(), pixels_.end(), color);
    }
}