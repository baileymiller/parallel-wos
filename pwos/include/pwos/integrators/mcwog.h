#pragma once

#include <pwos/common.h>

#include <pwos/image.h>
#include <pwos/integrator.h>
#include <pwos/scene.h>
#include <pwos/closestPointGrid.h>

/**
 * Holds all of the data needed for a random walk, allows random walks to be 
 * progressed by several different threads at once.
 */
struct RandomWalk
{
    // id of the parent thread
    int parentId;

    // the pixel id for this thread
    int pixelId;

    // the current position of the random walk
    Vec2f p;

    // the throughput of the current path
    float f;

    // the number of steps on the walk so far
    int currSteps;

    // the final value for the walk (i.e. boundary value when it terminates)
    float val;

    // finish the random walk
    bool finished;

    /**
     * Initialize a random walk.
     * 
     * @param parentId      the thread id of the parent who started this walk
     * @param pixelId       the pixel id that this random walk corresponds to
     * @param p             the current position of the random walk
     */
    RandomWalk(int parentId, int pixelId, Vec2f p)
    : parentId(parentId)
    , pixelId(pixelId)
    , p(p)
    , f(1.0f)
    , currSteps(0)
    , finished(false) {};

    /**
     * Progress the random walk by taking a step
     * 
     * @param stepVec       offset vector to add to the current position
     * @param fUpdatel      an update to the throughput 
     */
    void takeStep(Vec2f stepVec, float fUpdate)
    {
        currSteps++;
        f *= fUpdate;
        p += stepVec;
    }

    /**
     * Finish the random walk.
     * 
     * @param g       the boundary value at the end of the random walk
     */
    void finish(float g)
    {
        val = f * g;
        finished = true;
    }
};

class MCWoG: public Integrator
{
public:
    float cellLength, minGridR;
    float gridWidth, gridHeight;
    int ncols, nrows;

    MCWoG(Scene scene, Vec2i res = Vec2i(128, 128), int spp = 16, int nthreads = 1)
    : Integrator("mcwog", scene, res, spp, nthreads)
    {
        // Determine how many cols/rows the scene will be divided into
        float log2nthreads = log2(nthreads);
        int n = floor(log2nthreads);
        int numUsableThreads = int(pow(2, n));
        WARN_IF(log2nthreads != n, "MC WoG only utilizes 2^n threads. Using " + to_string(numUsableThreads) + " of the " + to_string(nthreads) + " available threads.");

        ncols = (n == 0) || (n % 2 != 0) ? n + 1 : n;
        nrows = numUsableThreads / ncols;
        
        // Set the dimensions of the grid/region that each thread is responsible for
        Vec4f window = scene.getWindow();
        Vec2f bl(window[0], window[1]);
        Vec2f tr(window[2], window[3]);
        float dx = tr.x() - bl.x();
        float dy = tr.y() - bl.y();
        gridWidth = dx / ncols;
        gridHeight = dy / nrows;

        // Determine the size of the cells in each grid 
        Vec2i res = image->getRes();
        cellLength = std::min(dx / res.x(), dy / res.y());

        // The minimum length of R before we switch to doing CPQ's and stop looking in grid
        minGridR = sqrt(2) * cellLength;
    };

    void virtual render() override
    {
        // preprocess by computing closest point grid
        Vec4f window = scene->getWindow();
        Vec2f bl(window[0], window[1]);
        Vec2f tr(window[2], window[3]);

        #pragma omp parallel num_threads(nthreads)
        {
            size_t tid = omp_get_thread_num();

            // make grid
            int col = tid % ncols;
            int row = tid / ncols;
            Vec2f gridBL(
                bl.x() + col * gridWidth,
                bl.y() + row * gridHeight
            );
            Vec2f gridTR = gridBL + Vec2f(gridWidth, gridHeight);
            ClosestPointGrid* cpg = new ClosestPointGrid(scene, gridBL, gridTR, cellLength, 1);

            // initialize
        }

        image->render(scene->getWindow(), nthreads, [this](Vec2f coord, pcg32& sampler) -> Vec3f
        {
            Vec3f pixelValue(0, 0, 0);
            for (int j = 0; j < spp; j++)
            {
                pixelValue += u_hat(coord, sampler);
            }
            return pixelValue / float(spp);
        });
    }

private:
    float rrProb = 0.99;

    Vec3f u_hat(Vec2f x0, pcg32 &sampler) const
    {
        Vec2f p = x0;
        Vec3f b;
        float R, dist, gridDist;
        float f = 1.0f;
        do
        {
            if (cpg->getDistToClosestPoint(p, b, dist, gridDist))
            {
                // conservative distance to nearest boundary
                R = dist - gridDist;
                if (R < minGridR)
                {
                    // grid point too close to boundary, just do our own cpq
                    R = (scene->getClosestPoint(p, b) - p).norm();
                    if (R < BOUNDARY_EPSILON) break;
                }
            }
            else
            {
                // if we end up here, the point is not within the grid. do a normal closest point query.
                R = (scene->getClosestPoint(p, b) - p).norm();
                if (R < BOUNDARY_EPSILON) break;
            }

            if (sampler.nextFloat() < (1.0f - rrProb)) break;
            f /= rrProb;
            p += sampleCirclePoint(R, sampler.nextFloat());
        }
        while (true);

        return R < BOUNDARY_EPSILON ? b : Vec3f(0, 0, 0); 
    }
};
