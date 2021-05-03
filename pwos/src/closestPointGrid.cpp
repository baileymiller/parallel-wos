#include <pwos/common.h>
#include <pwos/closestPointGrid.h>
#include <pwos/scene.h>
#include <pwos/progressBar.h>
#include <pwos/stats.h>

GridData::GridData() {};

GridData::GridData(float dist, shared_ptr<Vec3f> b): dist(dist), b(b) {};

ClosestPointGrid::ClosestPointGrid(shared_ptr<Scene> scene, Vec2f bl, Vec2f tr, float cellLength, int nthreads): bl(bl), tr(tr), cellLength(cellLength)
{
    float width = tr.x() - bl.x();
    float height = tr.y() - bl.y();

    gridWidth = ceil(width / cellLength) + 1;
    gridHeight = ceil(height / cellLength) + 1;

    grid = new GridData[gridWidth * gridHeight];

    #pragma omp parallel for num_threads(nthreads)
    for (int idy = 0; idy < gridHeight; idy++) {

        for (int idx = 0; idx < gridWidth; idx++)
        {
            // index of the grid point in the array
            int id = idx + idy * gridWidth;

            // compute grid data
            Vec3f b;
            Vec2f gp = getGridPointCoordinates(Vec2i(idx, idy));
            Vec2f closestPoint = scene->getClosestPoint(gp, b);
            grid[id].dist = (closestPoint - gp).norm();
            grid[id].b = make_shared<Vec3f>(b);

        }
    }
}

bool ClosestPointGrid::getDistToClosestPoint(Vec2f p, Vec3f &b, float &dist, float &gridDist) const
{
    Stats::INCREMENT_COUNT(StatType::GRID_QUERY);
    // ensure that p is within grid, otherwise return false
    if (p.x() < bl.x() || tr.x() <= p.x()) return false;
    if (p.y() < bl.y() || tr.y() <= p.y()) return false;

    // get offset within grid
    float xOffset = p.x() - bl.x();
    float yOffset = p.y() - bl.y();

    // TODO: potentially use another corner of the cell if it is closer, for now use bottom left (fast to compute)
    int idx = floor(xOffset / cellLength);
    int idy = floor(yOffset / cellLength);
    int id = idx + idy * gridWidth;

    b = *grid[id].b;
    dist = grid[id].dist;
    gridDist = (p - getGridPointCoordinates(Vec2i(idx, idy))).norm();

    return true;
}

