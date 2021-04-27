#pragma once

#include <pwos/common.h>

/**
 * The data stored at each grid point.
 */
struct GridData
{
    // distance to the closest point
    float dist;

    // pointer to circle containing the closest point
    shared_ptr<Vec3f> b;

    /**
     * Default constructor for grid data
     */
    GridData();

    /**
     * Construct a grid data point
     */
    GridData(float dist, shared_ptr<Vec3f> b);
};

/**
 * Grid that contains cached closest point queries.
 */
class ClosestPointGrid
{
public:
    // length of each square cell in grid
    float cellLength;

    /**
     * Constructs a closest point grid based on scene's geometry and an arbitrary window.
     * The grid resolution is determined by the cellLength.
     * 
     * @param scene
     * @param bl             the coordinates of bottom left corner of the grid
     * @param tr             the coordinates of the top right corner of the grid
     * @param cellLength    the cell length (square cells)
     * @param nthreads      the number of threads used to compute grid
     */
    ClosestPointGrid(shared_ptr<Scene> scene, Vec2f bl, Vec2f tr, float cellLength, int nthreads = 1);

    /**
     * Return the coordinates of the grid point relative to bl (bottom left coordinate) and cell length
     * 
     * @param g     a grid point (i.e. two indices that specify a point on the grid)
     * 
     * @return the coordinates of the grid point in scene space (use bottom left and cell length to calculate)
     */
    inline Vec2f getGridPointCoordinates(Vec2i g) const
    {
        return Vec2f(
            cellLength * g.x() + bl.x(),
            cellLength * g.y() + bl.y()
        );
    }

    /**
     * Returns the distance to the closest point and pointer to shape that distance corresponds to.
     * This allows callers of the function to access the relevant boundary data (assumed constant per shape)
     * 
     * @param o
     * @param b         the boundary condition at the closest point
     * @param dist      distance to the closest point from the grid point
     * @param gridDist  distance to the grid point used to compute the nearest point distance
     * 
     * @return true if able to lookup closest point, otherwise false
     */
    bool getDistToClosestPoint(Vec2f p, Vec3f &b, float &dist, float &gridDist) const;

private:
    // grid goes from bottom left to top right
    Vec2f bl, tr;

    // the width and height of the grid (i.e. # of cells wide and # of cells tall)
    int gridWidth, gridHeight;

    // the data of the grid (stored in the corner of grid cells)
    GridData* grid;
};
