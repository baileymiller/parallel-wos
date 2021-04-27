#pragma once

#include <pwos/common.h>

/**
 * Circle in R^2, defined by a center "c" and a radius "r".
 */
class Circle
{
public:
    /**
     * Construct a circle with specific center and radius.
     * 
     * @param c     center of the circle
     * @param r     radius of the circle
     * @param b     boundary value for the circle (rgb value)
     */
    Circle(Vec2f c, float r, Vec3f b);

    /**
     * Computes the closest point on the circle.
     * 
     * @param o     point from which to compute "closest point"
     * 
     * @return the closest point on the circle to "o"
     */
    Vec2f getClosestPoint(Vec2f o);

    /**
     * Get boundary condition.
     * 
     * @return the value of the boundary condition (only supports constant boundary conditions)
     */
    Vec3f getBoundaryCondition();

    /**
     * Get the bounding box associated with the circle.
     * 
     * @returns the bbox associated with the circle.
     */
    shared_ptr<BBox> getBoundingBox();

private:
   // center of the circle
    Vec2f c;

    // radius of the circle
    float r;

    // boundary condition for the circle
    Vec3f b;

    shared_ptr<BBox> bbox;
};
