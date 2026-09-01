#include "graphics/Color.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Surface.hpp"

#include "geometry/DirtyRegion.hpp"

#include "window/WindowManager.hpp"

#include "input/InputManager.hpp"
#include "input/Mouse.hpp"

#include <chrono>
#include <iostream>
#include <thread>

using namespace myos::graphics;

static Rect cursorBounds(
    Point position
)
{
    constexpr int cursorWidth =
        12;

    constexpr int cursorHeight =
        16;

    return Rect {
        position.x,
        position.y,
        cursorWidth,
        cursorHeight
    };
}

static void drawCursor(
    Renderer &renderer,
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

    static const char *cursorShape[] = {
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

    constexpr int cursorWidth =
        12;

    constexpr int cursorHeight =
        16;

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
            const char pixel =
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

    Renderer renderer(
        screen
    );

    WindowManager windowManager;

    Mouse mouse(
        framebuffer.width(),
        framebuffer.height()
    );

    InputManager inputManager(
        mouse
    );

    // Temporary fixed QEMU input paths.
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

    if (!inputManager.start())
    {
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

    using Clock =
        std::chrono::steady_clock;

    constexpr auto frameDuration =
        std::chrono::microseconds(
            16667
        );

    const Rect desktopBounds {
        0,
        0,
        framebuffer.width(),
        framebuffer.height()
    };

    DirtyRegion dirtyRegion(
        desktopBounds
    );

    // First frame must always render
    // the complete desktop.
    dirtyRegion.invalidateAll();

    Point lastMousePosition =
        mouse.position();

    while (true)
    {
        const auto frameStart =
            Clock::now();

        // -----------------------------------
        // Process queued input
        // -----------------------------------

        InputEvent event;

        while (
            inputManager.popEvent(event)
        ) {
            if (
                windowManager.handleEvent(
                    event
                )
            ) {
                // Temporary compatibility behavior.
                //
                // WindowManager currently only tells us
                // that visible state changed.
                //
                // It does not yet tell us exactly which
                // desktop rectangles changed.
                //
                // Precise window invalidation comes in
                // the next stage.
                dirtyRegion.invalidateAll();
            }
        }

        // -----------------------------------
        // Detect cursor movement
        // -----------------------------------

        const Point currentMousePosition =
            mouse.position();

        if (
            currentMousePosition.x !=
                lastMousePosition.x ||
            currentMousePosition.y !=
                lastMousePosition.y
        ) {
            // The old cursor area must be restored.
            dirtyRegion.invalidate(
                cursorBounds(
                    lastMousePosition
                )
            );

            // The cursor must be drawn in
            // its new location.
            dirtyRegion.invalidate(
                cursorBounds(
                    currentMousePosition
                )
            );

            lastMousePosition =
                currentMousePosition;
        }

        // -----------------------------------
        // Render only when something changed
        // -----------------------------------

        if (!dirtyRegion.empty())
        {
            // NOTE:
            //
            // We are tracking dirty regions now,
            // but rendering is intentionally still
            // full-screen at this stage.
            //
            // Partial rendering will be introduced
            // after window invalidation is correct.

            // Desktop background
            renderer.clear(
                Color(
                    24,
                    28,
                    36
                )
            );

            // Windows
            windowManager.render(
                renderer
            );

            // Taskbar
            renderer.fillRect(
                0,
                framebuffer.height() - 48,
                framebuffer.width(),
                48,
                Color(
                    18,
                    20,
                    26
                )
            );

            // Cursor must remain top-most.
            drawCursor(
                renderer,
                currentMousePosition
            );

            // Still full-surface presentation
            // for this stage.
            if (
                !framebuffer.present(
                    screen
                )
            ) {
                std::cerr
                    << "[desktop] Failed to present framebuffer.\n";

                inputManager.stop();

                return 1;
            }

            // Everything currently dirty has
            // now been represented on screen.
            dirtyRegion.clear();
        }

        // -----------------------------------
        // Frame pacing
        // -----------------------------------

        const auto elapsed =
            Clock::now() -
            frameStart;

        if (
            elapsed <
            frameDuration
        ) {
            std::this_thread::sleep_for(
                frameDuration -
                elapsed
            );
        }
    }

    return 0;
}