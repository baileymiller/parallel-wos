#include <pwos/common.h>
#include <pwos/bbox.h>

BBox::BBox(Vec2f p, Vec2f q): p(p), q(q) {};

Vec2f BBox::getClosestPoint(Vec2f o)
{
    // Compute the relative x position type
    RelativePositionType relX;
    if (o.x() > q.x())
    {
        relX = RelativePositionType::RIGHT;
    }
    else if (o.x() < p.x())
    {
        relX = RelativePositionType::LEFT;
    }
    else
    {
        relX = RelativePositionType::INSIDE;
    }
    
    // Compute the relative y position type
    RelativePositionType relY;
    if (o.y() > p.y())
    {
        relY = RelativePositionType::ABOVE;
    }
    else if (o.x() < q.y())
    {
        relY = RelativePositionType::BELOW;
    }
    else
    {
        relY = RelativePositionType::INSIDE;
    }

    if (relX == RelativePositionType::RIGHT)
    {
        // point is to the right of the box
        if (relY == RelativePositionType::ABOVE)
        {
            // return the top right corner
            return Vec2f(p.y(), q.x());
        }
        else if (relY == RelativePositionType::BELOW)
        {
            // return the bottom right corner
            return q;
        }
        else
        {
            // return a point on the right side of box
            return Vec2f(q.x(), o.y());
        }
    }
    else if (relX == RelativePositionType::LEFT)
    {
        // point is to the left of box
        if (relY == RelativePositionType::ABOVE)
        {
            // return the top left corner
            return p;
        }
        else if (relY == RelativePositionType::BELOW)
        {
            // return bottom left corner of box
            return Vec2f(p.x(), q.y());
        }
        else
        {
            // return a point on the right side of box
            return Vec2f(p.x(), o.y());
        }
    }
    else
    {
        // point is in middle of box (in x coords)
        if (relY == RelativePositionType::ABOVE)
        {
            // return a point on the top segment of the box
            return Vec2f(o.x(), p.y());
        }
        else if (relY == RelativePositionType::BELOW)
        {
            // return on a point on the bottom segment of the box
            return Vec2f(o.x(), q.y());
        }
        else
        {
            // return a point on whichever wall is closest

            // compute dist to each wall
            float xLeft = o.x() - p.x();
            float xRight = q.x() - o.x();
            float yTop = p.y() - o.y();
            float yBottom = o.y() - q.y();

            // smallest dist is the wall we compute nearest point on
            float minDist = std::min({xLeft, xRight, yTop, yBottom});

            if (minDist == xLeft)
            {
                // return a point on the left segment of the box
                return Vec2f(p.x(), o.y());
            }
            else if (minDist == xRight)
            {
                // return a point on the right segment of the box
                return Vec2f(q.x(), o.y());
            }
            else if (minDist == yTop)
            {
                // return a point on the top segment of the box
                return Vec2f(o.x(), p.y());
            }
            else
            {
                // return a point on the bottom segment of the box
                return Vec2f(o.x(), q.y());
            }
        }
    }
}
