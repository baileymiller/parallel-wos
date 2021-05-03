#include <pwos/common.h>
#include <pwos/progressBar.h>
#include <pwos/closestPointGrid.h>
#include <pwos/randomWalk.h>
#include <pwos/stats.h>
#include <unistd.h>

RandomWalk::RandomWalk(int parentId, int pixelId, Vec2f p, int nSamples)
    : parentId(parentId)
    , pixelId(pixelId)
    , startP(p)
    , val(Vec3f(0.0f, 0.0f, 0.0f))
    , nSamplesLeft(nSamples)
    , terminated(false)
    , f(1.0f)
    , p(startP)
    , currSteps(0) {};

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
    lock = make_shared<mutex>();
};

void RandomWalkQueue::pushBack(shared_ptr<RandomWalk> rw)
{
    lock->lock();
    q->push_back(rw);
    lock->unlock();
}

shared_ptr<RandomWalk> RandomWalkQueue::popFront()
{
    lock->lock();
    shared_ptr<RandomWalk> rw = q->front();
    q->pop_front();
    lock->unlock();
    return rw;
}

int RandomWalkQueue::size()
{
    lock->lock();
    int qsize = q->size();
    lock->unlock();
    return qsize;
}

RandomWalkManager::RandomWalkManager(shared_ptr<RandomWalkManager> rwm, size_t tid)
: tid(tid)
, activeWalks(rwm->activeWalks)
, terminatedWalks(rwm->terminatedWalks)
, cpg(rwm->cpg)
{}

RandomWalkManager::RandomWalkManager(shared_ptr<ClosestPointGrid> cpg, Vec4f window, Vec2i res, int spp, int nthreads)
: tid(0)
, cpg(cpg)
{
    // create all of the dequeues to be used
    for (int i = 0; i < nthreads; i++)
    {
        activeWalks.push_back(make_shared<RandomWalkQueue>());
        terminatedWalks.push_back(make_shared<RandomWalkQueue>());
    }

    ProgressBar progress;
    progress.start(res.x() * res.y());
    #pragma omp parallel for num_threads(nthreads)
    for (int ix = 0; ix < res.x(); ix++)
    {
        for (int iy = 0; iy < res.y(); iy++)
        {
            Vec2f coord = getXYCoords(Vec2i(ix, iy), window, res);
            int tid = getParentId(coord);
            pushWalk(tid, make_shared<RandomWalk>(tid, ix + iy * res.y(), coord, spp));
        }
    }
    progress.finish();
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
    THROW_IF(tid < 0 || tid > activeWalks.size(), "tid " + to_string(tid) + " is out of range. Cannot add random walk to queue.");
    Stats::TIME_THREAD(tid, StatTimerType::QUEUE, [this, tid, rw]() -> void {

    if (rw->terminated)
    {
        terminatedWalks[tid]->pushBack(rw);
    }
    else
    {
        activeWalks[tid]->pushBack(rw);
    }

    });
}

void RandomWalkManager::pushWalk(shared_ptr<RandomWalk> rw)
{
    Stats::TIME_THREAD(tid, StatTimerType::QUEUE, [this, rw]() -> void {

    if (cpg->pointInGridRange(rw->p))
    {
        // determine the row and column where random walk is located
        pushWalk(cpg->getBlockId(rw->p), rw);
    }
    else
    {
        // just push back into parent's queue, will need to do real closest point queries
        pushWalk(rw->parentId, rw);
    }

    });
}

void RandomWalkManager::printCounts()
{
    int activeWalkCount = 0;
    int terminatedWalkCount = 0;
    for (int i = 0; i < activeWalks.size(); i++)
    {
        std::cout << "tid #" << i << ": (activeWalks="<< activeWalks[i]->size() << ", terminatedWalks=" << terminatedWalks[i]->size() << std::endl;
        activeWalkCount += activeWalks[i]->size();
        terminatedWalkCount += terminatedWalks[i]->size();
    }
    std::cout << "totals: (activeWalks=" << activeWalkCount << ", terminatedWalks=" << terminatedWalkCount  << ")" << std::endl;
}
