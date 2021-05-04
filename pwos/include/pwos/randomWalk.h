#pragma once

#include <pwos/common.h>

/**
 * Holds all of the data needed for a random walk, allows random walks to be 
 * progressed by several different threads at once.
 * 
 * Each pixel should have one corresponding random walk.
 */
struct RandomWalk
{
    // id of the parent thread
    int parentId;

    // the pixel id for this thread
    int pixelId;

    // the starting position of the walk
    Vec2f startP;

    // the current position of the random walk
    Vec2f p;

    // the throughput of the current path
    float f;

    // the number of steps on the walk so far
    int currSteps;

    // the final value for the walk (i.e. boundary value when it terminates)
    Vec3f val;

    // the number of samples left (whenever a path is terminated this is decremented by 1)
    int nSamplesLeft;

    // terminated the current random walk
    bool terminated;

    /**
     * Initialize a random walk.
     * 
     * @param parentId      the thread id of the parent who started this walk
     * @param pixelId       the pixel id that this random walk corresponds to
     * @param startP        the position where the random walk starts
     * @param nSamples      the number of samples of this random walk to take
     */
    RandomWalk(int parentId, int pixelId, Vec2f p, int nSamples);

    /**
     * Reset the random walk.
     * 
     * @param p     position where to reset the walk
     */
    void initializeWalk();

    /**
     * Progress the random walk by taking a step
     * 
     * @param stepVec       offset vector to add to the current position
     * @param fUpdatel      an update to the throughput 
     */
    void takeStep(Vec2f stepVec, float fUpdate);

    /**
     * Finish the random walk.
     * 
     * @param g       the boundary value at the end of the random walk
     */
    void terminate(Vec3f g);
};

/**
 * Wrapper around dequeue with mutex to ensure atomicity.
 */
class RandomWalkQueue
{
public:
    /**
     * Default constructor.
     */
    RandomWalkQueue();

    /**
     * Push a random walk into the back of the queue
     * 
     * @param rws        random walks
     */
    void pushBackAll(vector<shared_ptr<RandomWalk>> rws);

    /**
     * Pop all random walks from the front of the queue.
     * 
     * @return random walks in the queue
     */
    vector<shared_ptr<RandomWalk>> popAllFront();

private:
    // lock for queue
    std::shared_ptr<mutex> lockA, lockB;

    // underlying deque
    shared_ptr<deque<shared_ptr<RandomWalk>>> qA, qB;

    /**
     * Push a random walk into the back of the queue
     * 
     * @param q         queue to push into (assumes the caller has a lock)
     * @param rw        random walk
     */
    void pushBackAll(shared_ptr<deque<shared_ptr<RandomWalk>>> &q, vector<shared_ptr<RandomWalk>> rws);

    /**
     * Pop a random walk from the front of the queue.
     * 
     * @param q       queue to pop from (assumes the caller has a lock)
     * @param rws     vector to populate with random walks 
     */
    void popAllFront(shared_ptr<deque<shared_ptr<RandomWalk>>> &q, vector<shared_ptr<RandomWalk>> &rws);

};

/**
 * Manager several different queues of random walkers, help determine
 * which queue to push from /pop to based on calling thread's id and
 * on the random walk's location.
 */
class RandomWalkManager
{
public:
    int nthreads;

    // pointer to a closest point grid, used to determine where to send a random walk
    shared_ptr<ClosestPointGrid> cpg;

    // thread id that is holding this random walk queue instance.
    size_t tid;

    // queues used to send active or terminated walks back and forth.
    vector<shared_ptr<RandomWalkQueue>> activeWalks;
    vector<shared_ptr<RandomWalkQueue>> terminatedWalks;
    
    // create buffer of all of the walks we intend to send to other threads
    vector<vector<shared_ptr<RandomWalk>>> activeWalksSendBuffer;
    vector<vector<shared_ptr<RandomWalk>>> terminatedWalksSendBuffer;

    // indicates how many walks are left
    shared_ptr<int> walksRemaining;

    /**
     * 
     * Build a copy of the random walk manager from existing random walk manager.
     * 
     * @param rwm       existing random walk manager
     * @param tid       thread id
     */
    RandomWalkManager(shared_ptr<RandomWalkManager> rwm, size_t tid = 0);

    /**
     * Build random walk queues
     * 
     * @param cpg       closest point grid for the scene
     * @param window    scene window (used to initialize random walks)
     * @param res       resolution of output image (used to initialize random walks)
     * @param spp       used to initialize random walks
     * @param nthreads  number of threads available for setup and that will share the RWM
     */
    RandomWalkManager(shared_ptr<ClosestPointGrid> cpg, Vec4f window, Vec2i res, int spp, int nthreads);

    /**
     * Get the queue for this thread (this->tid) to send a message to receiver
     * 
     * @param receiver      the tid of the thread that will receive the random walk
     * 
     * @return the RandomWalkQueue that receivier will read from
     */
    int getWriteQueueId(int receiver);

    /**
     * Get the queue for this thread (this->tid) to read a message from sender
     * 
     * @param sender     the tid of the thread that is sending the random walk
     * 
     * @return the RandomWalkQueue that sender will write to
     */
    int getReadQueueId(int sender);

    /**
     * Get the queue for messages from sender to receiver
     * 
     * @param sender     the tid of the thread that is sending the random walk
     * @param receiver   the tid of the thread that is receiving

     * 
     * @return the RandomWalkQueue that receivier will read from
     */
    int getQueueId(int sender, int receiver);

    /**
     * Set the thread id, used in logic for adding/removing random walks from queues.
     * 
     * @param tid
     */
    void setThreadId(size_t tid);

    /**
     * Get thread id for a 2D coordinate in the scene
     * 
     * @param p
     * 
     * @return the id of the thread that should start processing the point
     */
    int getParentId(Vec2f p);

    /**
     * @return true if the thread has active walks
     */
    bool hasActiveWalks();

    /**
     * @return true if the thread has terminated walks
     */
    bool hasTerminatedWalks();

    /**
     * Add random walk to a send buffer with destination explicitly defined.
     * 
     * @param receiver  destination
     * @param rw        random walk
     */
    void addWalkToBuffer(int receiver, shared_ptr<RandomWalk> rw);

    /**
     * Add random walk to a send buffer with destination automatically determined based
     * on random walk's position and whether it is terminated.
     * 
     * @param rw        random walk
     */
    void addWalkToBuffer(shared_ptr<RandomWalk> rw);

    /**
     * Send buffered random walks to other threads.
     */
    void sendWalks();

    /**
     * Receive active random walks from other threads.
     */
    vector<shared_ptr<RandomWalk>> recvActiveWalks();

    /**
     * Receive terminated random walks from other threads.
     */
    vector<shared_ptr<RandomWalk>> recvTerminatedWalks();

};
