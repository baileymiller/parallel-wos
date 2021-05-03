#pragma once

#include <pwos/common.h>

class Stats
{
public:
    // the total run time
    inline static fsec totalTime;

    // the setup time
    inline static fsec setupTime;

    // the total run time for each thread
    inline static vector<fsec> threadTime;

    // How long each thread spends accessing the random walk queue
    inline static vector<fsec> threadQueueTime;

    // How long each thread spends reading from memory (ClosestPointGrid Queries)
    inline static vector<fsec> threadCPGTime;

    // how long each thread spends doing a closest point query
    inline static vector<fsec> threadCPQTime;

    inline static int numClosestPointQueries = 0;

    inline static int numGridQueries = 0;

    static void reset();

    static void initTimers(int nthreads = 1);

    static void TIME_THREAD(size_t tid, StatTimerType type, FunctionBlock f);

    static void TIME(StatTimerType type, FunctionBlock f);

    static void INCREMENT_COUNT(StatType type);

    static void report();
};
