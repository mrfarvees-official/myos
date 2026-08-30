#pragma once

#include <cstdint>

namespace myos::graphics {
    struct Color {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t alpha;

        constexpr Color(
            uint8_t redValue,
            uint8_t greenValue,
            uint8_t blueValue,
            uint8_t alphaValue = 255
        ) 
        : red(redValue),
          green(greenValue),
          blue(blueValue),
          alpha(alphaValue)
        {
        }
        
        uint32_t toARGB() const
        {
            return 
                (static_cast<uint32_t>(alpha) << 24) |
                (static_cast<uint32_t>(red)   << 16) |
                (static_cast<uint32_t>(green) << 8)  |
                static_cast<uint32_t>(blue);
        }
    };
}