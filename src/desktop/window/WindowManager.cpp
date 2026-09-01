#include "WindowManager.hpp"

#include <algorithm>

#include "../display/DisplayMetrics.hpp"
#include "../graphics/Renderer.hpp"

WindowManager::WindowManager(
    const DisplayMetrics &displayMetrics
)
    : m_displayMetrics(displayMetrics)
{
}

Window &WindowManager::createWindow(
    const std::string &title,
    const Rect &bounds
)
{
    const int id =
        m_nextWindowId++;

    auto window =
        std::make_unique<Window>(
            id,
            title,
            bounds,
            m_displayMetrics
        );

    Window &reference =
        *window;

    m_windows.push_back(
        std::move(window)
    );

    focusWindow(
        id
    );

    return reference;
}

void WindowManager::focusWindow(
    int windowId
)
{
    auto it = std::find_if(
        m_windows.begin(),
        m_windows.end(),
        [windowId](
            const std::unique_ptr<Window> &window
        ) {
            return window->id() ==
                windowId;
        }
    );

    if (
        it ==
        m_windows.end()
    ) {
        return;
    }

    for (
        auto &window :
        m_windows
    ) {
        window->setFocused(
            false
        );
    }

    (*it)->setFocused(
        true
    );

    m_focusedWindowId =
        windowId;

    auto window =
        std::move(
            *it
        );

    m_windows.erase(
        it
    );

    m_windows.push_back(
        std::move(window)
    );
}

Window *WindowManager::focusedWindow()
{
    for (
        auto &window :
        m_windows
    ) {
        if (
            window->id() ==
            m_focusedWindowId
        ) {
            return window.get();
        }
    }

    return nullptr;
}

const Window *WindowManager::focusedWindow() const
{
    for (
        const auto &window :
        m_windows
    ) {
        if (
            window->id() ==
            m_focusedWindowId
        ) {
            return window.get();
        }
    }

    return nullptr;
}

WindowUpdate WindowManager::handleEvent(
    const InputEvent &event
)
{
    WindowUpdate update;

    // -----------------------------------
    // Alt + Tab
    // -----------------------------------

    if (
        event.type ==
            InputEventType::KeyDown &&
        event.alt &&
        event.key ==
            Key::Tab
    ) {
        if (
            m_windows.size() <
            2
        ) {
            return update;
        }

        const Window *previousFocused =
            focusedWindow();

        Rect previousBounds {};

        bool hadPrevious =
            false;

        if (
            previousFocused !=
            nullptr
        ) {
            previousBounds =
                previousFocused->bounds();

            hadPrevious =
                true;
        }

        focusNextWindow();

        const Window *newFocused =
            focusedWindow();

        if (
            hadPrevious
        ) {
            update.invalidate(
                previousBounds
            );
        }

        if (
            newFocused !=
            nullptr
        ) {
            update.invalidate(
                newFocused->bounds()
            );
        }

        return update;
    }

    // -----------------------------------
    // Left mouse button pressed
    // -----------------------------------

    if (
        event.type ==
            InputEventType::MouseButtonDown &&
        event.mouseButton ==
            MouseButton::Left
    ) {
        Window *window =
            windowAt(
                event.mousePosition
            );

        if (
            window ==
            nullptr
        ) {
            return update;
        }

        const int selectedWindowId =
            window->id();

        const int previousFocusedWindowId =
            m_focusedWindowId;

        Rect previousFocusedBounds {};

        bool hadPreviousFocused =
            false;

        const Window *previousFocused =
            focusedWindow();

        if (
            previousFocused !=
            nullptr
        ) {
            previousFocusedBounds =
                previousFocused->bounds();

            hadPreviousFocused =
                true;
        }

        const Rect selectedBounds =
            window->bounds();

        focusWindow(
            selectedWindowId
        );

        const bool focusChanged =
            previousFocusedWindowId !=
            m_focusedWindowId;

        if (
            focusChanged
        ) {
            if (
                hadPreviousFocused
            ) {
                update.invalidate(
                    previousFocusedBounds
                );
            }

            update.invalidate(
                selectedBounds
            );
        }

        window =
            focusedWindow();

        if (
            window ==
            nullptr
        ) {
            return update;
        }

        if (
            window->titleBarContains(
                event.mousePosition
            )
        ) {
            m_draggedWindo =
                window;

            const Rect &bounds =
                window->bounds();

            m_dragOffset =
                Point {
                    event.mousePosition.x -
                        bounds.x,

                    event.mousePosition.y -
                        bounds.y
                };
        }

        return update;
    }

    // -----------------------------------
    // Mouse moving while dragging
    // -----------------------------------

    if (
        event.type ==
            InputEventType::MouseMove &&
        m_draggedWindo !=
            nullptr
    ) {
        const int newX =
            event.mousePosition.x -
            m_dragOffset.x;

        const int newY =
            event.mousePosition.y -
            m_dragOffset.y;

        const Rect oldBounds =
            m_draggedWindo->bounds();

        if (
            oldBounds.x ==
                newX &&
            oldBounds.y ==
                newY
        ) {
            return update;
        }

        m_draggedWindo->setPosition(
            newX,
            newY
        );

        const Rect newBounds =
            m_draggedWindo->bounds();

        update.invalidate(
            oldBounds
        );

        update.invalidate(
            newBounds
        );

        return update;
    }

    // -----------------------------------
    // Left mouse button released
    // -----------------------------------

    if (
        event.type ==
            InputEventType::MouseButtonUp &&
        event.mouseButton ==
            MouseButton::Left
    ) {
        m_draggedWindo =
            nullptr;

        return update;
    }

    return update;
}

Window *WindowManager::windowAt(
    Point position
)
{
    for (
        auto it =
            m_windows.rbegin();
        it !=
            m_windows.rend();
        ++it
    ) {
        if (
            (*it)->contains(
                position
            )
        ) {
            return it->get();
        }
    }

    return nullptr;
}

const Window *WindowManager::windowAt(
    Point position
) const
{
    for (
        auto it =
            m_windows.rbegin();
        it !=
            m_windows.rend();
        ++it
    ) {
        if (
            (*it)->contains(
                position
            )
        ) {
            return it->get();
        }
    }

    return nullptr;
}

void WindowManager::focusNextWindow()
{
    if (
        m_windows.size() <
        2
    ) {
        return;
    }

    auto currentWindow =
        std::move(
            m_windows.back()
        );

    m_windows.pop_back();

    m_windows.insert(
        m_windows.begin(),
        std::move(
            currentWindow
        )
    );

    for (
        auto &window :
        m_windows
    ) {
        window->setFocused(
            false
        );
    }

    Window *nextWindow =
        m_windows.back().get();

    nextWindow->setFocused(
        true
    );

    m_focusedWindowId =
        nextWindow->id();
}

void WindowManager::render(
    myos::graphics::Renderer &renderer
) const
{
    for (
        const auto &window :
        m_windows
    ) {
        window->render(
            renderer
        );
    }
}