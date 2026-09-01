#include "Window.hpp"

#include "../display/DisplayMetrics.hpp"
#include "../graphics/Color.hpp"
#include "../graphics/Renderer.hpp"

Window::Window(
    int id,
    const std::string &title,
    const Rect &bounds,
    const DisplayMetrics &displayMetrics
)
    : m_id(id),
      m_title(title),
      m_bounds(bounds),
      m_displayMetrics(displayMetrics)
{
}

int Window::id() const
{
    return m_id;
}

const std::string &Window::title() const
{
    return m_title;
}

const Rect &Window::bounds() const
{
    return m_bounds;
}

void Window::setBounds(
    const Rect &bounds
)
{
    m_bounds =
        bounds;
}

void Window::setPosition(
    int x,
    int y
)
{
    m_bounds.x =
        x;

    m_bounds.y =
        y;
}

void Window::moveBy(
    int dx,
    int dy
)
{
    m_bounds.x +=
        dx;

    m_bounds.y +=
        dy;
}

bool Window::isFocused() const
{
    return m_focused;
}

void Window::setFocused(
    bool focused
)
{
    m_focused =
        focused;
}

Rect Window::titleBarBounds() const
{
    return Rect {
        m_bounds.x,
        m_bounds.y,
        m_bounds.width,
        m_displayMetrics.titleBarHeight()
    };
}

bool Window::contains(
    Point point
) const
{
    return m_bounds.contains(
        point
    );
}

bool Window::titleBarContains(
    Point point
) const
{
    return titleBarBounds().contains(
        point
    );
}

void Window::render(
    myos::graphics::Renderer &renderer
) const
{
    const Rect body =
        m_bounds;

    const Rect titleBar =
        titleBarBounds();

    const myos::graphics::Color bodyColor {
        45,
        45,
        48,
        255
    };

    const myos::graphics::Color focusedTitleColor {
        55,
        95,
        145,
        255
    };

    const myos::graphics::Color unfocusedTitleColor {
        65,
        65,
        70,
        255
    };

    renderer.fillRect(
        body.x,
        body.y,
        body.width,
        body.height,
        bodyColor
    );

    renderer.fillRect(
        titleBar.x,
        titleBar.y,
        titleBar.width,
        titleBar.height,
        m_focused
            ? focusedTitleColor
            : unfocusedTitleColor
    );
}