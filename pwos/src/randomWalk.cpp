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

vector<shared_ptr<RandomWalk>> RandomWalkQueue::popAllFront()
{
    vector<shared_ptr<RandomWalk>> rws;
    lock->lock();
    while(q->size() > 0)
    {
        shared_ptr<RandomWalk> rw = q->front();
        q->pop_front();
        rws.push_back(rw);
    }
    lock->unlock();
    return rws;
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
, nthreads(rwm->nthreads)
, activeWalks(rwm->activeWalks)
, terminatedWalks(rwm->terminatedWalks)
, cpg(rwm->cpg)
{}

RandomWalkManager::RandomWalkManager(shared_ptr<ClosestPointGrid> cpg, Vec4f window, Vec2i res, int spp, int nthreads)
: tid(0)
, cpg(cpg)
, nthreads(nthreads)
{
    // create all of the dequeues to be used
    for (int i = 0; i < nthreads * nthreads; i++)
    {
        activeWalks.push_back(make_shared<RandomWalkQueue>());
        terminatedWalks.push_back(make_shared<RandomWalkQueue>());
    }

    ProgressBar progress;
    progress.start(res.x() * res.y());
    #pragma omp parallel for num_threads(nthreads)
    for (int ix = 0; ix < res.x(); ix++)
    {
        size_t tid = omp_get_thread_num();
        for (int iy = 0; iy < res.y(); iy++)
        {
            Vec2f coord = getXYCoords(Vec2i(ix, iy), window, res);
            int parentId = getParentId(coord);
            pushWalk(tid, parentId, make_shared<RandomWalk>(tid, ix + iy * res.y(), coord, spp));
        }
    }
    progress.finish();
};

int RandomWalkManager::getWriteQueueId(int receiver)
{
    return tid * nthreads + receiver;
}

int RandomWalkManager::getReadQueueId(int receiver)
{
    return receiver * nthreads + tid;
}

int RandomWalkManager::getQueueId(int sender, int receiver)
{
    return (sender * nthreads) + receiver;
}

int RandomWalkManager::getParentId(Vec2f p)
{
    return cpg->getBlockId(p);
};

void RandomWalkManager::setThreadId(size_t tid)
{
    this->tid = tid;
};

vector<shared_ptr<RandomWalk>> RandomWalkManager::popActiveWalks()
{
    vector<shared_ptr<RandomWalk>> rws;
    for (size_t sender = 0; sender < nthreads; sender++)
    {
        int idx = getReadQueueId(sender);
        THROW_IF(idx < 0 || idx > 2 * nthreads * nthreads, "POP WALKS IDX OUT OF RANGE " + to_string(int(idx)) + " " + to_string(tid) + " " + to_string(sender) );
        vector<shared_ptr<RandomWalk>> rwsFromSender = activeWalks[idx]->popAllFront();
        for (shared_ptr<RandomWalk> rw : rwsFromSender)
        {
            rws.push_back(rw);
        }
    }
    return rws;
}

vector<shared_ptr<RandomWalk>> RandomWalkManager::popTerminatedWalks()
{
    vector<shared_ptr<RandomWalk>> rws;
    for (size_t sender = 0; sender < nthreads; sender++)
    {
        int idx = getReadQueueId(sender);
        THROW_IF(idx < 0 || idx > 2 * nthreads * nthreads, "POP WALKS IDX OUT OF RANGE " + to_string(int(idx))+ " " + to_string(tid) + " " + to_string(sender) );
        vector<shared_ptr<RandomWalk>> rwsFromSender = terminatedWalks[idx]->popAllFront();
        for (shared_ptr<RandomWalk> rw : rwsFromSender)
        {
            rws.push_back(rw);
        }
    }
    return rws;
}

void RandomWalkManager::pushWalk(int sender, int receiver, shared_ptr<RandomWalk> rw)
{
    int idx = getQueueId(sender, receiver);
    THROW_IF(tid < 0 || tid > activeWalks.size(), "tid " + to_string(tid) + " is out of range. Cannot add random walk to queue.");
    Stats::TIME_THREAD(tid, StatTimerType::QUEUE, [this, idx, rw]() -> void {
    if (rw->terminated)
    {
        terminatedWalks[idx]->pushBack(rw);
    }
    else
    {
        activeWalks[idx]->pushBack(rw);
    }

    });
}

void RandomWalkManager::pushWalks(vector<shared_ptr<RandomWalk>> rws)
{
Stats::TIME_THREAD(tid, StatTimerType::QUEUE, [this, &rws]() -> void {

    for (shared_ptr<RandomWalk> rw : rws)
    {
        if (cpg->pointInGridRange(rw->p))
        {
            // determine the row and column where random walk is located
            pushWalk(tid, cpg->getBlockId(rw->p), rw);
        }
        else
        {
            // just push back into parent's queue, will need to do real closest point queries
            pushWalk(tid, rw->parentId, rw);
        }
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
