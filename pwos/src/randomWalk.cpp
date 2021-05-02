#include <pwos/common.h>
#include <pwos/randomWalk.h>

RandomWalk::RandomWalk(int parentId, int pixelId, Vec2f p, int nSamples)
    : parentId(parentId)
    , pixelId(pixelId)
    , startP(p)
    , val(Vec3f(0.0f, 0.0f, 0.0f))
    , nSamplesLeft(nSamples) {};

void RandomWalk::initializeWalk()
{
    terminated = false;
    f = 1.0f;
    p = startP;
    currSteps = 0;
}

void RandomWalk::takeStep(Vec2f stepVec, float fUpdate)
{
    currSteps++;
    f *= fUpdate;
    p += stepVec;
}

void RandomWalk::terminate(Vec3f g)
{
    val += f * g;
    nSamplesLeft--;
    terminated = true;
}

RandomWalkQueue::RandomWalkQueue() 
{
    q = make_shared<deque<shared_ptr<RandomWalk>>>();
};

void RandomWalkQueue::pushBack(shared_ptr<RandomWalk> rw)
{
    #pragma omp critical
    {
        q->push_back(rw);
    }
}

shared_ptr<RandomWalk> RandomWalkQueue::popFront()
{
    shared_ptr<RandomWalk> rw;
    #pragma omp critical
    {
        rw = q->front();
        q->pop_front();
    }
    return rw;
}

int RandomWalkQueue::size()
{
    int qsize;
    #pragma omp critical
    {
        qsize = q->size();
    }
    return qsize;
}

RandomWalkManager::RandomWalkManager(int nthreads, Vec2f bl, Vec2f tr, float gridWidth, float gridHeight, int ncols, int nrows)
: nthreads(nthreads)
, bl(bl)
, tr(tr)
, gridWidth(gridWidth)
, gridHeight(gridHeight)
, ncols(ncols)
, nrows(nrows)
, tid(0)
{
    // create all of the dequeues to be used
    for (int i = 0; i < nthreads; i++)
    {
        activeWalks.push_back(make_shared<RandomWalkQueue>());
        terminatedWalks.push_back(make_shared<RandomWalkQueue>());
    }
};

void RandomWalkManager::setThreadId(size_t tid)
{
    this->tid = tid;
};

bool RandomWalkManager::hasActiveWalks()
{
    return activeWalks[tid]->size() > 0;
}

bool RandomWalkManager::hasTerminatedWalks()
{
    return terminatedWalks[tid]->size() > 0;
}

shared_ptr<RandomWalk> RandomWalkManager::popActiveWalk()
{
    return activeWalks[tid]->popFront();
}

shared_ptr<RandomWalk> RandomWalkManager::popTerminatedWalk()
{
    return terminatedWalks[tid]->popFront();
}

void RandomWalkManager::pushWalk(size_t tid, shared_ptr<RandomWalk> rw)
{
    if (rw->terminated)
    {
        terminatedWalks[tid]->pushBack(rw);
    }
    else
    {
        activeWalks[tid]->pushBack(rw);
    }
}

void RandomWalkManager::pushWalk(shared_ptr<RandomWalk> rw)
{
    bool xInRange = rw->p.x() > bl.x() && rw->p.x() < tr.x();
    bool yInRange = rw->p.y() > bl.y() && rw->p.y() < tr.y();
    if (!xInRange || !yInRange)
    {
        // just push back into parent's queue, will need to do real closest point queries
        pushWalk(rw->parentId, rw);
    }
    else
    {
        // determine the row and column where random walk is located
        int col = (rw->p.x() - bl.x()) / float(gridWidth);
        int row = (rw->p.y() - bl.y()) / float(gridHeight);
        int tid = col + row * ncols;
        pushWalk(tid, rw);
    }
}

