#include "graphics/Color.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Surface.hpp"

#include "window/WindowManager.hpp"
#include "input/InputManager.hpp"

#include <iostream>
#include <unistd.h>

using namespace myos::graphics;

int main()
{
    std::cout
        << "=============================\n"
        << "         MYOS Desktop\n"
        << "=============================\n";

    Framebuffer framebuffer;

    
    if (!framebuffer.open())
    {
        std::cerr << "[desktop] No usable framebuffer.\n";
        std::cerr << "[desktop] Check /dev/fb0 and kernel framebuffer support.\n";
        return 1;
    }

    Surface screen(
        framebuffer.width(),
        framebuffer.height()
    );

    Renderer renderer(screen);

    WindowManager windowManager;

    InputManager inputManager(
        framebuffer.width(),
        framebuffer.height()
    );

    // Temporary device paths.
    // We will verify the correct event numbers next.
    if (!inputManager.openMouse("/dev/input/event1")) {
        std::cerr << "[desktop] Failed to open mouse input.\n";
    }

    if (!inputManager.openKeyboard("/dev/input/event0")) {
        std::cerr << "[desktop] Failed to open keyboard input.\n";
    }

    windowManager.createWindow(
        "Terminal",
        Rect{
            80,
            70,
            420,
            280
        }
    );

    windowManager.createWindow(
        "Files",
        Rect{
            220,
            140,
            420,
            300
        }
    );

    windowManager.createWindow(
        "Settings",
        Rect{
            380,
            100,
            360,
            260
        }
    );

    std::cout << "[desktop] Entering desktop loop.\n";

    while (true) {
        // 1. Process input events
        InputEvent event;

        while (inputManager.pollEvent(event)) {
            windowManager.handleEvent(event);
        }

        // 2. Clear desktop
        renderer.clear(Color(24, 28, 36));

        // 3. Render windows
        windowManager.render(renderer);

        // 4. Render taskbar
        renderer.fillRect(
            0,
            framebuffer.height() - 48,
            framebuffer.width(),
            48, 
            Color(18, 20, 26)
        );

        //5. Present completed frame
        if (!framebuffer.present(screen)) {
            std::cerr << "[desktop] Failed to present framebuffer.\n";
            return 1;
        }

        // Temporary ~60 FSP loop
        usleep(16000);
    }

    return 0;
}
