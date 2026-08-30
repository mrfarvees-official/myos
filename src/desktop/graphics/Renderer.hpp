#pragma once

#include "graphics/Color.hpp"
#include "graphics/Surface.hpp"

namespace myos::graphics {
    class Renderer {
        public:
            explicit Renderer(Surface &surface);

            void clear(const Color &color);

            void drawPixel(
                int x,
                int y,
                const Color &color
            );

            void fillRect(
                int x,
                int y,
                int width, 
                int height,
                const Color &color
            );

        private:
            Surface &surface_;
    };
}