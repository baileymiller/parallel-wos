#include <pwos/common.h>
#include <pwos/circle.h>
#include <pwos/bbox.h>

Circle::Circle(Vec2f c, float r, Vec3f b): c(c), r(r), b(b)
{
    bbox = make_shared<BBox>(c + Vec2f(-r, r), c + Vec2f(r,  -r));
};

Vec2f Circle::getClosestPoint(Vec2f o)
{
    Vec2f v = o - c;
    float vnorm = v.norm();
    if (vnorm < EPSILON)
    {
        // all points on circle equally close, default to (0, r)
        return c + Vec2f(0, r);
    }

    return c + (v * r / vnorm);
}

Vec3f Circle::getBoundaryCondition()
{
    return b;
}

shared_ptr<BBox> Circle::getBoundingBox()
{
    return bbox;
}
