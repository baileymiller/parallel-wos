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
 * Wrapper around dequeue with OMP pragmas to ensure atomicity.
 */
class RandomWalkQueue
{
public:
    // underlying deque
    shared_ptr<deque<shared_ptr<RandomWalk>>> q;

    /**
     * Default constructor.
     */
    RandomWalkQueue();

    /**
     * Push a random walk into the back of the queue
     * 
     * @param rw        random walk
     */
    void pushBack(shared_ptr<RandomWalk> rw);

    /**
     * Pop a random walk from the front of the queue.
     * 
     * @return random walk at the front of the queue
     */
    shared_ptr<RandomWalk> popFront();

    /**
     * Gets size of queue.
     * 
     * @return the size of the  queue.
     */
    int size();
};

/**
 * Manager several different queues of random walkers, help determine
 * which queue to push from /pop to based on calling thread's id and
 * on the random walk's location.
 */
class RandomWalkManager
{
public:
    // number of threads.
    int nthreads;

    // number of rows and columns in MC-WoG.
    Vec2f bl, tr;
    int nrows, ncols;
    float gridWidth, gridHeight;

    // thread id that is holding this random walk queue instance.
    size_t tid;

    // walks that are still in progress
    vector<shared_ptr<RandomWalkQueue>> activeWalks;

    // walks that have terminated by reaching a boundary or by being killed in russian roulette
    vector<shared_ptr<RandomWalkQueue>> terminatedWalks;

    /**
     * Build random walk queues
     * 
     * @param nthreads
     * @param bl            bottom left of scene
     * @param tr            top right of scene
     * @param gridWidth     
     * @param gridHeight
     * @param ncols
     * @param nrows
     */
    RandomWalkManager(int nthreads, Vec2f bl, Vec2f tr, float gridWidth, float gridHeight, int ncols, int nrows);

    /**
     * Set the thread id, used in logic for adding/removing random walks from queues.
     * 
     * @param tid
     */
    void setThreadId(size_t tid);

    /**
     * @return true if the thread has active walks
     */
    bool hasActiveWalks();

    /**
     * @return true if the thread has terminated walks
     */
    bool hasTerminatedWalks();

    /**
     * Pop an active walk of the deque corresponding to tid
     * 
     * @return shared pointer for random walk popped from front of queue 
     */
    shared_ptr<RandomWalk> popActiveWalk();
    
    /**
     * Pop a terminated walk of the deque corresponding to tid.
     * 
     * @return shared pointer for random walk popped from front of queue
     */
    shared_ptr<RandomWalk> popTerminatedWalk();
    
    /**
     * Push random walk back into a specific queue. Will automatically
     * determine whether to push into terminated or active queue
     * 
     * @param tid       thread's queue that should receive random walk
     * @param rw        random walk
     */ 
    void pushWalk(size_t tid, shared_ptr<RandomWalk> rw);

    /**
     * Push random walk back into queue. Will automatically determine
     * which queue to push into based on whether the walk is terminated
     * or not, based on the parent id, and based on the position of the walk.
     * 
     * @param rw        random walk
     */
    void pushWalk(shared_ptr<RandomWalk> rw);
};
