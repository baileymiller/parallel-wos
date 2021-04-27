#pragma once

#include <pwos/common.h>

class Stats
{
public:
    inline static vector<fsec> threadTime;
    inline static int numClosestPointQueries = 0;
    inline static int numGridQueries = 0;

    static void reset();

    static void initThreadTimers(int nthreads);

    static void TIME_THREAD(size_t tid, FunctionBlock f);

    static void INCREMENT_COUNT(StatType type);

    static void report();
};
