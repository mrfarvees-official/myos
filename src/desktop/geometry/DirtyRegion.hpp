#pragma once

#include <algorithm>
#include <vector>

#include "Rect.hpp"

class DirtyRegion
{
public:
    explicit DirtyRegion(
        const Rect &bounds
    )
        : m_bounds(bounds)
    {
    }

    void invalidate(
        const Rect &rect
    )
    {
        Rect clipped =
            intersect(
                rect,
                m_bounds
            );

        if (
            clipped.width <= 0 ||
            clipped.height <= 0
        ) {
            return;
        }

        // If this area is already fully covered
        // by an existing dirty region, nothing
        // needs to be added.
        for (
            const Rect &existing :
            m_regions
        ) {
            if (
                contains(
                    existing,
                    clipped
                )
            ) {
                return;
            }
        }

        // Remove smaller regions completely
        // covered by the new dirty rectangle.
        m_regions.erase(
            std::remove_if(
                m_regions.begin(),
                m_regions.end(),
                [&clipped](
                    const Rect &existing
                ) {
                    return contains(
                        clipped,
                        existing
                    );
                }
            ),
            m_regions.end()
        );

        m_regions.push_back(
            clipped
        );
    }

    void invalidateAll()
    {
        m_regions.clear();

        m_regions.push_back(
            m_bounds
        );
    }

    bool empty() const
    {
        return m_regions.empty();
    }

    const std::vector<Rect> &regions() const
    {
        return m_regions;
    }

    void clear()
    {
        m_regions.clear();
    }

private:
    static bool contains(
        const Rect &outer,
        const Rect &inner
    )
    {
        return
            inner.x >= outer.x &&
            inner.y >= outer.y &&
            inner.right() <= outer.right() &&
            inner.bottom() <= outer.bottom();
    }

    static Rect intersect(
        const Rect &a,
        const Rect &b
    )
    {
        const int left =
            std::max(
                a.x,
                b.x
            );

        const int top =
            std::max(
                a.y,
                b.y
            );

        const int right =
            std::min(
                a.right(),
                b.right()
            );

        const int bottom =
            std::min(
                a.bottom(),
                b.bottom()
            );

        return Rect {
            left,
            top,
            right - left,
            bottom - top
        };
    }

    Rect m_bounds;

    std::vector<Rect> m_regions;
};