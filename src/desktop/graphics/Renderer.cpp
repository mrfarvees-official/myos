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
        const Color& color
    )
    {
        if (width <= 0 || height <= 0) {
            return;
        }

        const int surfaceWidth = surface_.width();
        const int surfaceHeight = surface_.height();

        const int startX = std::max(0, x);
        const int startY = std::max(0, y);

        const int endX = std::min(
            surfaceWidth,
            x + width
        );

        const int endY = std::min(
            surfaceHeight,
            y + height
        );

        if (
            startX >= endX ||
            startY >= endY
        ) {
            return;
        }

        uint32_t* pixels = surface_.pixels();

        const uint32_t pixelColor =
            color.toARGB();

        const int rowWidth =
            endX - startX;

        for (
            int py = startY;
            py < endY;
            ++py
        ) {
            uint32_t* row =
                pixels +
                static_cast<std::size_t>(py) *
                    static_cast<std::size_t>(surfaceWidth) +
                static_cast<std::size_t>(startX);

            std::fill_n(
                row,
                rowWidth,
                pixelColor
            );
        }
    }
}