#pragma once

#include <pwos/common.h>

/**
 * A bounding box for shapes in R^2
 * 
 */
class BBox
{
public:
    /**
     * Construct a bounding box.
     * 
     * @param _bl     bottom left corner of the bounding box
     * @param _tr     top right corner of the bounding box
     */
    BBox(Vec2f _bl, Vec2f _tr);

    /**
     * Computes the closest point on the bounding box.
     * 
     * @param o     point from which to compute "closest point"
     * 
     * @return the closest point on the circle to "o"
     */
    Vec2f getClosestPoint(Vec2f o);

    inline float top()
    {
        return _tr.y();
    }

    inline float bottom()
    {
        return _bl.y();
    }

    inline float right()
    {
        return _tr.x();
    }

    inline float left()
    {
        return _bl.x();
    }

    inline Vec2f bl()
    {
        return _bl;
    }
    
    inline Vec2f tl()
    {
        return Vec2f(left(), top());
    }

    inline Vec2f br()
    {
        return Vec2f(bottom(), right());
    }

    inline Vec2f tr()
    {
        return _tr;
    }

private:
    // corners of the bounding box.
    Vec2f _bl, _tr;
};
