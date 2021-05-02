#include <pwos/common.h>
#include <pwos/randomWalk.h>

RandomWalk::RandomWalk(int parentId, int pixelId, Vec2f p, int nSamples)
    : parentId(parentId)
    , pixelId(pixelId)
    , p(p)
    , f(1.0f)
    , currSteps(0)
    , terminated(false)
    , nSamplesLeft(nSamples) {};

void RandomWalk::takeStep(Vec2f stepVec, float fUpdate)
{
    terminated = false;
    currSteps++;
    f *= fUpdate;
    p += stepVec;
}

void RandomWalk::finish(float g)
{
    val += f * g;
    nSamplesLeft--;
    terminated = true;
}

RandomWalkQueue::RandomWalkQueue(int nthreads, Vec2f bl, Vec2f tr, float gridWidth, float gridHeight, int ncols, int nrows)
: nthreads(nthreads)
, bl(bl)
, tr(tr)
, gridWidth(gridWidth)
, gridHeight(gridHeight)
, ncols(ncols)
, nrows(nrows)
{
    // create all of the dequeues to be used
    for (int i = 0; i < nthreads; i++)
    {
        activeWalks.push_back(make_shared<deque<shared_ptr<RandomWalk>>>());
        terminatedWalks.push_back(make_shared<deque<shared_ptr<RandomWalk>>>());
    }
};
