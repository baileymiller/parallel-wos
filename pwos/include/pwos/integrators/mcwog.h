#pragma once

#include <pwos/common.h>

#include <pwos/image.h>
#include <pwos/integrator.h>
#include <pwos/scene.h>
#include <pwos/closestPointGrid.h>
#include <pwos/randomWalk.h>
#include <pwos/progressBar.h>

class MCWoG: public Integrator
{
public:
    float cellLength, minGridR;
    float gridWidth, gridHeight;
    int ncols, nrows;
    float rrProb = 0.95;
    int numUsableThreads;

    MCWoG(Scene scene, Vec2i res = Vec2i(128, 128), int spp = 16, int nthreads = 1)
    : Integrator("mcwog", scene, res, spp, nthreads)
    {
        // Determine how many cols/rows the scene will be divided into
        float log2nthreads = log2(nthreads);
        int n = floor(log2nthreads);
        numUsableThreads = int(pow(2, n));
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

        // setup random walk manager
        std::shared_ptr<RandomWalkManager> sharedRWM = make_shared<RandomWalkManager>(nthreads, bl, tr, gridWidth, gridHeight, ncols, nrows);

        spp = 1;

        // global counter to keep track of how many random walks remain
        int walksRemaining = image->getNumPixels() * spp;

        // keep track of progress
        ProgressBar progress;
        progress.start(walksRemaining);
        #pragma omp parallel num_threads(numUsableThreads)
        {
            size_t tid = omp_get_thread_num();
            pcg32 sampler = getSampler(tid);
            std::shared_ptr<RandomWalkManager> rwm = make_shared<RandomWalkManager>(sharedRWM, tid);

            // initialize grid
            int col = tid % ncols;
            int row = tid / ncols;
            Vec2f gridBL(
                bl.x() + col * gridWidth,
                bl.y() + row * gridHeight
            );
            Vec2f gridTR = gridBL + Vec2f(gridWidth, gridHeight);

            shared_ptr<ClosestPointGrid> cpg = make_shared<ClosestPointGrid>(scene, gridBL, gridTR, cellLength, 1);

            // initialize random walkers managter
            rwm->setThreadId(tid);

            std::cout << rwm->tid << std::endl;

            // determine block of pixels for thread to render
            initializeWalks(rwm, image->getRes(), col, row, ncols, nrows, window, spp, tid);

            // push walks for thread's pixels into queue
            vector<shared_ptr<RandomWalk>> readyToWrite;

            while (true)
            {
                // advance existing walks
                while (rwm->hasActiveWalks())
                {
                    shared_ptr<RandomWalk> rw = rwm->popActiveWalk();
                    advanceWalk(scene, rw, cpg, sampler, minGridR, rrProb);
                    rwm->pushWalk(rw);
                }

                // process finished walks
                int numCompleted = 0;
                while (rwm->hasTerminatedWalks())
                {
                    shared_ptr<RandomWalk> rw = rwm->popTerminatedWalk();
                    if (rw->nSamplesLeft == 0)
                    {
                        // no samples left, just terminate random walk
                        readyToWrite.push_back(rw);
                    }
                    else
                    {
                        // reinitialize the walk and push it back into the random walk queue.
                        rw->initializeWalk();
                        rwm->pushWalk(rw);
                    }
                    numCompleted++;
                }

                #pragma omp atomic
                walksRemaining = walksRemaining - numCompleted;

                progress += numCompleted;

                // break if no more walks are active anywhere
                if (walksRemaining <= 0) break;
            }
            std::cout << "finished!" << std::endl;

            // write all completed random walks to pixels
            for (shared_ptr<RandomWalk> randomWalk: readyToWrite)
            {
                image->set(randomWalk->pixelId, randomWalk->val / float(spp));
            }
        }
        progress.finish();
    }

private:
    inline static void initializeWalks(shared_ptr<RandomWalkManager> rwm, Vec2i res, int col, int row, int ncols, int nrows, Vec4f window, int spp, size_t tid)
    {
        int gridPixelWidth = ceil(res.x() / float(ncols));
        int gridPixelHeight = ceil(res.y() / float(nrows));

        int startX = col * gridPixelWidth;
        int stopX = std::min(startX + gridPixelWidth, res.x());

        int startY = row * gridPixelHeight;
        int stopY = std::min(startY + gridPixelHeight, res.y());

        for (int i = startX; i < stopX; i++)
        {
            for (int j = startY; j < stopY; j++)
            {
                Vec2f coord = getXYCoords(Vec2i(i, j), window, res);
                shared_ptr<RandomWalk> rw = make_shared<RandomWalk>(tid, i + j * res.y(), coord, spp);
                rw->initializeWalk();
                rwm->pushWalk(rw->parentId, rw);
            }
        }
    }

    inline static void advanceWalk(shared_ptr<Scene> scene, shared_ptr<RandomWalk> rw, shared_ptr<ClosestPointGrid> cpg, pcg32 &sampler, float minGridR, float rrProb)
    {
        Vec2f p = rw->p;
        Vec3f b;
        float R, dist, gridDist; 

        if (cpg->getDistToClosestPoint(p, b, dist, gridDist))
        {
            // conservative distance to nearest boundary
            R = dist - gridDist;
            if (R < minGridR)
            {
                // grid point too close to boundary, just do our own cpq
                R = (scene->getClosestPoint(p, b) - p).norm();
            }
        }
        else
        {
            // if we end up here, the point is not within the grid. do a normal closest point query.
            R = (scene->getClosestPoint(p, b) - p).norm();
        }

        if (R < BOUNDARY_EPSILON)
        {
            // within epsilon of boundary, terminate walk
            rw->terminate(b);
        }
        else if (sampler.nextFloat() < (1.0f - rrProb))
        {
            // walk terminated due to russian roulette (value is set to 0)
            rw->terminate(Vec3f(0.0f, 0.0f, 0.0f));
        }
        else
        {
            // walk continues, take another step
            rw->takeStep(sampleCirclePoint(R, sampler.nextFloat()), 1.0f / rrProb);
        }
    }
};
