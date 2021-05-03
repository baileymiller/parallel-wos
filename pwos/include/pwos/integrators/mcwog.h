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
    int ncols, nrows;
    float rrProb = 0.99;
    int numUsableThreads;
    shared_ptr<ClosestPointGrid> cpg;
    shared_ptr<RandomWalkManager> sharedRWM;

    MCWoG(Scene scene, Vec2i res = Vec2i(128, 128), int spp = 16, int nthreads = 1)
    : Integrator("mcwog", scene, res, spp, nthreads)
    {
        // Determine how many cols/rows the scene will be divided into
        float log2nthreads = log2(nthreads);
        int n = floor(log2nthreads);
        numUsableThreads = int(pow(2, n));
        WARN_IF(log2nthreads != n, "MC WoG only utilizes 2^n threads. Using " + to_string(numUsableThreads) + " of the " + to_string(nthreads) + " available threads.");

        // Set the dimensions of the grid/region that each thread is responsible for
        Vec4f window = scene.getWindow();
        Vec2f bl(window[0], window[1]);
        Vec2f tr(window[2], window[3]);
        float dx = tr.x() - bl.x();
        float dy = tr.y() - bl.y();

        // Determine the size of the cells in each grid 
        cellLength = std::min(dx / res.x(), dy / res.y());

        // The minimum length of R before we switch to doing CPQ's and stop looking in grid
        minGridR = sqrt(2) * cellLength;

        // create closest point grid and random walk manager
        cpg = make_shared<ClosestPointGrid>(this->scene, bl, tr, cellLength, nthreads);
        sharedRWM = make_shared<RandomWalkManager>(cpg, window, res, spp, nthreads);
    };

    void virtual render() override
    {
        ProgressBar progress;
        progress.start(image->getNumPixels() * spp);
        int walksRemaining = image->getNumPixels() * spp;
        #pragma omp parallel num_threads(numUsableThreads)
        {
        size_t tid = omp_get_thread_num();
Stats::TIME_THREAD(tid, StatTimerType::TOTAL, [this, tid, &progress, &walksRemaining]() -> void {

            pcg32 sampler = getSampler(tid);
            std::shared_ptr<RandomWalkManager> rwm = make_shared<RandomWalkManager>(sharedRWM, tid);

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
                if (walksRemaining <= 0) break;
            }

            // write all completed random walks to pixels
            for (shared_ptr<RandomWalk> randomWalk: readyToWrite)
            {
                image->set(randomWalk->pixelId, randomWalk->val / float(spp));
            }

});
        }
        progress.finish();
    }

private:
    inline static void advanceWalk(shared_ptr<Scene> scene, shared_ptr<RandomWalk> rw, shared_ptr<ClosestPointGrid> cpg, pcg32 &sampler, float minGridR, float rrProb)
    {
        Vec2f p = rw->p;
        Vec3f b;
        float R, dist, gridDist; 

        if (cpg->pointInGridRange(p))
        {
            cpg->getDistToClosestPoint(p, b, dist, gridDist);

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
