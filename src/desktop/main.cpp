#include "graphics/Color.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Surface.hpp"

#include "window/WindowManager.hpp"

#include "input/InputManager.hpp"
#include "input/Mouse.hpp"

#include <iostream>
#include <unistd.h>

using namespace myos::graphics;

static void drawCursor(
    Renderer& renderer,
    Point position
)
{
    const Color white(
        255,
        255,
        255
    );

    const Color black(
        0,
        0,
        0
    );

    static const char* cursorShape[] = {
        "X...........",
        "XX..........",
        "XOX.........",
        "XOOX........",
        "XOOOX.......",
        "XOOOOX......",
        "XOOOOOX.....",
        "XOOOOOOX....",
        "XOOOOOOOX...",
        "XOOOOXXXXX..",
        "XOOXOX......",
        "XOXX.OX.....",
        "XX...OX.....",
        "X.....OX....",
        "......OX....",
        ".......X...."
    };

    constexpr int cursorWidth = 12;
    constexpr int cursorHeight = 16;

    for (
        int y = 0;
        y < cursorHeight;
        ++y
    ) {
        for (
            int x = 0;
            x < cursorWidth;
            ++x
        ) {
            char pixel =
                cursorShape[y][x];

            if (pixel == 'X') {
                renderer.drawPixel(
                    position.x + x,
                    position.y + y,
                    black
                );
            }
            else if (pixel == 'O') {
                renderer.drawPixel(
                    position.x + x,
                    position.y + y,
                    white
                );
            }
        }
    }
}

int main()
{
    std::cout
        << "=============================\n"
        << "         MYOS Desktop\n"
        << "=============================\n";

    Framebuffer framebuffer;

    if (!framebuffer.open())
    {
        std::cerr
            << "[desktop] No usable framebuffer.\n";

        std::cerr
            << "[desktop] Check /dev/fb0 and kernel framebuffer support.\n";

        return 1;
    }

    Surface screen(
        framebuffer.width(),
        framebuffer.height()
    );

    Renderer renderer(screen);

    WindowManager windowManager;

    Mouse mouse(
        framebuffer.width(),
        framebuffer.height()
    );

    InputManager inputManager(mouse);

    // Temporary QEMU paths.
    if (
        !inputManager.openKeyboard(
            "/dev/input/event1"
        )
    ) {
        std::cerr
            << "[desktop] Failed to open keyboard input.\n";
    }

    if (
        !inputManager.openMouse(
            "/dev/input/event2"
        )
    ) {
        std::cerr
            << "[desktop] Failed to open mouse input.\n";
    }

    if (!inputManager.start()) {
        std::cerr
            << "[desktop] Failed to start input manager.\n";
    }

    windowManager.createWindow(
        "Terminal",
        Rect {
            80,
            70,
            420,
            280
        }
    );

    windowManager.createWindow(
        "Files",
        Rect {
            220,
            140,
            420,
            300
        }
    );

    windowManager.createWindow(
        "Settings",
        Rect {
            380,
            100,
            360,
            260
        }
    );

    std::cout
        << "[desktop] Entering desktop loop.\n";

    while (true)
    {
        // -------------------------
        // Input
        // -------------------------

        InputEvent event;

        while (
            inputManager.popEvent(event)
        ) {
            windowManager.handleEvent(event);
        }

        // -------------------------
        // Desktop background
        // -------------------------

        renderer.clear(
            Color(24, 28, 36)
        );

        // -------------------------
        // Windows
        // -------------------------

        windowManager.render(
            renderer
        );

        // -------------------------
        // Taskbar
        // -------------------------

        renderer.fillRect(
            0,
            framebuffer.height() - 48,
            framebuffer.width(),
            48,
            Color(18, 20, 26)
        );

        // -------------------------
        // Mouse cursor
        // -------------------------

        drawCursor(
            renderer,
            mouse.position()
        );

        // -------------------------
        // Present
        // -------------------------

        if (
            !framebuffer.present(screen)
        ) {
            std::cerr
                << "[desktop] Failed to present framebuffer.\n";

            inputManager.stop();

            return 1;
        }

        usleep(16000);
    }

    return 0;
}