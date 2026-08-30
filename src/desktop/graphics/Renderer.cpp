#include "graphics/Renderer.hpp"

#include <algorithm>

namespace myos::graphics {
    Renderer::Renderer(Surface &surface)
        : surface_(surface)
    {
    }

    void Renderer::clear(const Color &color)
    {
        surface_.clear(color.toARGB());
    }

    void Renderer::drawPixel(
        int x,
        int y,
        const Color &color
    )
    {
        if (x < 0 ||
            y < 0 ||
            x >= surface_.width() ||
            y >= surface_.height()
        ) {
            return;
        }

        const int index = y * surface_.width() + x;
        surface_.pixels()[index] = color.toARGB();
    }

    void Renderer::fillRect(
        int x,
        int y,
        int width, 
        int height,
        const Color &color
    ) {
        const int startX = std::max(0, x);
        const int startY = std::max(0, y);

        const int endX = std::min(surface_.width(), x + width);
        const int endY = std::min(surface_.height(), y + height);

        for (int py = startY; py < endY; ++py) {
            for (int px = startX; px < endX; ++px) {
                drawPixel(
                    px,
                    py,
                    color
                );
            }
        }
    }
}