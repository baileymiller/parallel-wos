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

    // the current position of the random walk
    Vec2f p;

    // the throughput of the current path
    float f;

    // the number of steps on the walk so far
    int currSteps;

    // the final value for the walk (i.e. boundary value when it terminates)
    float val;

    // the number of samples left (whenever a path is terminated this is decremented by 1)
    int nSamplesLeft;

    // terminated the current random walk
    bool terminated;

    /**
     * Initialize a random walk.
     * 
     * @param parentId      the thread id of the parent who started this walk
     * @param pixelId       the pixel id that this random walk corresponds to
     * @param p             the current position of the random walk
     * @param nSamples      the number of samples of this random walk to take
     */
    RandomWalk(int parentId, int pixelId, Vec2f p, int nSamples);

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
    void finish(float g);
};

/**
 * Wrapper with pragmas around deques used for the queueing walks.
 */
class RandomWalkQueue
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

    vector<shared_ptr<deque<shared_ptr<RandomWalk>>>> activeWalks;
    vector<shared_ptr<deque<shared_ptr<RandomWalk>>>> terminatedWalks;

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
    RandomWalkQueue(int nthreads, Vec2f bl, Vec2f tr, float gridWidth, float gridHeight, int ncols, int nrows);

    /**
     * Creates a copy of all the deque pointers and stores the threads
     * tid to use in future deque interactions.
     * 
     * @param tid
     */
    void getThreadCopy(size_t tid);

    /**
     * Pop an active walk of the deque corresponding to tid
     */
    void popActiveWalk();
    
    /**
     * Pop a terminated walk of the deque corresponding to tid.
     */
    void popTeriminatedWalk();

    /**
     * Add random walk back into queue. Will automatically determine
     * which queue to push into based on whether the walk is terminated
     * or not, based on the parent id, and based on the position of the walk.
     * 
     * @param rw        random walk
     */
    void addWalk(shared_ptr<RandomWalk> rw);
};
