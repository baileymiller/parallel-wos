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
     * @param p     top left corner of the bounding box
     * @param q     top right corner of the bounding box
     */
    BBox(Vec2f p, Vec2f q);

    /**
     * Computes the closest point on the bounding box.
     * 
     * @param o     point from which to compute "closest point"
     * 
     * @return the closest point on the circle to "o"
     */
    Vec2f getClosestPoint(Vec2f o);

private:
    // top left corner and bottom right corner of bounding box.
    Vec2f p, q;
};
