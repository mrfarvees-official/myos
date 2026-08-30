#pragma once

#include "graphics/Surface.hpp"

#include <cstddef>
#include <cstdint>

namespace myos::graphics {
    class Framebuffer {
        public:
            Framebuffer();
            ~Framebuffer();

            bool open();

            int width() const;
            int height() const;
            int bitsPerPixel() const;

            bool present(const Surface &surface);

        private:
            int fd_;
            uint8_t *memory_;
            std::size_t memorySize_;

            int width_;
            int height_;
            int bitsPerPixel_;
            int lineLength_;
    };
}