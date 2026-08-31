#include "WindowManager.hpp"

#include <algorithm>

#include "../graphics/Renderer.hpp"

Window &WindowManager::createWindow(
    const std::string &title,
    const Rect &bounds
)
{
    const int id = m_nextWindowId++;

    auto window = std::make_unique<Window>(
        id,
        title,
        bounds
    );

    Window &reference = *window;

    m_windows.push_back(std::move(window));

    focusWindow(id);

    return reference;
}

void WindowManager::focusWindow(int windowId)
{
    auto it = std::find_if(
        m_windows.begin(),
        m_windows.end(),
        [windowId](const std::unique_ptr<Window> &window) {
            return window->id() == windowId;
        }
    );

    if (it == m_windows.end()) {
        return;
    }

    for (auto &window : m_windows) {
        window->setFocused(false);
    }

    (*it)->setFocused(true);

    m_focusedWindowId = windowId;

    auto window = std::move(*it);

    m_windows.erase(it);

    m_windows.push_back(std::move(window));
}

Window *WindowManager::focusedWindow()
{
    for (auto &window : m_windows) {
        if (window->id() == m_focusedWindowId) {
            return window.get();
        }
    }

    return nullptr;
}

const Window *WindowManager::focusedWindow() const {
    for (const auto &window : m_windows) {
        if (window->id() == m_focusedWindowId) {
            return window.get();
        }
    }

    return nullptr;
}

void WindowManager::handleEvent(const InputEvent &event)
{
    if (event.type == InputEventType::MouseButtonDown &&
        event.mouseButton == MouseButton::Left)
    {
        Window *window = windowAt(event.mousePosition);

        if (!window) {
            return;
        }

        focusWindow(window->id());

        window = focusedWindow();

        if (!window) {
            return;
        }

        if (window->titleBarBounds().contains(event.mousePosition)) {
            m_draggedWindo = window;

            const Rect &bounds = window->bounds();

            m_dragOffset = Point {
                event.mousePosition.x - bounds.x,
                event.mousePosition.y - bounds.y
            };
        }
    }
}

Window *WindowManager::windowAt(Point position)
{
    for (auto it = m_windows.rbegin(); it != m_windows.rend(); ++it) {
        if ((*it)->contains(position)) {
            return it->get();
        }
    }

    return nullptr;
}

const Window *WindowManager::windowAt(Point position) const
{
    for (auto it = m_windows.rbegin(); it != m_windows.rend(); ++it) {
        if ((*it)->contains(position)) {
            return it->get();
        }
    }

    return nullptr;
}

void WindowManager::focusNextWindow() 
{
    if (m_windows.size() < 2) {
        return;
    }

    const int nextWindowId = m_windows[m_windows.size() - 2]->id();

    focusWindow(nextWindowId);
}

void WindowManager::render(myos::graphics::Renderer &renderer) const {
    for (const auto &window : m_windows) {
        window->render(renderer);
    }
}
