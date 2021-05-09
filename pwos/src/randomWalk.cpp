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
    qA = make_shared<deque<shared_ptr<RandomWalk>>>();
    qB = make_shared<deque<shared_ptr<RandomWalk>>>();
    lockA = make_shared<mutex>();
    lockB = make_shared<mutex>();
};

void RandomWalkQueue::pushBackAll(shared_ptr<deque<shared_ptr<RandomWalk>>> &q, vector<shared_ptr<RandomWalk>> rws)
{
    for (shared_ptr<RandomWalk> rw : rws)
    {
        q->push_back(rw);
    }
}

void RandomWalkQueue::pushBackAll(vector<shared_ptr<RandomWalk>> rws)
{
    if (rws.size() == 0)return;
    if (lockA->try_lock())
    {
        pushBackAll(qA, rws);
        lockA->unlock();
    }
    else
    {
        lockB->lock();
        pushBackAll(qB, rws);
        lockB->unlock();
    }
}

void RandomWalkQueue::popAllFront(shared_ptr<deque<shared_ptr<RandomWalk>>> &q, vector<shared_ptr<RandomWalk>> &rws)
{
    while(q->size() > 0)
    {
        shared_ptr<RandomWalk> rw = q->front();
        q->pop_front();
        rws.push_back(rw);
    }
}

vector<shared_ptr<RandomWalk>> RandomWalkQueue::popAllFront()
{
    vector<shared_ptr<RandomWalk>> rws;
    if (qA->size() > 0 && lockA->try_lock())
    {
        int qASize = qA->size();
        popAllFront(qA, rws);
        lockA->unlock();
    }

    if (qB->size() > 0 && lockB->try_lock())
    {
        int qBSize = qB->size();
        popAllFront(qB, rws);
        lockB->unlock();
    }

    return rws;
}

RandomWalkManager::RandomWalkManager(shared_ptr<RandomWalkManager> rwm, size_t tid)
: tid(tid)
, nthreads(rwm->nthreads)
, activeWalks(rwm->activeWalks)
, terminatedWalks(rwm->terminatedWalks)
, cpg(rwm->cpg)
{
    // setup new buffers
    activeWalksSendBuffer = vector<vector<shared_ptr<RandomWalk>>>();
    terminatedWalksSendBuffer = vector<vector<shared_ptr<RandomWalk>>>();

    // create rw send buffer
    for (int i = 0; i < nthreads; i++)
    {
        activeWalksSendBuffer.push_back(vector<shared_ptr<RandomWalk>>());
        terminatedWalksSendBuffer.push_back(vector<shared_ptr<RandomWalk>>());
    }
}

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

    // create rw send buffer
    for (int i = 0; i < nthreads; i++)
    {
        activeWalksSendBuffer.push_back(vector<shared_ptr<RandomWalk>>());
        terminatedWalksSendBuffer.push_back(vector<shared_ptr<RandomWalk>>());
    }

    ProgressBar progress;
    progress.start(res.x() * res.y());
    for (int ix = 0; ix < res.x(); ix++)
    {
        size_t tid = omp_get_thread_num();
        for (int iy = 0; iy < res.y(); iy++)
        {
            Vec2f coord = getXYCoords(Vec2i(ix, iy), window, res);
            shared_ptr<RandomWalk> rw = make_shared<RandomWalk>(tid, ix + iy * res.y(), coord, spp);
            addWalkToBuffer(getParentId(coord), rw);
        }
    }
    sendWalks();
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

vector<shared_ptr<RandomWalk>> RandomWalkManager::recvActiveWalks()
{
    vector<shared_ptr<RandomWalk>> rws;
Stats::TIME_THREAD(StatTimerType::RECV_WALKS, [this, &rws]() -> void {
    for (size_t sender = 0; sender < nthreads; sender++)
    {
        vector<shared_ptr<RandomWalk>> rwsFromSender;
        if (tid != sender)
        {
            int idx = getReadQueueId(sender);
            rwsFromSender = activeWalks[idx]->popAllFront();
        }
        else
        {
            // read from our own send buffer
            rwsFromSender = activeWalksSendBuffer[tid];
        }

        for (shared_ptr<RandomWalk> rw : rwsFromSender)
        {
            rws.push_back(rw);
        }
    }
    // extra cleanup, clear our own send buffer
    activeWalksSendBuffer[tid].clear();
});
    return rws;
}

vector<shared_ptr<RandomWalk>> RandomWalkManager::recvTerminatedWalks()
{
    vector<shared_ptr<RandomWalk>> rws;
Stats::TIME_THREAD(StatTimerType::RECV_WALKS, [this, &rws]() -> void {
    for (size_t sender = 0; sender < nthreads; sender++)
    {
        vector<shared_ptr<RandomWalk>> rwsFromSender;
        if (tid != sender)
        {
            int idx = getReadQueueId(sender);
            rwsFromSender = terminatedWalks[idx]->popAllFront();
        }
        else
        {
            // read from our own send buffer
            rwsFromSender = terminatedWalksSendBuffer[tid];
        }

        for (shared_ptr<RandomWalk> rw : rwsFromSender)
        {
            rws.push_back(rw);
        }
    }
    
    // extra cleanup, clear our own send buffer
    terminatedWalksSendBuffer[tid].clear();
});
    return rws;
}

void RandomWalkManager::sendWalks()
{
Stats::TIME_THREAD(StatTimerType::SEND_WALKS, [this]() -> void {
    for (int i = 0; i < nthreads; i++)
    {
        if (tid != i)
        {
            // write to other threads
            int idx = getWriteQueueId(i);
            if (activeWalksSendBuffer[i].size() > 0)
            {
                activeWalks[idx]->pushBackAll(activeWalksSendBuffer[i]);
                activeWalksSendBuffer[i].clear();
            }

            if (terminatedWalksSendBuffer[i].size() > 0)
            {
                terminatedWalks[idx]->pushBackAll(terminatedWalksSendBuffer[i]);
                terminatedWalksSendBuffer[i].clear();
            }
        }
    }
});
}

void RandomWalkManager::addWalkToBuffer(int receiver, shared_ptr<RandomWalk> rw)
{
    if (rw->terminated)
    {
        terminatedWalksSendBuffer[receiver].push_back(rw);
    }
    else
    {
        activeWalksSendBuffer[receiver].push_back(rw);
    }
}

void RandomWalkManager::addWalkToBuffer(shared_ptr<RandomWalk> rw)
{
    if (cpg->pointInGridRange(rw->p))
    {
        // determine the row and column where random walk is located
        addWalkToBuffer(cpg->getBlockId(rw->p), rw);
    }
    else
    {
        // just push back into parent's queue, will need to do real closest point queries
        addWalkToBuffer(rw->parentId, rw);
    }
}
