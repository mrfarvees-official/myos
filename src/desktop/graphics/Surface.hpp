#pragma once

#include <cstdint>
#include <vector>

namespace myos::graphics {
    class Surface {
        public:
            Surface(int witdth, int height);

            int width() const;
            int height() const;

            uint32_t* pixels();
            const uint32_t* pixels() const;

            void clear(uint32_t color);

        private:
            int width_;
            int height_;

            std::vector<uint32_t> pixels_;
    };
}