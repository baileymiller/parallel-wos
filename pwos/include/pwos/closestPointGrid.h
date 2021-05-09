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
    // grid goes from bottom left to top right
    Vec2f bl, tr;

    // the width and height of the grid (i.e. # of cells wide and # of cells tall)
    int gridWidth, gridHeight;

    // some variables to keep track of the block layout of the grid

    // the width and height (in # of grid cells) of the block
    int blockWidth, blockHeight;

    // the # of grid cells in a give block
    int blockSize;

    // the number of rows and columns of blocks
    int nBlockRows, nBlockCols, nBlocks;

    // the data of the grid (stored in the corner of grid cells)
    GridData* grid;
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
     * Indicate whether or not p is in the grid's range.
     * 
     * @param p
     * 
     * @return true if p in range otherwise false
     */
    inline bool pointInGridRange(Vec2f p) const
    {
        return p.x() >= bl.x() && p.x() < tr.x() && p.y() >= bl.y() && p.y() < tr.y();
    }
    
    /**
     * Returns the block id for a 2D coordinate
     * 
     * @param p     a 2D coordinate
     * 
     * @return the id of the block that the contains the point
     */
    inline int getBlockId(Vec2f p) const
    {
        THROW_IF(!pointInGridRange(p), "(1) Invalid point, not in range of the grid: " + to_string(p.x()) + ", " + to_string(p.y()));
        Vec2i g = getGridCoordinates(p);
        return getBlockId(g, p);
    }

    /**
     * Returns the block id that the grid coordinates falls into
     * 
     * @param g     a grid point (i.e. two indices that specify a point on the grid)
     * 
     * @return the id of the block that contains the grid point
     * 
     */
    inline int getBlockId(Vec2i g, Vec2f p) const
    {
        int bidx = floor(g.x() / blockWidth);
        int bidy = floor(g.y() / blockHeight);
        int bid = bidx + bidy * nBlockCols;
        THROW_IF(bid < 0 || bid >= nBlocks, "(2) Invalid grid point, not in range of the grid: " + to_string(g.x()) + ", " + to_string(g.y()) + " (Block id: " + to_string(bid) + ") " + to_string(p.x()) + ", " + to_string(p.y()));
        return bid;
    }

    /**
     * Returns the block id that the grid coordinates falls into
     * 
     * @param g     a grid point (i.e. two indices that specify a point on the grid)
     * @param b     the x,y indices of the block
     * 
     * @return the id of the block that contains the grid point
     * 
     */
    inline int getBlockId(Vec2i g, Vec2i &b) const
    {
        b.x() = floor(float(g.x()) / blockWidth);
        b.y() = floor(float(g.y()) / blockHeight);
        int bid = b.x() + b.y() * nBlockCols;
        THROW_IF(bid < 0 || bid >= nBlocks, "(3) Invalid grid point, not in range of the grid: " + to_string(g.x()) + ", " + to_string(g.y()) + " (Block id: " + to_string(bid) + ")");
        return bid;
    }

    /**
     * Returns the grid coordainates of a point.
     * 
     * @param p     a point in 2D
     * 
     * @return grid coordinates
     */
    inline Vec2i getGridCoordinates(Vec2f p) const
    {
        Vec2i g(
           floor((p.x() - bl.x()) / cellLength),
           floor((p.y() - bl.y()) / cellLength)
        );
        THROW_IF(g.x() < 0 || g.x() >= gridWidth || g.y() < 0 || g.y() >= gridHeight, "Point p is not in grid range!");
        return g;
    }

    /**
     * Return the id of the grid point within the grid data array.
     * 
     * @param g     a grid point
     * 
     * @returns the id of the grid
     */
    inline int getGridPointIndex(Vec2i g) const
    {
        Vec2i b;
        int bid = getBlockId(g, b);
        Vec2i offset(
            g.x() - b.x() * blockWidth,
            g.y() - b.y() * blockHeight
        );
        return (offset.x() + offset.y() * blockWidth) + (bid * blockSize);
    }

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
            cellLength * float(g.x()) + bl.x(),
            cellLength * float(g.y()) + bl.y()
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
};
