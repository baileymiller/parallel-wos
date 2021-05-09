#pragma once

#include <pwos/common.h>

#include <pwos/image.h>
#include <pwos/integrator.h>
#include <pwos/scene.h>
#include <pwos/closestPointGrid.h>
#include <pwos/randomWalk.h>
#include <pwos/progressBar.h>

class MCWoGVisual: public Integrator
{
public:
    float cellLength, minGridR;
    int ncols, nrows;
    float rrProb = 0.99;
    int numUsableThreads;
    shared_ptr<ClosestPointGrid> cpg;
    shared_ptr<RandomWalkManager> sharedRWM;
    shared_ptr<Image> heatMap;

    MCWoGVisual(Scene scene, Vec2i res = Vec2i(128, 128), int spp = 16, int nthreads = 1, float cellSize = 1)
    : Integrator("mcwog", scene, res, spp, nthreads)
    {
        // Set the dimensions of the grid/region that each thread is responsible for
        Vec4f window = scene.getWindow();
        Vec2f bl(window[0], window[1]);
        Vec2f tr(window[2], window[3]);
        float dx = tr.x() - bl.x();
        float dy = tr.y() - bl.y();

        // Determine the size of the cells in each grid 
        cellLength = cellSize * std::min(dx / res.x(), dy / res.y());
        minGridR = BOUNDARY_EPSILON;
        cpg = make_shared<ClosestPointGrid>(this->scene, bl, tr, cellLength, nthreads);

        // Determine how many cols/rows the scene will be divided into
        float log2nthreads = log2(nthreads);
        int n = floor(log2nthreads);
        numUsableThreads = int(pow(2, n));
        WARN_IF(log2nthreads != n, "MC WoG only utilizes 2^n threads. Using " + to_string(numUsableThreads) + " of the " + to_string(nthreads) + " available threads.");

        // create closest point grid and random walk manager
        sharedRWM = make_shared<RandomWalkManager>(cpg, window, res, spp, nthreads);

        heatMap = make_shared<Image>(res);
    };

    void virtual render() override
    {
        ProgressBar progress;
        progress.start(image->getNumPixels());
        int walksRemaining = image->getNumPixels();
        #pragma omp parallel num_threads(numUsableThreads)
        {
Stats::TIME_THREAD(StatTimerType::TOTAL, [this, &progress, &walksRemaining]() -> void {
            size_t tid = omp_get_thread_num();
            pcg32 sampler = getSampler(tid);
            std::shared_ptr<RandomWalkManager> rwm = (tid == 0)
                ? sharedRWM
                : make_shared<RandomWalkManager>(sharedRWM, tid);

            vector<shared_ptr<RandomWalk>> readyToWrite;
            while (true)
            {
                // advance existing walks
                vector<shared_ptr<RandomWalk>> activeRandomWalks = rwm->recvActiveWalks();
                for (int i = 0; i < activeRandomWalks.size(); i++)
                {
                    shared_ptr<RandomWalk> rw = activeRandomWalks[i];
                    advanceWalk(heatMap, scene, rw, cpg, sampler, minGridR, rrProb);
                    rwm->addWalkToBuffer(rw);
                }

                // process finished walks
                vector<shared_ptr<RandomWalk>> terminatedRandomWalks = rwm->recvTerminatedWalks();
                for (int i = 0; i < terminatedRandomWalks.size(); i++)
                {
                    shared_ptr<RandomWalk> rw = terminatedRandomWalks[i];
                    if (rw ->nSamplesLeft == 0)
                    {
                        // no samples left, just terminate random walk and remove it from the list
                        readyToWrite.push_back(rw);
                        
                        progress++;

                        #pragma omp atomic
                        walksRemaining -= 1;
                    }
                    else
                    {
                        // reinitialize the walk
                        rw->initializeWalk();
                        rwm->addWalkToBuffer(rw);
                    }
                }
                rwm->sendWalks();

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
        heatMap->save("mc-wog-heatmap");
    }

private:
    inline static void advanceWalk(shared_ptr<Image> heatMap, shared_ptr<Scene> scene, shared_ptr<RandomWalk> rw, shared_ptr<ClosestPointGrid> cpg, pcg32 &sampler, float minGridR, float rrProb)
    {
        Vec2f p = rw->p;
        Vec3f b;
        float R, dist, gridDist; 

        if (cpg->pointInGridRange(p))
        {
            cpg->getDistToClosestPoint(p, b, dist, gridDist);

             // mark heat map where grid was touched
            if (omp_get_thread_num() == 0)
            {
                Vec2i g = cpg->getGridCoordinates(p);
                Vec2f gp = cpg->getGridPointCoordinates(g);
                Vec2i pxy = getPixelCoords(gp, scene->getWindow(), heatMap->getRes());
                heatMap->operator()(pxy.x(), pxy.y()) = Vec3f(1.0f, 1.0f, 1.0f);
            }

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
            // if we end up here, the point is not within the grid do a normal closest point query.
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
