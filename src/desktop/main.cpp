#include "graphics/Color.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Surface.hpp"

#include <iostream>
#include <unistd.h>

using namespace myos::graphics;

int main()
{
    std::cout 
        << "=============================\n"
        << "         MYOS Desktop"
        << "=============================\n";

    Framebuffer framebuffer;

    if (!framebuffer.open()) {
        std::cerr << "[desktop] No usable framebuffer.\n";
        std::cerr << "[desktop] Check /dev/fb0 and kernel framebuffer support.\n";
        return 1;
    }

    Surface screen(framebuffer.width(), framebuffer.height());
    Renderer renderer(screen);

    // Desktop background
    renderer.clear(Color(24, 28, 36));

    // Fake first window
    renderer.fillRect(100, 100, 500, 300, Color(45, 50, 60));

    // Fake titlebar
    renderer.fillRect(100, 100, 500, 36, Color(65, 75, 95));

    // Taskbar
    renderer.fillRect(0, framebuffer.height() - 48, framebuffer.width(), 48, Color(18, 20, 26));

    if (!framebuffer.present(screen)) {
        std::cerr << "[desktop] Failed to present framebuffer.\n";
        return 1;
    }

    std::cout << "[desktop] First frame rendrered.\n";

    while (true) {
        sleep(1);
    }

    return 0;
}