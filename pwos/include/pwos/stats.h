#pragma once

#include <pwos/common.h>

class Stats
{
public:
    // total time to create grid
    inline static fsec gridCreationTime;

    // the total run time
    inline static fsec totalTime;

    // the setup time
    inline static fsec setupTime;

    // the total run time for each thread
    inline static vector<fsec> threadTime;

    // How long each thread spends sending random walks
    inline static vector<fsec> threadSendWalksTime;

    // How long each thread spends receiving random walks
    inline static vector<fsec> threadRecvWalksTime;

    // How long each thread spends reading from memory (ClosestPointGrid Queries)
    inline static vector<fsec> threadCPGTime;

    // how long each thread spends doing a closest point query
    inline static vector<fsec> threadCPQTime;

    // how long each thread spends doing a closest point queries during setup
    inline static vector<fsec> threadCPQSetupTime;

    inline static int numGridPoints;

    inline static vector<int> numClosestPointQueries;

    inline static vector<int> numClosestPointQueriesSetup;

    inline static vector<int> numGridQueries;

    static void init(int nthreads = 1);

    static void TIME_THREAD(StatTimerType type, FunctionBlock f);

    static void TIME(StatTimerType type, FunctionBlock f);

    static void SET_COUNT(StatType type, int val);

    static void INCREMENT_COUNT(StatType type);

    static void report();
};
