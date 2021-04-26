#include <pwos/common.h>
#include <pwos/bbox.h>

BBox::BBox(Vec2f _bl, Vec2f _tr)
    : _bl(_bl)
    , _tr(_tr) {};

Vec2f BBox::getClosestPoint(Vec2f o)
{
    // Compute the relative x position type
    RelativePositionType relX;
    if (o.x() > right())
    {
        relX = RelativePositionType::RIGHT;
    }
    else if (o.x() < left())
    {
        relX = RelativePositionType::LEFT;
    }
    else
    {
        relX = RelativePositionType::INSIDE;
    }
    
    // Compute the relative y position type
    RelativePositionType relY;
    if (o.y() > top())
    {
        relY = RelativePositionType::ABOVE;
    }
    else if (o.x() < bottom())
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
            return tr();
        }
        else if (relY == RelativePositionType::BELOW)
        {
            // return the bottom right corner
            return br();
        }
        else
        {
            // return a point on the right segment of box
            return Vec2f(right(), o.y());
        }
    }
    else if (relX == RelativePositionType::LEFT)
    {
        // point is to the left of box
        if (relY == RelativePositionType::ABOVE)
        {
            // return the top left corner
            return tl();
        }
        else if (relY == RelativePositionType::BELOW)
        {
            // return bottom left corner of box
            return bl();
        }
        else
        {
            // return a point on the left segment of box
            return Vec2f(left(), o.y());
        }
    }
    else
    {
        // point is in middle of box (in x coords)
        if (relY == RelativePositionType::ABOVE)
        {
            // return a point on the top segment of the box
            return Vec2f(o.x(), top());
        }
        else if (relY == RelativePositionType::BELOW)
        {
            // return on a point on the bottom segment of the box
            return Vec2f(o.x(), bottom());
        }
        else
        {
            // return a point on whichever wall is closest

            // compute dist to each wall
            float xLeft = o.x() - left();
            float xRight = right() - o.x();
            float yTop = top() - o.y();
            float yBottom = o.y() - bottom();

            // smallest dist is the wall we compute nearest point on
            float minDist = std::min({xLeft, xRight, yTop, yBottom});

            if (minDist == xLeft)
            {
                // return a point on the left segment of the box
                return Vec2f(left(), o.y());
            }
            else if (minDist == xRight)
            {
                // return a point on the right segment of the box
                return Vec2f(right(), o.y());
            }
            else if (minDist == yTop)
            {
                // return a point on the top segment of the box
                return Vec2f(o.x(), top());
            }
            else
            {
                // return a point on the bottom segment of the box
                return Vec2f(o.x(), bottom());
            }
        }
    }
}
