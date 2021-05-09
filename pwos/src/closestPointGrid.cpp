#include <pwos/common.h>
#include <pwos/closestPointGrid.h>
#include <pwos/scene.h>
#include <pwos/progressBar.h>
#include <pwos/stats.h>

GridData::GridData() {};

GridData::GridData(float dist, shared_ptr<Vec3f> b): dist(dist), b(b) {};

ClosestPointGrid::ClosestPointGrid(shared_ptr<Scene> scene, Vec2f bl, Vec2f tr, float cellLength, int nthreads): bl(bl), tr(tr), cellLength(cellLength)
{
Stats::TIME(StatTimerType::GRID_CREATION, [this, bl, tr, cellLength, nthreads, scene]() -> void
{
    float width = tr.x() - bl.x();
    float height = tr.y() - bl.y();

    gridWidth = ceil(width / cellLength + 1) + 10;
    gridHeight = ceil(height / cellLength + 1) + 10;

    grid = new GridData[gridWidth * gridHeight];

    // layout grid in memory in blocks (nthreads = 2^n blocks)
    float log2nthreads = log2(nthreads);
    int n = floor(log2nthreads);
    int numUsableThreads = int(pow(2, n));
    WARN_IF(log2nthreads != n, "CPG only utilizes 2^n blocks. Will support blocked regions for " + to_string(numUsableThreads) + " of the " + to_string(nthreads) + " available threads.");

    nBlockCols = (n == 0) || (n % 2 != 0) ? n + 1 : n;
    nBlockRows = numUsableThreads / nBlockCols;
    nBlocks = nBlockCols * nBlockRows;

    blockWidth = ceil(gridWidth / nBlockCols);
    blockHeight = ceil(gridHeight / nBlockRows);
    blockSize = blockWidth * blockHeight;

    ProgressBar progress;
    progress.start(nBlocks * blockWidth);

    Stats::SET_COUNT(StatType::GRID_POINTS, gridWidth * gridHeight);

    #pragma omp parallel for num_threads(nthreads)
    for (int bid = 0; bid < nBlocks; bid++)
    {
        int bidy = bid / nBlockCols;
        int bidx = bid % nBlockCols;
        int maxIdx = std::min(blockWidth, gridWidth - blockWidth * bidx);
        int maxIdy = std::min(blockHeight, gridHeight - blockHeight * bidy);

        int blockOffset = bid * blockSize;
        int blockX = bidx * blockWidth;
        int blockY = bidy * blockHeight;

        // iterate over all cells in the block
        for (int idx = 0; idx < maxIdx; idx++)
        {
            for (int idy = 0; idy < maxIdy; idy++)
            {
                // id = offset within block + global block offset
                int id = (idx + idy * blockWidth) + blockOffset;
                Vec3f b;
                Vec2i gc = Vec2i(idx + blockX, idy + blockY);
                Vec2f gp = getGridPointCoordinates(gc);
                Vec2f closestPoint = scene->getClosestPoint(gp, b, true);
                grid[id].dist = (closestPoint - gp).norm();
                grid[id].b = make_shared<Vec3f>(b);
            }
            progress++;
        }
    }
    progress.finish();
});
}

bool ClosestPointGrid::getDistToClosestPoint(Vec2f p, Vec3f &b, float &dist, float &gridDist) const
{
Stats::INCREMENT_COUNT(StatType::GRID_QUERY);
Stats::TIME_THREAD(StatTimerType::CLOSEST_POINT_GRID, [this, p, &b, &dist, &gridDist]() -> void {
    // TODO: potentially use another corner of the cell if it is closer, for now use bottom left (fast to compute)
    Vec2i g = getGridCoordinates(p);
    int id = getGridPointIndex(g);
    b = *grid[id].b;
    dist = grid[id].dist;
    gridDist = (p - getGridPointCoordinates(g)).norm();

});
    return true;
}

